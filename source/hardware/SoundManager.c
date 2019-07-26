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


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
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

	int i = 0;
	for(; i < __TOTAL_CHANNELS; i++)
	{
		this->soundWrappers[i] = NULL;
	}

	SoundManager::reset(this);
}

/**
 * Class destructor
 */
void SoundManager::destructor()
{
	Base::destructor();
}

void SoundManager::reset()
{
	int i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		if(!isDeleted(this->soundWrappers[i]))
		{
			delete this->soundWrappers[i];
		}

		this->soundWrappers[i] = new SoundWrapper(i + 1);

		this->waveforms[i].number = i;
		this->waveforms[i].wave = __WAVE_ADDRESS(i + 1);
		this->waveforms[i].data = NULL;
	}

	SoundManager::stopAllSounds(this);
}

void SoundManager::playSounds(u32 type)
{
	u16 i = 0;
	
	for(; i < __TOTAL_CHANNELS; i++)
	{
		if(type == SoundWrapper::getType(this->soundWrappers[i]))
		{
			SoundWrapper::updatePlayback(this->soundWrappers[i]);
		}
	}
}

void SoundManager::playMIDISounds()
{
	SoundManager::playSounds(this, kMIDI);
}

void SoundManager::playPCMSounds()
{
	SoundManager::playSounds(this, kPCM);
}

s8 SoundManager::getWaveform(const u8* waveFormData)
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
			return this->waveforms[i].number;
		}
	}

	if(NULL != freeWaveform)
	{
		freeWaveform->data = waveFormData;
		return freeWaveform->number;
	}

	return -1;
}

void SoundManager::setWaveform(Waveform* waveform, const u8* data)
{
	if(NULL != waveform)
	{
		waveform->data = data;

		int i;

		for(i = 0; i < 32; i++)
		{
			waveform->wave[i * 4] = data[i];
		}
	}
}

static u8 SoundManager::getSoundChannelsCount(Sound* sound)
{
	// Compute the number of 
	u8 soundChannelsCount = 0;

	for(; sound->soundChannels[soundChannelsCount]; soundChannelsCount++);

	return __TOTAL_CHANNELS < soundChannelsCount ? __TOTAL_CHANNELS : soundChannelsCount;
}

u8 SoundManager::getFreeSoundWrappers(Sound* sound, VirtualList availableSoundWrappers , u8 soundChannelsCount)
{
	if(NULL == sound || isDeleted(availableSoundWrappers ))
	{
		return 0;
	}

	u16 i = 0;
	u8 usableSoundWrappersCount = 0;

	for(i = 0; usableSoundWrappersCount < soundChannelsCount && i < __TOTAL_CHANNELS; i++)
	{
		if(SoundWrapper::isFree(this->soundWrappers[i]))
		{
			usableSoundWrappersCount++;
			VirtualList::pushBack(availableSoundWrappers , this->soundWrappers[i]);
		}
	}

	return usableSoundWrappersCount;
}

SoundWrapper SoundManager::playSound(Sound* sound, bool forceAllChannels, const Vector3D* position)
{
	SoundWrapper soundWrapper = SoundManager::getSound(this, sound, forceAllChannels);

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
SoundWrapper SoundManager::getSound(Sound* sound, bool forceAllChannels)
{
	if(NULL == sound)
	{
		return NULL;
	}

	// Compute the number of 
	u8 soundChannelsCount = SoundManager::getSoundChannelsCount(sound);
	
	// Check for free channels
	VirtualList availableSoundWrappers  = new VirtualList();
	u8 usableSoundWrappersCount = SoundManager::getFreeSoundWrappers(this, sound, availableSoundWrappers , soundChannelsCount);

	if(forceAllChannels)
	{
		/* TODO */
	}
	// If there are enough usable channels
	else if(soundChannelsCount <= usableSoundWrappersCount)
	{
		s8 waves[__TOTAL_CHANNELS] = {-1, -1, -1, -1, -1};

		u16 i = 0;

		for(; i < soundChannelsCount; i++)
		{
			waves[i] = SoundManager::getWaveform(this, sound->soundChannels[i]->soundChannelConfiguration->waveFormData);

			if(0 > waves[i])
			{
				return NULL;
			}
		}

		for(i = 0; i < soundChannelsCount; i++)
		{
			SoundManager::setWaveform(this, &this->waveforms[waves[i]], sound->soundChannels[i]->soundChannelConfiguration->waveFormData);
		}

		VirtualNode node = VirtualList::begin(availableSoundWrappers);

		SoundWrapper leaderSoundWrapper = node ? SoundWrapper::safeCast(VirtualNode::getData(node)) : NULL;

		for(i = 0; node; node = VirtualNode::getNext(node), i++)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			SoundWrapper::setup(soundWrapper, sound, leaderSoundWrapper, i, waves[i], soundChannelsCount);
			SoundWrapper::addLeadedSound(leaderSoundWrapper, soundWrapper);
		}

		return leaderSoundWrapper;
	}

	delete availableSoundWrappers ;

	return NULL;
}

/**
 * Stop all sound playback
 */
void SoundManager::stopAllSounds()
{
	int i = 0;

	// Reset all sounds and channels
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		if(!isDeleted(this->soundWrappers[i]))
		{
			SoundWrapper::stop(this->soundWrappers[i]);
		}
	}

	__SSTOP = 0x01;
}

void SoundManager::print()
{
	int i = 0;
	int x = 1;
	int y = 0;

	for(; i < __TOTAL_CHANNELS; i++)
	{
		SoundWrapper::print(this->soundWrappers[i], x, y);

		x += 16;

		if(x > 33)
		{
			x = 1;
			y += 15;
		}
	}
}