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

#include <SoundManager.h>
#include <Optics.h>
#include <VirtualList.h>
#include <TimerManager.h>
#include <Game.h>
#include <Profiler.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#undef __SOUND_MANAGER_PROFILE


//---------------------------------------------------------------------------------------------------------
//											 CLASS' DEFINITIONS
//---------------------------------------------------------------------------------------------------------

SoundRegistry* const _soundRegistries =	(SoundRegistry*)0x01000400; //(SoundRegistry*)0x010003C0;

#define __WAVE_ADDRESS(n)			(uint8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA			(uint8*)0x01000280;
#define __SSTOP						*(uint8*)0x01000580


//---------------------------------------------------------------------------------------------------------
//												 FRIENDS
//---------------------------------------------------------------------------------------------------------

friend class SoundWrapper;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------


typedef struct QueuedSound
{
	const Sound* sound;
	uint32 command;
	Vector3D position;
	bool isPositionValid;
	uint32 playbackType;
	EventListener soundReleaseListener;
	Object scope;

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
	Base::constructor();

	this->soundWrappers = new VirtualList();
	this->queuedSounds = new VirtualList();
	this->hasPCMSounds = false;
	this->MIDIPlaybackCounterPerInterrupt = false;
	this->soundWrapperMIDINode = NULL;
	this->lock = false;
}

/**
 * Class destructor
 */
void SoundManager::destructor()
{
	if(!isDeleted(this->queuedSounds))
	{
		VirtualNode node = this->queuedSounds->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->queuedSounds;
		this->queuedSounds = NULL;
	}

	if(!isDeleted(this->soundWrappers))
	{
		VirtualNode node = this->soundWrappers->head;

		for(; node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			VirtualNode auxNode = soundWrapper->channels->head;

			for(; auxNode; auxNode = auxNode->next)
			{
				Channel* channel = (Channel*)auxNode->data;

				SoundManager::releaseSoundChannel(this, channel);
			}

			delete soundWrapper;
		}

		delete this->soundWrappers;
		this->soundWrappers = NULL;
	}

	Base::destructor();
}

void SoundManager::purgeReleasedSoundWrappers()
{
	for(VirtualNode node = this->soundWrappers->head, nextNode = NULL; node; node = nextNode)
	{
		nextNode = node->next;

		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(soundWrapper->released)
		{
			// Release soundWrapper's channels
			VirtualNode auxNode = soundWrapper->channels->head;

			for(; auxNode; auxNode = auxNode->next)
			{
				Channel* channel = (Channel*)auxNode->data;

				SoundManager::releaseSoundChannel(this, channel);
			}

			VirtualList::removeNode(this->soundWrappers, node);

			delete soundWrapper;

			this->soundWrapperMIDINode = NULL;
		}
	}
}

void SoundManager::reset()
{
	for(VirtualNode node = this->queuedSounds->head; node; node = node->next)
	{
		delete node->data;
	}

	VirtualList::clear(this->queuedSounds);

	for(VirtualNode node = this->soundWrappers->head; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		NM_ASSERT(!isDeleted(soundWrapper), "SoundManager::reset: deleted sound wrapper");

		VirtualNode auxNode = soundWrapper->channels->head;

		for(; auxNode; auxNode = auxNode->next)
		{
			Channel* channel = (Channel*)auxNode->data;

			SoundManager::releaseSoundChannel(this, channel);
		}

		delete soundWrapper;
	}

	VirtualList::clear(this->soundWrappers);

	int32 i = 0;

	// Reset all channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		this->channels[i].number = i;
		this->channels[i].sound = NULL;
		this->channels[i].cursor = 0;
		this->channels[i].ticks = 0;
		this->channels[i].ticksPerNote = 0;
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

	this->pcmPlaybackCycles = 0;
	this->pcmPlaybackCyclesToSkip = 0;
	this->pcmTargetPlaybackFrameRate = __DEFAULT_PCM_HZ;
	this->MIDIPlaybackCounterPerInterrupt = 0;
	this->soundWrapperMIDINode = NULL;

	SoundManager::stopAllSounds(this, false);
	SoundManager::unlock(this);
}

void SoundManager::deferMIDIPlayback(uint32 MIDIPlaybackCounterPerInterrupt)
{
	this->MIDIPlaybackCounterPerInterrupt = MIDIPlaybackCounterPerInterrupt;
}

void SoundManager::startPCMPlayback()
{
	this->pcmPlaybackCycles = 0;
	this->pcmPlaybackCyclesToSkip = 100;

	SoundManager::muteAllSounds(this, kPCM);
}

