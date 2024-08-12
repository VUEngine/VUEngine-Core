/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Camera.h>
#include <Printing.h>
#include <Profiler.h>
#include <Sound.h>
#include <VirtualList.h>
#include <WaveForms.h>

#include "SoundManager.h"


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											 CLASS' DEFINITIONS
//---------------------------------------------------------------------------------------------------------

SoundRegistry* const _soundRegistries =	(SoundRegistry*)0x01000400; //(SoundRegistry*)0x010003C0;

#define __WAVE_ADDRESS(n)			(uint8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA			(uint8*)0x01000280;
#define __SSTOP						*(uint8*)0x01000580

static SoundManager _soundManager = NULL;
static Camera _camera = NULL;


//---------------------------------------------------------------------------------------------------------
//												 FRIENDS
//---------------------------------------------------------------------------------------------------------

friend class Sound;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------


typedef struct QueuedSound
{
	const SoundSpec* soundSpec;
	uint32 command;
	Vector3D position;
	bool isPositionValid;
	uint32 playbackType;
	EventListener soundReleaseListener;
	ListenerObject scope;

} QueuedSound;

/**
 * Get instance
 *
 * @fn			SoundManager::getInstance()
 * @public
 * @return		SoundManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void SoundManager::constructor()
{
	_camera = Camera::getInstance();
	
	Base::constructor();

	this->sounds = new VirtualList();
	this->soundsMIDI = new VirtualList();
	this->soundsPCM = new VirtualList();

	this->queuedSounds = new VirtualList();
	this->MIDIPlaybackCounterPerInterrupt = false;
	this->soundMIDINode = NULL;
	this->lock = false;
	this->pcmTargetPlaybackFrameRate = __DEFAULT_PCM_HZ;
	this->targetPCMUpdates = __MICROSECONDS_PER_SECOND / this->pcmTargetPlaybackFrameRate;

	_soundManager = this;
}

/**
 * Class destructor
 */
void SoundManager::destructor()
{
	_soundManager = NULL;

	if(!isDeleted(this->queuedSounds))
	{
		VirtualList::deleteData(this->queuedSounds);
		delete this->queuedSounds;
		this->queuedSounds = NULL;
	}

	if(!isDeleted(this->sounds))
	{
		VirtualList::deleteData(this->sounds);
		delete this->sounds;
		this->sounds = NULL;
	}

	delete this->soundsMIDI;
	delete this->soundsPCM;

	Base::destructor();
}

void SoundManager::releaseChannels(VirtualList channels)
{
	NM_ASSERT(!isDeleted(channels), "SoundManager::releaseChannels: deleted channels list");

	if(!isDeleted(channels))
	{
		NM_ASSERT(0 < VirtualList::getSize(channels), "SoundManager::releaseChannels: soundSpec wrapper with no channels");

		for(VirtualNode node = channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			SoundManager::releaseSoundChannel(this, channel);
		}
	}
}

void SoundManager::purgeReleasedSoundWrappers()
{
	for(VirtualNode node = this->sounds->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Sound sound = Sound::safeCast(node->data);

		if(NULL == sound->soundSpec)
		{
			VirtualList::removeElement(this->soundsMIDI, sound);
			VirtualList::removeElement(this->soundsPCM, sound);
			VirtualList::removeNode(this->sounds, node);

			delete sound;

			this->soundMIDINode = NULL;
		}
	}
}

