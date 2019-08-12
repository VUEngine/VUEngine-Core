/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#undef __SOUND_MANAGER_PROFILE


//---------------------------------------------------------------------------------------------------------
//											 CLASS' DEFINITIONS
//---------------------------------------------------------------------------------------------------------

const unsigned char sawSquareWave[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Saw + Square */
	0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x18, 0x20,
	0x28, 0x30, 0x38, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const unsigned char glockenWave[32] =
{
	0x20, 0x35, 0x26, 0x2d, 0x32, 0x19, 0x1d, 0x2a,	/* Glocken */
	0x24, 0x30, 0x3e, 0x2e, 0x25, 0x21, 0x17, 0x18,
	0x20, 0x28, 0x29, 0x1f, 0x1c, 0x12, 0x02, 0x10,
	0x1c, 0x16, 0x23, 0x27, 0x0f, 0x13, 0x1a, 0x0b,
};

const unsigned char square0Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const unsigned char square1Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave (Duty 75%) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const unsigned char organWave[32] =
{
	0x20, 0x3f, 0x38, 0x38, 0x27, 0x23, 0x27, 0x11,	/* Organ (2+3)*/
	0x13, 0x26, 0x0c, 0x26, 0x23, 0x22, 0x1e, 0x00,
	0x20, 0x3f, 0x23, 0x1e, 0x1e, 0x1a, 0x35, 0x1a,
	0x2c, 0x2f, 0x1a, 0x1d, 0x1a, 0x08, 0x09, 0x00,
};

const unsigned char sinAlphaWave[32] =
{
	0x20, 0x29, 0x30, 0x33, 0x33, 0x31, 0x31, 0x35,	/* Sin Wave + alpha */
	0x39, 0x3e, 0x3e, 0x3a, 0x33, 0x2a, 0x23, 0x20,
	0x20, 0x20, 0x1d, 0x16, 0x0e, 0x06, 0x02, 0x02,
	0x06, 0x0b, 0x0f, 0x0f, 0x0e, 0x0d, 0x10, 0x17,
};

const unsigned char sawWave[32] =
{
	0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f,	/* Saw Wave */
	0x31, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f,
	0x11, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3d, 0x3f,
};

const unsigned char sinWave[32] =
{
	0x00, 0x06, 0x0C, 0x11, 0x16, 0x1A, 0x1D, 0x1E,
	0x1F, 0x1E, 0x29, 0x1D, 0x16, 0x11, 0x0C, 0x06,
	0x00, 0x39, 0x33, 0x2E, 0x29, 0x25, 0x22, 0x21,
	0x20, 0x21, 0x22, 0x25, 0x29, 0x2E, 0x33, 0x39
};

const unsigned char linearWave[32] =
{
	0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
	0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
	0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
	0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
};

SoundRegistry* const _soundRegistries =	(SoundRegistry*)0x01000400; //(SoundRegistry*)0x010003C0;

#define __WAVE_ADDRESS(n)			(u8*)(0x01000000 + (n - 1) * 128)
#define __MODDATA					(u8*)0x01000280;
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

			VirtualNode auxNode = soundWrapper->channels->head;

			for(; auxNode; auxNode = auxNode->next)
			{
				Channel* channel = (Channel*)auxNode->data;

				SoundManager::releaseSoundChannel(this, channel);
			}

			VirtualList::removeElement(this->soundWrappers, soundWrapper);

			delete soundWrapper;
		}

		VirtualList::clear(this->releasedSoundWrappers);
	}
}