void SoundManager::setTargetPlaybackFrameRate(uint16 pcmTargetPlaybackFrameRate)
{
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;
}

void SoundManager::flushQueuedSounds()
{
	for(VirtualNode node = this->queuedSounds->head; node; node = node->next)
	{
		delete node->data;
	}

	VirtualList::clear(this->queuedSounds);
}

void SoundManager::tryToPlayQueuedSounds()
{
	for(VirtualNode node = this->queuedSounds->head; node;)
	{
		QueuedSound* queuedSound = (QueuedSound*)node->data;

		if(!isDeleted(queuedSound))
		{
			SoundWrapper queuedSoundWrapper = SoundManager::doGetSound(this, queuedSound->sound, queuedSound->command, queuedSound->soundReleaseListener, queuedSound->scope);

			if(!isDeleted(queuedSoundWrapper))
			{
				SoundWrapper::play(queuedSoundWrapper, queuedSound->isPositionValid ? &queuedSound->position : NULL, queuedSound->playbackType);

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

bool SoundManager::isPlayingSound(const Sound* sound)
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(sound == soundWrapper->sound)
		{
			return true;
		}
	}

	return false;
}

bool SoundManager::playMIDISounds(uint32 elapsedMicroseconds)
{
	if(0 < this->MIDIPlaybackCounterPerInterrupt)
	{
		static uint32 accumulatedElapsedMicroseconds = 0;
		accumulatedElapsedMicroseconds += elapsedMicroseconds;

		if(NULL == this->soundWrapperMIDINode)
		{
			this->soundWrapperMIDINode = this->soundWrappers->head;
			accumulatedElapsedMicroseconds = elapsedMicroseconds;
		}

		uint16 counter = this->MIDIPlaybackCounterPerInterrupt;

		for(; counter-- && this->soundWrapperMIDINode;)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(this->soundWrapperMIDINode->data);

			if(soundWrapper->hasMIDITracks)
			{
				SoundWrapper::updateMIDIPlayback(soundWrapper, accumulatedElapsedMicroseconds);
			}

			this->soundWrapperMIDINode = this->soundWrapperMIDINode->next;
		}
	}
	else
	{
		VirtualNode node = this->soundWrappers->head;

		for(; node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			if(soundWrapper->hasMIDITracks)
			{
				SoundWrapper::updateMIDIPlayback(soundWrapper, elapsedMicroseconds);
			}
		}
	}

	return true;
}

bool SoundManager::playPCMSounds()
{
	if(!this->hasPCMSounds)
	{
		return false;
	}

	// Gives good results on hardware
	// Do not waste CPU cycles returning to the call point
	volatile uint16 pcmReimainingPlaybackCyclesToSkip = this->pcmPlaybackCyclesToSkip;
	while(0 < --pcmReimainingPlaybackCyclesToSkip);

	this->pcmPlaybackCycles++;

	VirtualNode node = this->soundWrappers->head;

	this->hasPCMSounds = false;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(soundWrapper->hasPCMTracks)
		{
			this->hasPCMSounds = true;
			SoundWrapper::updatePCMPlayback(soundWrapper, 0);
		}
	}

	return true;
}

void SoundManager::updateFrameRate()
{
	if(!this->hasPCMSounds)
	{
		return;
	}

	int16 deviation = (this->pcmPlaybackCycles - this->pcmTargetPlaybackFrameRate/ (__MILLISECONDS_PER_SECOND / __GAME_FRAME_DURATION));

	if(2 < deviation)
	{
		deviation = 2;
	}
	else if(-2 > deviation)
	{
		deviation = -2;
	}

	// Dubious optimization
	this->pcmPlaybackCyclesToSkip += deviation;

	if(0 > this->pcmPlaybackCyclesToSkip)
	{
		this->pcmPlaybackCyclesToSkip = 1;
	}

#ifdef __SOUND_MANAGER_PROFILE
	static uint16 counter = 20;

	if(++counter > 20)
	{
		counter = 0;
		PRINT_TEXT("    ", 35, 20);
		PRINT_INT(this->pcmPlaybackCyclesToSkip, 35, 20);
	//	PRINT_INT(this->pcmPlaybackCycles, 40, 20);
	}
#endif
	this->pcmPlaybackCycles = 0;
}

void SoundManager::rewindAllSounds(uint32 type)
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(soundWrapper->hasMIDITracks)
				{
					SoundWrapper::rewind(soundWrapper);
				}
				break;

			case kPCM:

				if(soundWrapper->hasPCMTracks)
				{
					SoundWrapper::rewind(soundWrapper);
				}
				break;

			default:

				NM_ASSERT(false, "SoundManager::playSounds: unknown track type");
				break;

		}
	}
}