void SoundManager::reset()
{
	VirtualList::deleteData(this->queuedSounds);

	VirtualList::clear(this->soundsMIDI);
	VirtualList::clear(this->soundsPCM);

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		NM_ASSERT(!isDeleted(sound), "SoundManager::reset: deleted soundSpec wrapper");

		delete sound;
	}

	VirtualList::clear(this->sounds);

	int32 i = 0;

	// Reset all channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		this->channels[i].number = i;
		this->channels[i].soundSpec = NULL;
		this->channels[i].cursor = 0;
		this->channels[i].ticks = 0;
		this->channels[i].elapsedTicks = 0;
		this->channels[i].nextElapsedTicksTarget = 0;
		this->channels[i].soundChannel = 0;

		this->channels[i].soundChannelConfiguration.trackType = kUnknownType;
		this->channels[i].soundChannelConfiguration.SxLRV = 0;
		this->channels[i].soundChannelConfiguration.SxRAM = 0;
		this->channels[i].soundChannelConfiguration.SxEV0 = 0;
		this->channels[i].soundChannelConfiguration.SxEV1 = 0;
		this->channels[i].soundChannelConfiguration.SxFQH = 0;
		this->channels[i].soundChannelConfiguration.SxFQL = 0;
		this->channels[i].soundChannelConfiguration.waveFormData = NULL;
		this->channels[i].soundChannelConfiguration.volume = 0xFF;
	}

	// Reset all waveforms
	for(i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		this->waveforms[i].number = i;
		this->waveforms[i].usageCount = 0;
		this->waveforms[i].wave = __WAVE_ADDRESS(i);
		this->waveforms[i].data = NULL;
		this->waveforms[i].overwrite = true;

		for(uint32 j = 0; j < 128; j++)
		{
			this->waveforms[i].wave[j] = 0;
		}
	}

	// Reset modulation data
	uint8* modulationData = __MODULATION_DATA;
	for(i = 0; i <= 32 * 4; i++)
	{
		modulationData[i] = 0;
	}

	for(i = 0; i < __TOTAL_NORMAL_CHANNELS; i++)
	{
		this->channels[i].type = kChannelNormal;
	}

	for(i = __TOTAL_NORMAL_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i++)
	{
		this->channels[i].type = kChannelModulation;
	}

	for(i = __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS; i < __TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS + __TOTAL_NOISE_CHANNELS; i++)
	{
		this->channels[i].type = kChannelNoise;
	}

	this->pcmTargetPlaybackFrameRate = __DEFAULT_PCM_HZ;
	this->targetPCMUpdates = __MICROSECONDS_PER_SECOND / this->pcmTargetPlaybackFrameRate;
	this->MIDIPlaybackCounterPerInterrupt = 0;
	this->soundMIDINode = NULL;

	SoundManager::stopAllSounds(this, false, NULL);
	SoundManager::unlock(this);
}

void SoundManager::deferMIDIPlayback(uint32 MIDIPlaybackCounterPerInterrupt)
{
	this->MIDIPlaybackCounterPerInterrupt = MIDIPlaybackCounterPerInterrupt;
}

void SoundManager::setTargetPlaybackFrameRate(uint16 pcmTargetPlaybackFrameRate)
{
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;

	if(0 == this->pcmTargetPlaybackFrameRate)
	{
		this->pcmTargetPlaybackFrameRate = __DEFAULT_PCM_HZ;
	}

	this->targetPCMUpdates = __MICROSECONDS_PER_SECOND / this->pcmTargetPlaybackFrameRate;
}

void SoundManager::flushQueuedSounds()
{
	VirtualList::deleteData(this->queuedSounds);
}

void SoundManager::tryToPlayQueuedSounds()
{
	for(VirtualNode node = this->queuedSounds->head; NULL != node;)
	{
		QueuedSound* queuedSound = (QueuedSound*)node->data;

		if(!isDeleted(queuedSound))
		{
			Sound queuedSoundWrapper = SoundManager::doGetSound(this, queuedSound->soundSpec, queuedSound->command, queuedSound->soundReleaseListener, queuedSound->scope);

			if(!isDeleted(queuedSoundWrapper))
			{
				Sound::play(queuedSoundWrapper, queuedSound->isPositionValid ? &queuedSound->position : NULL, queuedSound->playbackType);

				VirtualNode auxNode = node;
				node = node->next;
				VirtualList::removeNode(this->queuedSounds, auxNode);
				delete queuedSound;

				continue;
			}
		}

		node = node->next;
	}
}

void SoundManager::update()
{
	SoundManager::purgeReleasedSoundWrappers(this);
	SoundManager::tryToPlayQueuedSounds(this);
}

bool SoundManager::isPlayingSound(const SoundSpec* soundSpec)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(soundSpec == sound->soundSpec)
		{
			return true;
		}
	}

	return false;
}

