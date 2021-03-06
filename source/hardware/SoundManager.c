/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

#define __WAVE_ADDRESS(n)			(u8*)(0x01000000 + (n * 128))
#define __MODULATION_DATA			(u8*)0x01000280;
#define __SSTOP						*(u8*)0x01000580


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
	Sound* sound;
	u32 command;
	Vector3D position;
	bool isPositionValid;
	u32 playbackType;
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

	this->soundWrappers = NULL;
	this->releasedSoundWrappers = NULL;
	this->hasPCMSounds = false;
	this->MIDIPlaybackCounterPerInterrupt = false;
	this->soundWrapperMIDINode = NULL;
	this->lock = false;
	this->queuedSounds = new VirtualList();

	SoundManager::reset(this);
}

/**
 * Class destructor
 */
void SoundManager::destructor()
{
	SoundManager::purgeReleasedSoundWrappers(this);

	if(!isDeleted(this->releasedSoundWrappers))
	{
		delete this->releasedSoundWrappers;
		this->releasedSoundWrappers = NULL;
	}

	if(!isDeleted(this->queuedSounds))
	{
		VirtualNode node = this->queuedSounds->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->queuedSounds;
		this->soundWrappers = NULL;
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
	if(!isDeleted(this->releasedSoundWrappers))
	{
		VirtualNode node = this->releasedSoundWrappers->head;

		for(; node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			this->soundWrapperMIDINode = NULL;

			VirtualList::removeElement(this->soundWrappers, soundWrapper);

			delete soundWrapper;
		}

		VirtualList::clear(this->releasedSoundWrappers);
	}
}

void SoundManager::reset()
{
	SoundManager::purgeReleasedSoundWrappers(this);

	if(!isDeleted(this->queuedSounds))
	{
		VirtualNode node = this->queuedSounds->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		VirtualList::clear(this->queuedSounds);
	}

	if(!isDeleted(this->soundWrappers))
	{
		VirtualNode node = this->soundWrappers->head;

		for(; node; node = node->next)
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

		delete this->soundWrappers;
		this->soundWrappers = NULL;
	}

	this->soundWrappers = new VirtualList();

	if(!isDeleted(this->releasedSoundWrappers))
	{
		delete this->releasedSoundWrappers;
		this->releasedSoundWrappers = NULL;
	}

	this->releasedSoundWrappers = new VirtualList();

	int i = 0;

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

		for(u16 j = 0; j < 128; j++)
		{
			this->waveforms[i].wave[j] = 0;
		}
	}

	// Reset modulation data
	u8* modulationData = __MODULATION_DATA;
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

void SoundManager::deferMIDIPlayback(u32 MIDIPlaybackCounterPerInterrupt)
{
	this->MIDIPlaybackCounterPerInterrupt = MIDIPlaybackCounterPerInterrupt;
}

void SoundManager::startPCMPlayback()
{
	this->pcmPlaybackCycles = 0;
	this->pcmPlaybackCyclesToSkip = 100;

	SoundManager::muteAllSounds(this, kPCM);
}

void SoundManager::setTargetPlaybackFrameRate(u16 pcmTargetPlaybackFrameRate)
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
	this->lockSoundWrappersList = true;	
	SoundManager::purgeReleasedSoundWrappers(this);

	SoundManager::tryToPlayQueuedSounds(this);
	this->lockSoundWrappersList = false;	
}