void SoundManager::unmuteAllSounds(uint32 type)
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(soundWrapper->hasMIDITracks)
				{
					SoundWrapper::unmute(soundWrapper);
				}
				break;

			case kPCM:

				if(soundWrapper->hasPCMTracks)
				{
					SoundWrapper::unmute(soundWrapper);
				}
				break;

			default:

				NM_ASSERT(false, "SoundManager::muteAllSounds: unknown track type");
				break;

		}
	}
}

void SoundManager::muteAllSounds(uint32 type)
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		switch(type)
		{
			case kMIDI:

				if(soundWrapper->hasMIDITracks)
				{
					SoundWrapper::mute(soundWrapper);
				}
				break;

			case kPCM:

				if(soundWrapper->hasPCMTracks)
				{
					SoundWrapper::mute(soundWrapper);
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

		// Must stop all sound before writing the waveforms
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
	if(channel)
	{
		if(kChannelNoise != channel->type)
		{
			SoundManager::releaseWaveform(this, channel->soundChannelConfiguration.SxRAM, !isDeleted(channel->sound) ? channel->sound->soundChannels[channel->soundChannel]->soundChannelConfiguration->waveFormData: NULL);
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
		channel->sound = NULL;
	}
}

void SoundManager::turnOffPlayingSounds()
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(!isDeleted(soundWrapper) && !SoundWrapper::isPaused(soundWrapper))
		{
			SoundWrapper::turnOff(soundWrapper);
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
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(!isDeleted(soundWrapper) && !SoundWrapper::isPaused(soundWrapper))
		{
			SoundWrapper::turnOn(soundWrapper);
		}
	}
}

static uint8 SoundManager::getSoundChannelsCount(const Sound* sound, uint32 channelType)
{
	// Compute the number of
	uint8 channelsCount = 0;

	for(uint32 i = 0; sound->soundChannels[i] && i < __TOTAL_CHANNELS; i++)
	{
		if(channelType == sound->soundChannels[i]->soundChannelConfiguration->channelType)
		{
			channelsCount++;
		}
	}

	return __TOTAL_CHANNELS < channelsCount ? __TOTAL_CHANNELS : channelsCount;
}

uint8 SoundManager::getFreeChannels(const Sound* sound, VirtualList availableChannels, uint8 channelsCount, uint32 channelType)
{
	if(NULL == sound || isDeleted(availableChannels))
	{
		return 0;
	}

	uint16 i = 0;
	uint8 usableChannelsCount = 0;

	for(i = 0; usableChannelsCount < channelsCount && i < __TOTAL_CHANNELS; i++)
	{
		if(NULL == this->channels[i].sound && (this->channels[i].type & channelType))
		{
			usableChannelsCount++;
			VirtualList::pushBack(availableChannels , &this->channels[i]);
		}
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

void SoundManager::playSound(const Sound* sound, uint32 command, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, Object scope)
{
	if(this->lock || NULL == sound)
	{
		return;
	}

	SoundWrapper soundWrapper = SoundManager::getSound(this, sound, command, soundReleaseListener, scope);

	if(!isDeleted(soundWrapper))
	{
		SoundWrapper::play(soundWrapper, position, playbackType);
	}
	else
	{
		QueuedSound* queuedSound = new QueuedSound;
		queuedSound->sound = sound;
		queuedSound->command = command;
		queuedSound->isPositionValid = NULL != position;
		queuedSound->position = queuedSound->isPositionValid ? *position : Vector3D::zero();
		queuedSound->playbackType = playbackType;
		queuedSound->soundReleaseListener = soundReleaseListener;
		queuedSound->scope = scope;

		VirtualList::pushBack(this->queuedSounds, queuedSound);
	}
}

/**
 * Request a new sound
 *
 * @param sound		Sound*
 */
SoundWrapper SoundManager::getSound(const Sound* sound, uint32 command, EventListener soundReleaseListener, Object scope)
{
	if(this->lock)
	{
		return NULL;
	}

	return SoundManager::doGetSound(this, sound, command, soundReleaseListener, scope);
}

SoundWrapper SoundManager::doGetSound(const Sound* sound, uint32 command, EventListener soundReleaseListener, Object scope)
{
	if(NULL == sound)
	{
		return NULL;
	}

	// Compute the number of
	uint8 normalChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNormal);
	uint8 modulationChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelModulation);
	uint8 noiseChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNoise);
	uint8 normalExtendeChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNormalExtended);

	// Check for free channels
	VirtualList availableChannels  = new VirtualList();

	uint8 usableNormalChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, normalChannelsCount, kChannelNormal | (normalExtendeChannelsCount && 0 == modulationChannelsCount ? kChannelModulation : kChannelNormal));
	uint8 usableModulationChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, modulationChannelsCount, kChannelModulation);
	uint8 usableNoiseChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, noiseChannelsCount, kChannelNoise);

	if(kPlayAll != command)
	{
		NM_ASSERT(0 == normalChannelsCount || normalChannelsCount <= usableNormalChannelsCount, "SoundManager::getSound: not enough normal channels");
		NM_ASSERT(0 == modulationChannelsCount || 0 < usableModulationChannelsCount, "SoundManager::getSound: not enough modulation channels");
		NM_ASSERT(0 == noiseChannelsCount || 0 < usableNoiseChannelsCount, "SoundManager::getSound: not enough noise channels");
	}

	SoundWrapper soundWrapper = NULL;

	/* TODO
	if(forceAllChannels)
	{
	}
	// If there are enough usable channels
	else */
	switch(command)
	{
		case kPlayAll:

			if(normalChannelsCount <= usableNormalChannelsCount && modulationChannelsCount <= usableModulationChannelsCount && noiseChannelsCount <= usableNoiseChannelsCount)
			{
				int8 waves[__TOTAL_WAVEFORMS] = {-1, -1, -1, -1, -1};

				uint16 i = 0;

				if(NULL != sound->soundChannels[i]->soundChannelConfiguration->waveFormData)
				{
					for(i = 0; i < normalChannelsCount + modulationChannelsCount; i++)
					{
						if(kChannelNoise != sound->soundChannels[i]->soundChannelConfiguration->channelType)
						{
							waves[i] = SoundManager::getWaveform(this, sound->soundChannels[i]->soundChannelConfiguration->waveFormData);
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
					if(kChannelNoise != sound->soundChannels[i]->soundChannelConfiguration->channelType)
					{
						SoundManager::setWaveform(this, &this->waveforms[waves[i]], sound->soundChannels[i]->soundChannelConfiguration->waveFormData);
					}
				}

				//NM_ASSERT(0 < VirtualList::getSize(availableChannels), "SoundManager::getSound: 0 availableNormalChannels");

				if(0 < VirtualList::getSize(availableChannels))
				{
					soundWrapper = new SoundWrapper(sound, availableChannels, waves, this->pcmTargetPlaybackFrameRate, soundReleaseListener, scope);

					VirtualList::pushBack(this->soundWrappers, soundWrapper);
				}
			}
			break;

		case kPlayAny:
		case kPlayForceAny:
		case kPlayForceAll:

			break;
	}

	if(!isDeleted(soundWrapper))
	{
		this->hasPCMSounds |= soundWrapper->hasPCMTracks;
	}
	else
	{
		delete availableChannels;
	}

	return soundWrapper;
}

/**
 * Stop all sound playback
 */
void SoundManager::stopAllSounds(bool release)
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(!isDeleted(soundWrapper))
		{
			if(release)
			{
				SoundWrapper::release(soundWrapper);
			}
			else
			{
				SoundWrapper::stop(soundWrapper);
			}
		}
	}

	if(release)
	{
		SoundManager::purgeReleasedSoundWrappers(this);
	}

	__SSTOP = 0x01;
}

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
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxINT | (NULL == this->channels[i].sound ? 0 : 0x80), x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxLRV, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("EV0/EV1:  /   ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV1, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("FQH/FQL:  /   ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQL, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("Loop   :      ", x, ++y);
		PRINT_TEXT(this->channels[i].sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

		PRINT_TEXT("Length :      ", x, ++y);
		PRINT_INT(this->channels[i].length, x + xDisplacement, y);

		PRINT_TEXT("Note   :     ", x, ++y);
		switch(this->channels[i].soundChannelConfiguration.trackType)
		{
			case kMIDI:

				PRINT_HEX_EXT(this->channels[i].sound->soundChannels[this->channels[i].soundChannel]->soundTrack.dataMIDI[this->channels[i].cursor], x + xDisplacement, y, 2);
				break;

			case kPCM:

				PRINT_HEX_EXT(this->channels[i].sound->soundChannels[this->channels[i].soundChannel]->soundTrack.dataPCM[this->channels[i].cursor], x + xDisplacement, y, 2);
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

#ifdef __SOUND_TEST
void SoundManager::printPlaybackTime()
{
	VirtualNode node = this->soundWrappers->head;

	if(!isDeleted(node))
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(!isDeleted(soundWrapper))
		{
			SoundWrapper::printPlaybackTime(soundWrapper, 24, 8);
		}
	}
}
#endif