/* Static because it is more performant */
static void SoundManager::playSounds(uint32 elapsedMicroseconds)
{
	VirtualNode node = _soundManager->soundsMIDI->head;

	if(NULL != node)
	{
		if(0 < _soundManager->MIDIPlaybackCounterPerInterrupt)
		{		
			static uint32 accumulatedElapsedMicroseconds = 0;
			accumulatedElapsedMicroseconds += elapsedMicroseconds;

			if(NULL == _soundManager->soundMIDINode)
			{
				_soundManager->soundMIDINode = node;
				accumulatedElapsedMicroseconds = elapsedMicroseconds;
			}

			for(uint16 counter = _soundManager->MIDIPlaybackCounterPerInterrupt; counter-- && _soundManager->soundMIDINode;)
			{
				Sound::updateMIDIPlayback(Sound::safeCast(_soundManager->soundMIDINode->data), accumulatedElapsedMicroseconds);

				_soundManager->soundMIDINode = _soundManager->soundMIDINode->next;
			}
		}
		else
		{
			do
			{
				NM_ASSERT(NULL != node, "SoundManager::playMIDISounds: NULL node");
				NM_ASSERT(!isDeleted(node), "SoundManager::playMIDISounds: deleted node");
				NM_ASSERT(NULL != node->data, "SoundManager::playMIDISounds: NULL node data");
				NM_ASSERT(!isDeleted(node->data), "SoundManager::playMIDISounds: deleted node data");
				Sound::updateMIDIPlayback(Sound::safeCast(node->data), elapsedMicroseconds);

				node = node->next;
			}
			while(NULL != node);
		}
	}

	node = _soundManager->soundsPCM->head;

	if(NULL != node)
	{
		do
		{
			Sound::updatePCMPlayback(Sound::safeCast(node->data), elapsedMicroseconds, _soundManager->targetPCMUpdates);

			node = node->next;
		}
		while(NULL != node);
	}
}

void SoundManager::rewindAllSounds(uint32 type)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(sound->hasMIDITracks)
				{
					Sound::rewind(sound);
				}
				break;

			case kPCM:

				if(sound->hasPCMTracks)
				{
					Sound::rewind(sound);
				}
				break;

			default:

				NM_ASSERT(false, "SoundManager::rewindAllSounds: unknown track type");
				break;
		}
	}
}

void SoundManager::unmuteAllSounds(uint32 type)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(sound->hasMIDITracks)
				{
					Sound::unmute(sound);
				}
				break;

			case kPCM:

				if(sound->hasPCMTracks)
				{
					Sound::unmute(sound);
				}
				break;

			default:

				NM_ASSERT(false, "SoundManager::unmuteAllSounds: unknown track type");
				break;

		}
	}
}

void SoundManager::muteAllSounds(uint32 type)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(sound->hasMIDITracks)
				{
					Sound::mute(sound);
				}
				break;

			case kPCM:

				if(sound->hasPCMTracks)
				{
					Sound::mute(sound);
				}
				break;

			default:

				NM_ASSERT(false, "SoundManager::muteAllSounds: unknown track type");
				break;

		}
	}
}

int8 SoundManager::getWaveform(const int8* waveFormData)
{
	if(NULL == waveFormData)
	{
		return -1;
	}

	Waveform* freeWaveformPriority1 = NULL;
	Waveform* freeWaveformPriority2 = NULL;

	// Reset all sounds and channels
//	for(int16 i = __TOTAL_WAVEFORMS - 1; 0 <= i; i--)
	for(int32 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		if(NULL == this->waveforms[i].data)
		{
			freeWaveformPriority1 = &this->waveforms[i];
		}

		if(0 == this->waveforms[i].usageCount)
		{
			freeWaveformPriority2 = &this->waveforms[i];
		}

		if(waveFormData == this->waveforms[i].data)
		{
			this->waveforms[i].usageCount++;
			return this->waveforms[i].number;
		}
	}

	if(NULL != freeWaveformPriority1)
	{
		freeWaveformPriority1->overwrite = freeWaveformPriority1->data != waveFormData;
		freeWaveformPriority1->data = waveFormData;
		freeWaveformPriority1->usageCount = 1;

		return freeWaveformPriority1->number;
	}

	if(NULL != freeWaveformPriority2)
	{
		freeWaveformPriority2->overwrite = freeWaveformPriority2->data != waveFormData;
		freeWaveformPriority2->data = waveFormData;
		freeWaveformPriority2->usageCount = 1;

		return freeWaveformPriority2->number;
	}

	return -1;
}