bool SoundManager::playMIDISounds(u32 elapsedMicroseconds)
{
	bool lockSoundWrappersList = this->lockSoundWrappersList;	

	this->lockSoundWrappersList = true;	

	if(0 < this->MIDIPlaybackCounterPerInterrupt)
	{
		static u32 accumulatedElapsedMicroseconds = 0;
		accumulatedElapsedMicroseconds += elapsedMicroseconds;

		if(NULL == this->soundWrapperMIDINode)
		{
			this->soundWrapperMIDINode = this->soundWrappers->head;
			accumulatedElapsedMicroseconds = elapsedMicroseconds;
		}

		u16 counter = this->MIDIPlaybackCounterPerInterrupt;

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

	this->lockSoundWrappersList = lockSoundWrappersList;

	return true;
}

bool SoundManager::playPCMSounds()
{
	if(!this->hasPCMSounds)
	{
		return false;
	}

	this->lockSoundWrappersList = true;	

	// Gives good results on hardware
	// Do not waste CPU cycles returning to the call point
	volatile u16 pcmReimainingPlaybackCyclesToSkip = this->pcmPlaybackCyclesToSkip;
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

	this->lockSoundWrappersList = false;

	return true;
}

void SoundManager::updateFrameRate()
{
	if(!this->hasPCMSounds)
	{
		return;
	}

	s16 deviation = (this->pcmPlaybackCycles - this->pcmTargetPlaybackFrameRate/ (__MILLISECONDS_PER_SECOND / __GAME_FRAME_DURATION));

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
	static u16 counter = 20;

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

void SoundManager::rewindAllSounds(u32 type)
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

void SoundManager::unmuteAllSounds(u32 type)
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

void SoundManager::muteAllSounds(u32 type)
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

s8 SoundManager::getWaveform(const s8* waveFormData)
{
	if(NULL == waveFormData)
	{
		return -1;
	}

	Waveform* freeWaveformPriority1 = NULL;
	Waveform* freeWaveformPriority2 = NULL;

	// Reset all sounds and channels
//	for(s16 i = __TOTAL_WAVEFORMS - 1; 0 <= i; i--)
	for(s16 i = 0; i < __TOTAL_WAVEFORMS; i++)
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

void SoundManager::setWaveform(Waveform* waveform, const s8* data)
{
	if(NULL != waveform && waveform->overwrite)
	{
		waveform->data = (s8*)data;
		waveform->overwrite = false;

		// Disable interrupts to make the following as soon as possible
		HardwareManager::suspendInterrupts();

		// Must stop all sound before writing the waveforms
		SoundManager::turnOffPlayingSounds(this);

		for(u16 i = 0; i < 32; i++)
		{
			waveform->wave[(i << 2)] = (u8)data[i];
		}

		// Resume playing sounds
		SoundManager::turnOnPlayingSounds(this);

		// Turn back interrupts on
		HardwareManager::resumeInterrupts();
		/*
		// TODO
		const u8 kModData[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 18, 17, 18, 19, 20, 21, -1, -2, -3, -4, -5,
		-6, -7, -8, -9, -16, -17, -18, -19, -20, -21, -22
		};

		u8* moddata = __MODULATION_DATA;
		for(i = 0; i <= 0x7C; i++)
		{
			moddata[i << 2] = kModData[i];
		}
		*/
	}
}

void SoundManager::releaseWaveform(s8 waveFormIndex, const s8* waveFormData)
{
	if(0 <= waveFormIndex && waveFormIndex < __TOTAL_CHANNELS)
	{
		if(this->waveforms[waveFormIndex].data == waveFormData)
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
			Printing::int(Printing::getInstance(), waveFormIndex, 18, 12, NULL);
			Printing::text(Printing::getInstance(), "Waveform data: ", 1, 13, NULL);
			Printing::hex(Printing::getInstance(), (int)waveFormData, 18, 13, 8, NULL);
			Printing::text(Printing::getInstance(), "Waveform data[]: ", 1, 14, NULL);
			Printing::hex(Printing::getInstance(), (int)this->waveforms[waveFormIndex].data, 18, 14, 8, NULL);
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
			SoundManager::releaseWaveform(this, channel->soundChannelConfiguration.SxRAM, channel->sound->soundChannels[channel->soundChannel]->soundChannelConfiguration->waveFormData);
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

	for(int i = 0; i < __TOTAL_CHANNELS; i++)
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

static u8 SoundManager::getSoundChannelsCount(Sound* sound, u32 channelType)
{
	// Compute the number of
	u8 channelsCount = 0;

	for(u16 i = 0; sound->soundChannels[i] && i < __TOTAL_CHANNELS; i++)
	{
		if(channelType == sound->soundChannels[i]->soundChannelConfiguration->channelType)
		{
			channelsCount++;
		}
	}

	return __TOTAL_CHANNELS < channelsCount ? __TOTAL_CHANNELS : channelsCount;
}

u8 SoundManager::getFreeChannels(Sound* sound, VirtualList availableChannels, u8 channelsCount, u32 channelType)
{
	if(NULL == sound || isDeleted(availableChannels))
	{
		return 0;
	}

	u16 i = 0;
	u8 usableChannelsCount = 0;

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

void SoundManager::playSound(Sound* sound, u32 command, const Vector3D* position, u32 playbackType, EventListener soundReleaseListener, Object scope)
{
	SoundManager::purgeReleasedSoundWrappers(this);
	SoundManager::tryToPlayQueuedSounds(this);

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

void SoundManager::onSoundWrapperReleased(Object eventFirer)
{
	SoundWrapper releasedSoundWrapper = SoundWrapper::safeCast(eventFirer);

	if(!isDeleted(releasedSoundWrapper))
	{
		SoundManager::releaseSoundWrapper(this, releasedSoundWrapper);
	}
}

/**
 * Request a new sound
 *
 * @param sound		Sound*
 */
SoundWrapper SoundManager::getSound(Sound* sound, u32 command, EventListener soundReleaseListener, Object scope)
{
	if(this->lock)
	{
		return NULL;
	}

	return SoundManager::doGetSound(this, sound, command, soundReleaseListener, scope);
}

SoundWrapper SoundManager::doGetSound(Sound* sound, u32 command, EventListener soundReleaseListener, Object scope)
{
	if(NULL == sound)
	{
		return NULL;
	}

	// Compute the number of
	u8 normalChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNormal);
	u8 modulationChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelModulation);
	u8 noiseChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNoise);
	u8 normalExtendeChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNormalExtended);

	// Check for free channels
	VirtualList availableChannels  = new VirtualList();

	u8 usableNormalChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, normalChannelsCount, kChannelNormal | (normalExtendeChannelsCount && 0 == modulationChannelsCount ? kChannelModulation : kChannelNormal));
	u8 usableModulationChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, modulationChannelsCount, kChannelModulation);
	u8 usableNoiseChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, noiseChannelsCount, kChannelNoise);

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
				s8 waves[__TOTAL_WAVEFORMS] = {-1, -1, -1, -1, -1};

				u16 i = 0;

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

	delete availableChannels;

	if(!isDeleted(soundWrapper))
	{
		this->hasPCMSounds |= soundWrapper->hasPCMTracks;

		SoundWrapper::addEventListener(soundWrapper, Object::safeCast(this), (EventListener)SoundManager::onSoundWrapperReleased, kEventSoundReleased);
	}

	return soundWrapper;
}