void SoundManager::reset()
{
	SoundManager::purgeReleasedSoundWrappers(this);

	if(!isDeleted(this->soundWrappers))
	{
		VirtualNode node = this->soundWrappers->head;

		for(; node; node = node->next)
		{
			delete node->data;
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

	// Reset all sounds and channels
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

		this->waveforms[i].number = i;
		this->waveforms[i].usageCount = 0;
		this->waveforms[i].wave = __WAVE_ADDRESS(i + 1);
		this->waveforms[i].data = NULL;
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

	SoundManager::stopAllSounds(this);
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

bool SoundManager::playMIDISounds()
{
	u32 elapsedMicroseconds = TimerManager::getTimePerInterruptInUS(TimerManager::getInstance());

	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(soundWrapper->hasMIDITracks)
		{
			SoundWrapper::updateMIDIPlayback(soundWrapper, elapsedMicroseconds);
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

	return true;
}

void SoundManager::updateFrameRate()
{
	if(!this->hasPCMSounds)
	{
		return;
	}

	s16 deviation = (this->pcmPlaybackCycles - this->pcmTargetPlaybackFrameRate/ (__MILLISECONDS_PER_SECOND / __GAME_FRAME_DURATION));

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
	int i = 0;

	Waveform* freeWaveform = NULL;

	// Reset all sounds and channels
	for(i = __TOTAL_CHANNELS - 1; 0 <= i; i--)
	{
		if(NULL == this->waveforms[i].data)
		{
			freeWaveform = &this->waveforms[i];
		}

		if(waveFormData == this->waveforms[i].data)
		{
			this->waveforms[i].usageCount++;
			return this->waveforms[i].number;
		}
	}

	if(NULL != freeWaveform)
	{
		freeWaveform->data = waveFormData;
		freeWaveform->usageCount += 1;

		return freeWaveform->number;
	}

	return -1;
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
				this->waveforms[waveFormIndex].data = NULL;
			}
		}
		else
		{
			NM_ASSERT(false, "SoundManager::releaseWaveform: mismatch between index and data");
		}	
	}
}

void SoundManager::releaseSoundChannel(Channel* channel)
{
	if(channel)
	{
		SoundManager::releaseWaveform(this, channel->soundChannelConfiguration.SxRAM, channel->sound->soundChannels[channel->soundChannel]->soundChannelConfiguration->waveFormData);
		channel->sound = NULL;
	}
}

void copymem (u8* dest, const u8* src, u16 num)
{
	u16 i;
	for (i = 0; i < num; i++) *dest++ = *src++;
}
void SoundManager::setWaveform(Waveform* waveform, const s8* data, u32 type)
{
	if(NULL != waveform)
	{
		waveform->data = (s8*)data;

		int i;
		int displacement = 0;

		switch (type)
		{
			case kMIDI:

				displacement = 0;
				break;

			case kPCM:

				displacement = 2;
				break;

			default:
				break;
		}

		for(i = 0; i < 32; i++)
		{
			waveform->wave[i << displacement] = (u8)data[i];
		}

		/*
		// TODO
		const u8 kModData[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, -1, -2, -3, -4, -5,
		-6, -7, -8, -9, -16, -17, -18, -19, -20, -21, -22
		};

		u8* moddata = __MODDATA;
		for(i = 0; i <= 0x7C; i++)
		{
			moddata[i << 2] = kModData[i];
		}
		*/
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
		if(NULL == this->channels[i].sound && this->channels[i].type & channelType)
		{
			usableChannelsCount++;
			VirtualList::pushBack(availableChannels , &this->channels[i]);
		}
	}

	return usableChannelsCount;
}

SoundWrapper SoundManager::playSound(Sound* sound, bool forceAllChannels, const Vector3D* position)
{
	SoundWrapper soundWrapper = SoundManager::getSound(this, sound, forceAllChannels);

	NM_ASSERT(!isDeleted(soundWrapper), "SoundManager::playSound: could not get any sound");

	if(!isDeleted(soundWrapper))
	{
		SoundWrapper::play(soundWrapper, position);
	}

	return soundWrapper;
}

/**
 * Request a new sound
 *
 * @param sound		Sound*
 */
SoundWrapper SoundManager::getSound(Sound* sound, bool forceAllChannels __attribute__((unused)))
{
	SoundManager::purgeReleasedSoundWrappers(this);

	if(NULL == sound)
	{
		return NULL;
	}

	// Compute the number of 
	u8 normalChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNormal);
	u8 modulationChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelModulation);
	u8 noiseChannelsCount = SoundManager::getSoundChannelsCount(sound, kChannelNoise);
	
	// Check for free channels
	VirtualList availableChannels  = new VirtualList();

	u8 usableNormalChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, normalChannelsCount, kChannelNormal | (0 == modulationChannelsCount ? kChannelModulation : kChannelNormal));
	u8 usableModulationChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, modulationChannelsCount, kChannelModulation);
	u8 usableNoiseChannelsCount = SoundManager::getFreeChannels(this, sound, availableChannels, noiseChannelsCount, kChannelNoise);

	NM_ASSERT(0 == normalChannelsCount || normalChannelsCount <= usableNormalChannelsCount, "SoundManager::getSound: not enough normal channels");
	NM_ASSERT(0 == modulationChannelsCount || 0 < usableModulationChannelsCount, "SoundManager::getSound: not enough modulation channels");
	NM_ASSERT(0 == noiseChannelsCount || 0 < usableNoiseChannelsCount, "SoundManager::getSound: not enough noise channels");

	SoundWrapper soundWrapper = NULL;

		/* TODO 
	if(forceAllChannels)
	{
	}
	// If there are enough usable channels
	else */
	if(normalChannelsCount <= usableNormalChannelsCount && modulationChannelsCount <= usableModulationChannelsCount && noiseChannelsCount <= usableNoiseChannelsCount)
	{
		s8 waves[__TOTAL_CHANNELS] = {-1, -1, -1, -1, -1};

		u16 i = 0;

		for(; i < normalChannelsCount; i++)
		{
			waves[i] = SoundManager::getWaveform(this, sound->soundChannels[i]->soundChannelConfiguration->waveFormData);

			if(0 > waves[i])
			{
				return NULL;
			}
		}

		for(i = 0; i < normalChannelsCount; i++)
		{
			SoundManager::setWaveform(this, &this->waveforms[waves[i]], sound->soundChannels[i]->soundChannelConfiguration->waveFormData, sound->soundChannels[i]->soundChannelConfiguration->trackType);
		}

		NM_ASSERT(0 < VirtualList::getSize(availableChannels), "SoundManager::getSound: 0 availableNormalChannels");

		if(0 < VirtualList::getSize(availableChannels))
		{
			soundWrapper = new SoundWrapper(sound, availableChannels, waves, this->pcmTargetPlaybackFrameRate);

			VirtualList::pushBack(this->soundWrappers, soundWrapper);
		}
	}

	delete availableChannels;

	if(!isDeleted(soundWrapper))
	{
		this->hasPCMSounds |= soundWrapper->hasPCMTracks;
	}

	return soundWrapper;
}