void SoundManager::setWaveform(Waveform* waveform, const int8* data)
{
	if(NULL != waveform && waveform->overwrite)
	{
		waveform->data = (int8*)data;
		waveform->overwrite = false;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all soundSpec before writing the waveforms
		SoundManager::turnOffPlayingSounds(this);

		for(uint32 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (uint8)data[i];
		}

		// Resume playing sounds
		SoundManager::turnOnPlayingSounds(this);

		// Turn back interrupts on
		HardwareManager::resumeInterrupts();
		/*
		// TODO
		const uint8 kModData[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 18, 17, 18, 19, 20, 21, -1, -2, -3, -4, -5,
		-6, -7, -8, -9, -16, -17, -18, -19, -20, -21, -22
		};

		uint8* moddata = __MODULATION_DATA;
		for(i = 0; i <= 0x7C; i++)
		{
			moddata[i << 2] = kModData[i];
		}
		*/
	}
}

void SoundManager::releaseWaveform(int8 waveFormIndex, const int8* waveFormData)
{
	if(0 <= waveFormIndex && waveFormIndex < __TOTAL_CHANNELS)
	{
		if(NULL == waveFormData || this->waveforms[waveFormIndex].data == waveFormData)
		{
			this->waveforms[waveFormIndex].usageCount -= 1;

			if(0 >= this->waveforms[waveFormIndex].usageCount)
			{
				this->waveforms[waveFormIndex].usageCount = 0;
			}
		}
		else
		{
#ifndef __RELEASE
			Printing::setDebugMode(Printing::getInstance());
			Printing::clear(Printing::getInstance());
			Printing::text(Printing::getInstance(), "Waveform index: ", 1, 12, NULL);
			Printing::int32(Printing::getInstance(), waveFormIndex, 18, 12, NULL);
			Printing::text(Printing::getInstance(), "Waveform data: ", 1, 13, NULL);
			Printing::hex(Printing::getInstance(), (int32)waveFormData, 18, 13, 8, NULL);
			Printing::text(Printing::getInstance(), "Waveform data[]: ", 1, 14, NULL);
			Printing::hex(Printing::getInstance(), (int32)this->waveforms[waveFormIndex].data, 18, 14, 8, NULL);
#endif
			NM_ASSERT(false, "SoundManager::releaseWaveform: mismatch between index and data");
		}
	}
}

void SoundManager::releaseSoundChannel(Channel* channel)
{
	NM_ASSERT(NULL != channel, "SoundManager::releaseSoundChannel: NULL channel");

	if(NULL != channel)
	{
		if(kChannelNoise != channel->type)
		{
			SoundManager::releaseWaveform(this, channel->soundChannelConfiguration.SxRAM, !isDeleted(channel->soundSpec) ? channel->soundSpec->soundChannels[channel->soundChannel]->soundChannelConfiguration->waveFormData: NULL);
		}

		channel->soundChannelConfiguration.trackType = kUnknownType;
		channel->soundChannelConfiguration.SxINT = 0x00;
		channel->soundChannelConfiguration.SxLRV = 0x00;
		channel->soundChannelConfiguration.SxRAM = 0x00;
		channel->soundChannelConfiguration.SxEV0 = 0x00;
		channel->soundChannelConfiguration.SxEV1 = 0x00;
		channel->soundChannelConfiguration.SxFQH = 0x00;
		channel->soundChannelConfiguration.SxFQL = 0x00;
		channel->soundChannelConfiguration.S5SWP = 0x00;
		channel->soundChannelConfiguration.waveFormData = NULL;
		channel->soundChannelConfiguration.channelType = kChannelNormal;
		channel->soundChannelConfiguration.volume = 0x00;
		channel->soundSpec = NULL;
	}
}

void SoundManager::turnOffPlayingSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound) && !Sound::isPaused(sound))
		{
			Sound::turnOff(sound);
		}
	}

	for(int32 i = 0; i < __TOTAL_CHANNELS; i++)
	{
		_soundRegistries[i].SxINT = 0x00;
		_soundRegistries[i].SxLRV = 0x00;
	}
}

void SoundManager::turnOnPlayingSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound) && !Sound::isPaused(sound))
		{
			Sound::turnOn(sound);
		}
	}
}

static uint8 SoundManager::getSoundChannelsCount(const SoundSpec* soundSpec, uint32 channelType)
{
	// Compute the number of
	uint8 channelsCount = 0;

	for(uint32 i = 0; soundSpec->soundChannels[i] && i < __TOTAL_CHANNELS; i++)
	{
		if(channelType == soundSpec->soundChannels[i]->soundChannelConfiguration->channelType)
		{
			channelsCount++;
		}
	}

	return __TOTAL_CHANNELS < channelsCount ? __TOTAL_CHANNELS : channelsCount;
}