/**
 * Release sound wrapper
 */
void SoundManager::releaseSoundWrapper(SoundWrapper soundWrapper)
{
//	NM_ASSERT(!isDeleted(VirtualList::find(this->soundWrappers, soundWrapper)), "SoundManager::releaseSoundWrapper: invalid soundWrapper");
//	NM_ASSERT(NULL == VirtualList::find(this->releasedSoundWrappers, soundWrapper), "SoundManager::releaseSoundWrapper: already released soundWrapper");

	if(isDeleted(soundWrapper))
	{
		return;
	}

	if(!VirtualList::find(this->releasedSoundWrappers, soundWrapper))
	{
		// Release soundWrapper's channels
		VirtualNode auxNode = soundWrapper->channels->head;

		for(; auxNode; auxNode = auxNode->next)
		{
			Channel* channel = (Channel*)auxNode->data;

			SoundManager::releaseSoundChannel(this, channel);
		}

		VirtualList::pushBack(this->releasedSoundWrappers, soundWrapper);
	}
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

	if(release && !this->lockSoundWrappersList)
	{
		SoundManager::purgeReleasedSoundWrappers(this);
	}

	__SSTOP = 0x01;
}

void SoundManager::print()
{
	int x = 1;
	int xDisplacement = 8;
	int yDisplacement = 0;

	int i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		int y = yDisplacement;

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

void SoundManager::printWaveFormStatus(int x, int y)
{
	// Reset all waveforms
	for(u16 i = 0; i < __TOTAL_WAVEFORMS; i++)
	{
		PRINT_TEXT("           ", x, y + this->waveforms[i].number);
		PRINT_INT(this->waveforms[i].number, x, y + this->waveforms[i].number);
		PRINT_INT(this->waveforms[i].usageCount, x + 4, y + this->waveforms[i].number);
		PRINT_HEX((u32)this->waveforms[i].data, x + 8, y + this->waveforms[i].number);
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