/**
 * Release sound wrapper
 */
void SoundManager::releaseSoundWrapper(SoundWrapper soundWrapper)
{
	NM_ASSERT(!isDeleted(VirtualList::find(this->soundWrappers, soundWrapper)), "SoundManager::releaseSoundWrapper: invalid soundWrapper");
	NM_ASSERT(NULL == VirtualList::find(this->releasedSoundWrappers, soundWrapper), "SoundManager::releaseSoundWrapper: already released soundWrapper");

	if(isDeleted(soundWrapper))
	{
		return;
	}

	if(!VirtualList::find(this->releasedSoundWrappers, soundWrapper))
	{
		VirtualList::pushBack(this->releasedSoundWrappers, soundWrapper);
	}
}

/**
 * Stop all sound playback
 */
void SoundManager::stopAllSounds()
{
	VirtualNode node = this->soundWrappers->head;

	for(; node; node = node->next)
	{
		SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

		if(!isDeleted(soundWrapper))
		{
			SoundWrapper::stop(soundWrapper);
		}
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

		PRINT_TEXT("Snd Ch : ", x, ++y);
		PRINT_INT(this->channels[i].soundChannel, x + xDisplacement, y);

		PRINT_TEXT("SxRAM  : ", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxRAM, x + xDisplacement, y, 2);
		PRINT_TEXT("INT/LVR:  /", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxINT | (NULL == this->channels[i].sound ? 0 : 0x80), x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxLRV, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("EV0/EV1:  /", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxEV1, x + xDisplacement + 3, y, 2);

		PRINT_TEXT("FQH/FQL:  /", x, ++y);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);
		PRINT_HEX_EXT(this->channels[i].soundChannelConfiguration.SxFQL, x + xDisplacement + 3, y, 2);
	
		PRINT_TEXT("Loop   : ", x, ++y);
		PRINT_TEXT(this->channels[i].sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

		PRINT_TEXT("Length : ", x, ++y);
		PRINT_INT(this->channels[i].length, x + xDisplacement, y);
		
		PRINT_TEXT("Note   : ", x, ++y);
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