uint8 SoundManager::getFreeChannels(const SoundSpec* soundSpec, VirtualList availableChannels, uint8 channelsCount, uint32 channelType, bool force)
{
	if(NULL == soundSpec || isDeleted(availableChannels))
	{
		return 0;
	}

	uint16 i = 0;
	uint8 usableChannelsCount = 0;

	for(i = 0; usableChannelsCount < channelsCount && i < __TOTAL_CHANNELS; i++)
	{
		if(NULL == this->channels[i].soundSpec && (this->channels[i].type & channelType))
		{
			usableChannelsCount++;
			VirtualList::pushBack(availableChannels , &this->channels[i]);
		}
	}

	if(usableChannelsCount < channelsCount && force)
	{
		VirtualList::clear(availableChannels);
		
		usableChannelsCount = 0;

		VirtualList soundsToRelease = new VirtualList();

		for(i = 0; usableChannelsCount < channelsCount && i < __TOTAL_CHANNELS; i++)
		{
			if((this->channels[i].type & channelType))
			{
				if(NULL != this->channels[i].soundSpec)
				{
					for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
					{
						Sound sound = Sound::safeCast(node->data);

						if(!isDeleted(sound) && NULL != sound->soundSpec && !sound->locked)
						{
							if(Sound::isUsingChannel(sound, &this->channels[i]))
							{
								usableChannelsCount++;
								VirtualList::pushBack(availableChannels , &this->channels[i]);
								VirtualList::pushBack(soundsToRelease, sound);
								break;
							}
						}
					}					
				}
				else
				{
					usableChannelsCount++;
					VirtualList::pushBack(availableChannels , &this->channels[i]);
				}
			}
		}

		if(usableChannelsCount >= channelsCount)
		{
			for(VirtualNode node = soundsToRelease->head; NULL != node; node = node->next)
			{
				Sound::release(Sound::safeCast(node->data));
			}
		}

		delete soundsToRelease;
	}

	return usableChannelsCount;
}

void SoundManager::lock()
{
	this->lock = true;
}

void SoundManager::unlock()
{
	this->lock = false;
}

bool SoundManager::playSound(const SoundSpec* soundSpec, uint32 command, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, ListenerObject scope)
{
	if(this->lock || NULL == soundSpec)
	{
		return false;
	}

	Sound sound = SoundManager::doGetSound(this, soundSpec, command, soundReleaseListener, scope);

	if(!isDeleted(sound))
	{
		Sound::play(sound, position, playbackType);

		return true;
	}
	else if(kPlayAsSoonAsPossible == playbackType)
	{
		QueuedSound* queuedSound = new QueuedSound;
		queuedSound->soundSpec = soundSpec;
		queuedSound->command = command;
		queuedSound->isPositionValid = NULL != position;
		queuedSound->position = queuedSound->isPositionValid ? *position : Vector3D::zero();
		queuedSound->playbackType = playbackType;
		queuedSound->soundReleaseListener = soundReleaseListener;
		queuedSound->scope = scope;

		VirtualList::pushBack(this->queuedSounds, queuedSound);
	}

	return kPlayForceAll != playbackType;
}

/**
 * Find a previously loaded soundSpec
 *
 * @param soundSpec		SoundSpec*
 */
Sound SoundManager::findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	if(NULL == soundReleaseListener || NULL == scope)
	{
		return NULL;
	}

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			if(soundSpec == sound->soundSpec)
			{
				Sound::addEventListener(sound, scope, soundReleaseListener, kEventSoundReleased);
				return sound;
			}
		}
	}

	return NULL;
}

/**
 * Request a new soundSpec
 *
 * @param soundSpec		SoundSpec*
 */
Sound SoundManager::getSound(const SoundSpec* soundSpec, uint32 command, EventListener soundReleaseListener, ListenerObject scope)
{
	if(this->lock || NULL == soundReleaseListener || NULL == scope)
	{
		return NULL;
	}

	return SoundManager::doGetSound(this, soundSpec, command, soundReleaseListener, scope);
}

Sound SoundManager::doGetSound(const SoundSpec* soundSpec, uint32 command, EventListener soundReleaseListener, ListenerObject scope)
{
	if(NULL == soundSpec)
	{
		return NULL;
	}

	// Compute the number of
	uint8 normalChannelsCount = SoundManager::getSoundChannelsCount(soundSpec, kChannelNormal);
	uint8 modulationChannelsCount = SoundManager::getSoundChannelsCount(soundSpec, kChannelModulation);
	uint8 noiseChannelsCount = SoundManager::getSoundChannelsCount(soundSpec, kChannelNoise);

	// Check for free channels
	VirtualList availableChannels = new VirtualList();

	uint8 usableNormalChannelsCount = SoundManager::getFreeChannels(this, soundSpec, availableChannels, normalChannelsCount, kChannelNormal | (0 == modulationChannelsCount ? kChannelModulation : kChannelNormal), kPlayForceAll == command);
	uint8 usableModulationChannelsCount = SoundManager::getFreeChannels(this, soundSpec, availableChannels, modulationChannelsCount, kChannelModulation, kPlayForceAll == command);
	uint8 usableNoiseChannelsCount = SoundManager::getFreeChannels(this, soundSpec, availableChannels, noiseChannelsCount, kChannelNoise, kPlayForceAll == command);

	if(kPlayAll != command && kPlayAsSoonAsPossible != command)
	{
		NM_ASSERT(0 == normalChannelsCount || normalChannelsCount <= usableNormalChannelsCount, "SoundManager::getSound: not enough normal channels");
		NM_ASSERT(0 == modulationChannelsCount || 0 < usableModulationChannelsCount, "SoundManager::getSound: not enough modulation channels");
		NM_ASSERT(0 == noiseChannelsCount || 0 < usableNoiseChannelsCount, "SoundManager::getSound: not enough noise channels");
	}

	Sound sound = NULL;

	/* TODO
	if(forceAllChannels)
	{
	}
	// If there are enough usable channels
	else */
	switch(command)
	{
		case kPlayAll:
		case kPlayAsSoonAsPossible:
		case kPlayForceAll:

			if(normalChannelsCount <= usableNormalChannelsCount && modulationChannelsCount <= usableModulationChannelsCount && noiseChannelsCount <= usableNoiseChannelsCount)

			{
				int8 waves[__TOTAL_WAVEFORMS] = {-1, -1, -1, -1, -1};

				uint16 i = 0;

				if(NULL != soundSpec->soundChannels[i]->soundChannelConfiguration->waveFormData)
				{
					for(i = 0; i < normalChannelsCount + modulationChannelsCount; i++)
					{
						if(kChannelNoise != soundSpec->soundChannels[i]->soundChannelConfiguration->channelType)
						{
							waves[i] = SoundManager::getWaveform(this, soundSpec->soundChannels[i]->soundChannelConfiguration->waveFormData);
							if(0 > waves[i])
							{
								NM_ASSERT(false, "No wave found");
								delete availableChannels;
								return NULL;
							}
						}
					}
				}

				for(i = 0; i < normalChannelsCount + modulationChannelsCount; i++)
				{
					if(kChannelNoise != soundSpec->soundChannels[i]->soundChannelConfiguration->channelType)
					{
						SoundManager::setWaveform(this, &this->waveforms[waves[i]], soundSpec->soundChannels[i]->soundChannelConfiguration->waveFormData);
					}
				}

				//NM_ASSERT(0 < VirtualList::getSize(availableChannels), "SoundManager::getSound: 0 availableNormalChannels");

				if(0 < VirtualList::getSize(availableChannels))
				{
					sound = new Sound(soundSpec, availableChannels, waves, this->pcmTargetPlaybackFrameRate, soundReleaseListener, scope);

					VirtualList::pushBack(this->sounds, sound);

					if(sound->hasMIDITracks)
					{
						VirtualList::pushBack(this->soundsMIDI, sound);
					}

					if(sound->hasPCMTracks)
					{
						VirtualList::pushBack(this->soundsPCM, sound);
					}
				}
			}
			break;

		case kPlayAny:
		case kPlayForceAny:

			break;
	}

	if(isDeleted(sound))
	{
		delete availableChannels;
	}

	return sound;
}

/**
 * Stop all soundSpec playback
 */
void SoundManager::stopAllSounds(bool release, SoundSpec** excludedSounds)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			bool excludeSoundWrapper = false;

			if(NULL != excludedSounds)
			{
				for(int16 i = 0; NULL != excludedSounds[i]; i++)
				{
					if(excludedSounds[i] == sound->soundSpec)
					{
						if(release)
						{
							Sound::removeAllEventListeners(sound);
						}

						excludeSoundWrapper = true;

						break;
					}
				}
			}

			if(!excludeSoundWrapper)
			{
				if(release)
				{
					Sound::release(sound);
				}
				else
				{
					Sound::stop(sound);
				}
			}
		}
	}

	if(release)
	{
		SoundManager::purgeReleasedSoundWrappers(this);
	}

	if(NULL == excludedSounds)
	{
		__SSTOP = 0x01;
	}
}

#ifndef __SHIPPING
void SoundManager::print()
{
	int32 x = 1;
	int32 xDisplacement = 8;
	int32 yDisplacement = 0;

	int32 i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		int32 y = yDisplacement;

		PRINT_TEXT("CHANNEL ", x, y);
		PRINT_INT(this->channels[i].number + 1, x + xDisplacement, y);

		PRINT_TEXT("Type   : ", x, ++y);

		char* channelType = "?";
		switch(this->channels[i].type)
		{
			case kChannelNormal:

				channelType = "Normal";
				break;

			case kChannelModulation:

				channelType = "Modulation";
				break;

			case kChannelNoise:

				channelType = "Noise";
				break;
		}
		PRINT_TEXT(channelType, x + xDisplacement, y);

		PRINT_TEXT("Track  :     ", x, ++y);

		char* soundType = "?";
		switch(this->channels[i].soundChannelConfiguration.trackType)
		{
			case kMIDI:

				soundType = "MIDI";
				break;

			case kPCM:

				soundType = "PCM";
				break;
		}

		PRINT_TEXT(soundType, x + xDisplacement, y);

		PRINT_TEXT("Cursor :        ", x, ++y);
		PRINT_INT(this->channels[i].cursor, x + xDisplacement, y);

		PRINT_TEXT("Snd Ch :     ", x, ++y);
		PRINT_INT(this->channels[i].soundChannel, x + xDisplacement, y);

		PRINT_TEXT("SxRAM  :     ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxRAM, x + xDisplacement, y, 2);
		PRINT_TEXT("INT/LRV:  /   ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxINT | (NULL == this->channels[i].soundSpec ? 0 : 0x80), x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxLRV, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("EV0/EV1:  /   ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV1, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("FQH/FQL:  /   ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQL, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("Loop   :      ", x, ++y);
		PRINT_TEXT(this->channels[i].soundSpec->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

		PRINT_TEXT("Samples :      ", x, ++y);
		PRINT_INT(this->channels[i].samples, x + xDisplacement, y);

		PRINT_TEXT("Note   :     ", x, ++y);
		switch(this->channels[i].soundChannelConfiguration.trackType)
		{
			case kMIDI:

				PRINT_HEX_EXT(this->channels[i].soundSpec->soundChannels[this->channels[i].soundChannel]->soundTrack.dataMIDI[this->channels[i].cursor], x + xDisplacement, y, 2);
				break;

			case kPCM:

				PRINT_HEX_EXT(this->channels[i].soundSpec->soundChannels[this->channels[i].soundChannel]->soundTrack.dataPCM[this->channels[i].cursor], x + xDisplacement, y, 2);
				break;
		}

		x += 16;

		if(x > 33)
		{
			x = 1;
			yDisplacement += 15;
		}
	}
}
#endif

#ifndef __RELEASE
void SoundManager::printWaveFormStatus(int32 x, int32 y)
{
	// Reset all waveforms
	for(uint32 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		PRINT_TEXT("           ", x, y + this->waveforms[i].number);
		PRINT_INT(this->waveforms[i].number, x, y + this->waveforms[i].number);
		PRINT_INT(this->waveforms[i].usageCount, x + 4, y + this->waveforms[i].number);
		PRINT_HEX((uint32)this->waveforms[i].data, x + 8, y + this->waveforms[i].number);
	}
}
#endif

#ifdef __SOUND_TEST
void SoundManager::printPlaybackTime()
{
	VirtualNode node = this->sounds->head;

	if(!isDeleted(node))
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			Sound::printPlaybackTime(sound, 24, 8);
		}
	}
}
#endif