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

/**
 * Sound Registry
 *
 */
typedef struct SoundRegistry
{
	// this table is for the most part untested, but looks to be accurate
	//				 	|		D7	   ||		D6	   ||		D5	   ||		D4	   ||		D3	   ||		D2	   ||		D1	   ||		D0	   |
	u8 SxINT; //		[----Enable----][--XXXXXXXXXX--][-Interval/??--][--------------------------------Interval Data---------------------------------]
	u8 spacer1[3];
	u8 SxLRV; //		[---------------------------L Level----------------------------][---------------------------R Level----------------------------]
	u8 spacer2[3];
	u8 SxFQL; //		[------------------------------------------------------Frequency Low Byte------------------------------------------------------]
	u8 spacer3[3];
	u8 SxFQH; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Frequency High Byte-------------]
	u8 spacer4[3];
	u8 SxEV0; //		[---------------------Initial Envelope Value-------------------][------U/D-----][-----------------Envelope Step----------------]
	u8 spacer5[3];
			 //Ch. 1-4 	[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
			 //Ch. 5	[--XXXXXXXXXX--][------E/D-----][----?/Short---][--Mod./Sweep--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 SxEV1; //Ch. 6	[--XXXXXXXXXX--][----------------------E/D---------------------][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 spacer6[3];
	//Ch. 1-5 only (I believe address is only 3 bits, but may be 4, needs testing)
	u8 SxRAM; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Waveform RAM Address------------]
	u8 spacer7[3];
	//Ch. 5 only
	u8 S5SWP; //		[------CLK-----][-------------Sweep/Modulation Time------------][------U/D-----][----------------Number of Shifts--------------]
	u8 spacer8[35];
} SoundRegistry;

static SoundRegistry* const _soundRegistries =	(SoundRegistry*)0x01000400; //(SoundRegistry*)0x010003C0;

#define __WAVE_ADDRESS(n)			(u8*)(0x01000000 + (n - 1) * 128)
#define __MODDATA					(u8*)0x01000280;
#define __SSTOP						*(u8*)0x01000580

#define __MAXIMUM_OUTPUT_LEVEL		0xF
#define __MAXIMUM_VOLUMEN			0xF


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
		this->channels[i].number = i + 1;
		this->channels[i].sound = NULL;
		this->channels[i].cursor = 0;
		this->channels[i].delay = 0;
		this->channels[i].soundChannel = 0;
		this->channels[i].partners = 0;
		this->channels[i].leaderChannel = NULL;

		this->channels[i].soundChannelConfiguration.type = kUnknownType;
		this->channels[i].soundChannelConfiguration.SxLRV = 0;
		this->channels[i].soundChannelConfiguration.SxRAM = 0;
		this->channels[i].soundChannelConfiguration.SxEV0 = 0;
		this->channels[i].soundChannelConfiguration.SxEV1 = 0;
		this->channels[i].soundChannelConfiguration.SxFQH = 0;
		this->channels[i].soundChannelConfiguration.SxFQL = 0;
		this->channels[i].soundChannelConfiguration.waveFormData = NULL;
		this->channels[i].soundChannelConfiguration.isModulation = false;

		this->waveforms[i].number = i;
		this->waveforms[i].wave = __WAVE_ADDRESS(i + 1);
		this->waveforms[i].data = NULL;
	}

	SoundManager::stopAllSounds(this);
}

void SoundManager::updateMIDIPlayback(Channel* channel)
{
	//_soundRegistries[channel->number].SxLRV = SoundManager::calculateSoundPosition(this, fxS)
	u16 note = channel->sound->soundChannels[channel->soundChannel]->soundTrack[channel->cursor];
	_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
	_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH = (note >> 8);
}

void SoundManager::updatePCMPlayback(Channel* channel)
{
	//_soundRegistries[channel->number].SxLRV = SoundManager::calculateSoundPosition(this, fxS)
	u8 volume = channel->sound->soundChannels[channel->soundChannel]->soundTrack[channel->cursor];

	u16 maximumAccumulatedVolume = __MAXIMUM_VOLUMEN * (channel->partners + 1);

	// Clamp volume to avoid saturation
	volume = volume > maximumAccumulatedVolume? maximumAccumulatedVolume : volume;

	s8 finalVolume  = 0;
	finalVolume  = (s8)volume - __MAXIMUM_VOLUMEN * (channel->soundChannel);

	if(finalVolume  < 0)
	{
		finalVolume  = 0;
	}
	else if (finalVolume  > __MAXIMUM_VOLUMEN)
	{
		finalVolume  = __MAXIMUM_VOLUMEN;
	}			

	u8 leftVolume = ((u8)finalVolume  << 4) & 0xF0;
	u8 rightVolume = ((u8)finalVolume ) & 0x0F;

	_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = leftVolume | rightVolume;

//	_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1 = 0x40;
}

/**
 * Update sound playback
 */
bool SoundManager::updatePlayback(Channel* channel, u32 type)
{
	if(NULL == channel->sound)
	{
		return false;
	}

	switch(type)
	{
		case kMIDI:

			SoundManager::updateMIDIPlayback(this, channel);
			break;

		case kPCM:

			SoundManager::updatePCMPlayback(this, channel);
			break;
	}

	if(channel->partners && channel->leaderChannel != channel)
	{
		channel->cursor = channel->leaderChannel->cursor;
	}
	else if(++channel->delay > channel->sound->soundChannels[channel->soundChannel]->delay)
	{
		channel->delay = 0;

		if(++channel->cursor >= channel->sound->soundChannels[channel->soundChannel]->length)
		{
			channel->cursor = 0;

			if(!channel->sound->loop)
			{
				return false;
			}
		}
	}

	
	//channel->cursor = this->channels[0].cursor + channel->number;

	return true;
}

void SoundManager::playSounds(u32 type)
{
	u16 i = 0;
	
	for(i = 0; i < __TOTAL_CHANNELS; i++)
	{
		if(NULL != this->channels[i].sound && type == this->channels[i].soundChannelConfiguration.type)
		{
			if(!SoundManager::updatePlayback(this, &this->channels[i], type))
			{
				this->channels[i].sound = NULL;
			}
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

/**
 * Update background music playback
 *
 * @private
 */
void SoundManager::continuePlayingBGM()
{
	return;
	// only if bgm loaded
	if(!this->bgm)
	{
		return;
	}

	int channel = 0;

	u32 note = 0;

	int i;

	// check if note's length has been played
	if(this->noteWait[0] > this->bgm[0][1])
	{
		// move to the next note
		this->actualNote[0]++;

		// initialize this->noteWait[0]
		this->noteWait[0] = 0;

		// if note is greater than song's length
		if(this->actualNote[0] >= this->bgm[0][0])
		{
			// rewind song
			this->actualNote[0] = 0;
		}
	}

	// if note has changed
	if(this->noteWait[0])
	{
		this->noteWait[0]++;
		return;
	}

	for(channel = 0, i = 0; channel < 2; channel++)
	{
		// stop sound on the current channel
		/* There is a bug which makes the sound of
		 * _soundRegistries 0 to not stop if not explicitly
		 * done.
		 */

		_soundRegistries[channel].SxINT = 0x00;

		// grab note
		for(; i < 2 && !this->bgm[this->actualNote[0] + 3][i]; i++);

		if(i < 2)
		{
			note = this->bgm[this->actualNote[0] + 3][i];
		}

		// if note is not off
		if(note != 0)
		{
			// set note's output level
			_soundRegistries[channel].SxLRV = this->bgm[0][2];

			// set note's frequency
			_soundRegistries[channel].SxFQL = (note & 0xFF);
			_soundRegistries[channel].SxFQH = (note >> 8);

			// set note's envelope
			_soundRegistries[channel].SxEV0 = this->bgm[0][3];

			// set note's envelope mode
			_soundRegistries[channel].SxEV1 = this->bgm[0][4];

			// set waveform source
			_soundRegistries[channel].SxRAM = this->bgm[0][5];

			// output note
			_soundRegistries[channel].SxINT = 0x80;
		}

		// not sure about this
		if(channel == 4)
		{
			_soundRegistries[channel].S5SWP = this->bgm[0][5];
		}
	}

	this->noteWait[0]++;
}

/**
 * Calculate sound volume according to its spatial position
 *
 * @private
 * @param fxS	Fx sound index
 */
int SoundManager::calculateSoundPosition(int fxS)
{
	int output = 0x00;

	/* The maximum sound level for each side is 0xF
	 * In the center position the output level is the one
	 * defined in the sound's spec */
	if(0 < this->fxLeftOutput[fxS])
	{
		output |= (((int)this->fxLeftOutput[fxS]) << 4);
	}

	if(0 < this->fxRightOutput[fxS])
	{
		output|=(((int)this->fxRightOutput[fxS]));
	}
	else
	{
		output &= 0xF0;
	}

	return output;
}

/**
 * Update fx sounds playback
 *
 * @private
 */
void SoundManager::continuePlayingFxSounds()
{
	return;
	int note = 0;
	int fxS = 0;

	for(; fxS < __FXS; fxS++)
	{
		// only if fx defined
		if(this->fxSound[fxS])
		{
			// check if note's length has been played
			if(this->noteWait[fxS + 1] > this->fxSound[fxS][1])
			{
				// move to the next note
				this->actualNote[fxS + 1]++;

				// initialize this->noteWait[0]
				this->noteWait[fxS + 1] = 0;

				// if note if greater than song's length
				if(this->actualNote[fxS + 1] > this->fxSound[fxS][0])
				{
					if(true)
					{
						// rewind fx
						this->actualNote[fxS + 1] = 0;
						continue;
					}
					// stop sound
					this->fxSound[fxS] = NULL;
					this->fxLeftOutput[fxS] = 0;
					this->fxRightOutput[fxS] = 0;
					this->noteWait[fxS + 1] = 0;
					this->actualNote[fxS + 1] = 0;
					_soundRegistries[fxS + 2].SxLRV = 0x00;
					_soundRegistries[fxS + 2].SxINT = 0x00;

					continue;
				}
			}

			// if note has changed
			if(!this->noteWait[fxS + 1])
			{
				// stop sound on the current channel
				/* There is a bug which makes the sound of
				 * _soundRegistries 0 to not stop if not explicitly
				 * done.
				 */
				_soundRegistries[fxS + 2].SxINT = 0x00;

				// grab note
				note = this->fxSound[fxS][this->actualNote[fxS + 1] + 6];

				// if note is not off
				if(note != 0)
				{
					// if sound is positioned
					_soundRegistries[fxS + 2].SxLRV = SoundManager::calculateSoundPosition(this, fxS);

					// set note's frequency
					_soundRegistries[fxS + 2].SxFQL = (note & 0xFF);
					_soundRegistries[fxS + 2].SxFQH = (note >> 8);

					// set note's envelope
					_soundRegistries[fxS + 2].SxEV0 = this->fxSound[fxS][3];

					// set note's envelope mode
					_soundRegistries[fxS + 2].SxEV1 = this->fxSound[fxS][4];

					// set waveform source
					_soundRegistries[fxS + 2].SxRAM = this->fxSound[fxS][5];

					// output note
					_soundRegistries[fxS + 2].SxINT = 0x80;
				}
			}

			this->noteWait[fxS + 1]++;
		}
		else
		{
			_soundRegistries[fxS + 2].SxLRV = 0x00;

			_soundRegistries[fxS + 2].SxINT = 0x00;
		}
	}
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

u8 SoundManager::getChannelsForSound(Sound* sound, VirtualList availableChannels, u8 soundChannelsCount)
{
	if(NULL == sound || isDeleted(availableChannels))
	{
		return 0;
	}

	u16 i = 0;
	u8 usableChannels = 0;

	for(i = 0; usableChannels < soundChannelsCount && i < __TOTAL_CHANNELS; i++)
	{
		if(NULL == this->channels[i].sound)
		{
			usableChannels++;
			VirtualList::pushBack(availableChannels, &this->channels[i]);
		}
	}

	return usableChannels;
}

void SoundManager::configureChannel(Channel* channel)
{
	if(NULL == channel)
	{
		return;
	}

	_soundRegistries[channel->number].SxEV0 = channel->soundChannelConfiguration.SxEV0;
	_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
	_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH;
	_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL;
	_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV;
	_soundRegistries[channel->number].SxRAM = channel->soundChannelConfiguration.SxRAM;
	_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;	
}

/**
 * Start playback of a sound
 *
 * @param sound		Sound*
 */
u32 SoundManager::play(Sound* sound, bool forceAllChannels)
{
	u32 returnMessage = kNullSound;

	if(NULL == sound)
	{
		return returnMessage;
	}

	// Compute the number of 
	u8 soundChannelsCount = SoundManager::getSoundChannelsCount(sound);
	
	// Check for free channels
	VirtualList availableChannels = new VirtualList();
	u8 usableChannels = SoundManager::getChannelsForSound(this, sound, availableChannels, soundChannelsCount);

	if(forceAllChannels)
	{
		returnMessage = kNotEnoughFreeChannels;
	}
	// If there are enough usable channels
	else if(soundChannelsCount <= usableChannels)
	{
		s8 waves[__TOTAL_CHANNELS] = {-1, -1, -1, -1, -1};

		u16 i = 0;

		for(; i < soundChannelsCount; i++)
		{
			waves[i] = SoundManager::getWaveform(this, sound->soundChannels[i]->soundChannelConfiguration->waveFormData);

			if(0 > waves[i])
			{
				return kNotEnoughFreeWaveforms;
			}
		}

		for(i = 0; i < soundChannelsCount; i++)
		{
			SoundManager::setWaveform(this, &this->waveforms[waves[i]], sound->soundChannels[i]->soundChannelConfiguration->waveFormData);
		}

		VirtualNode node = VirtualList::begin(availableChannels);

		Channel* leaderChannel = node ? (Channel*)VirtualNode::getData(node) : NULL;

		for(i = 0; node; node = VirtualNode::getNext(node), i++)
		{
			Channel* channel = (Channel*)VirtualNode::getData(node);

			channel->sound = sound;
			channel->cursor = 0;
			channel->delay = 0;
			channel->soundChannel = i;
			channel->soundChannelConfiguration = *sound->soundChannels[i]->soundChannelConfiguration;
			channel->soundChannelConfiguration.SxRAM = waves[i];
			channel->partners = sound->combineChannels ? soundChannelsCount - 1: 0;
			channel->leaderChannel = leaderChannel;


			SoundManager::configureChannel(this, channel);
		}

		returnMessage = kPlayRequestSuccess;
	}
	else
	{
		returnMessage = kNotEnoughFreeChannels;
	}

	delete availableChannels;

	return returnMessage;
}

/**
 * Start playback of background music
 *
 * @param bgm	Background music
 */
void SoundManager::playBGM(const u16 (*bgm)[])
{
	SoundManager::stopAllSounds(this);
	this->bgm = bgm;
}

int SoundManager::playFxSound(const u16* fxSound, Vector3D position)
{
//	init_speech();
	int i = 0;

	// try to find a free channel
	for(;i < __FXS && this->fxSound[i]; i++);

	// if a channel was available
	if(i < __FXS)
	{
		// record the fx spec's address
		this->fxSound[i] = fxSound;

		// set position inside camera coordinates
		position = Vector3D::getRelativeToCamera(position);

		fix10_6 maxOutputLevel = __I_TO_FIX10_6(__MAXIMUM_OUTPUT_LEVEL);
		fix10_6 leftDistance = __ABS(__FIX10_6_MULT(position.x - __PIXELS_TO_METERS(__LEFT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));
		fix10_6 rightDistance = __ABS(__FIX10_6_MULT(position.x - __PIXELS_TO_METERS(__RIGHT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));

		fix10_6 leftOutput = maxOutputLevel - __FIX10_6_MULT(maxOutputLevel, __FIX10_6_DIV(leftDistance, _optical->horizontalViewPointCenter));
		this->fxLeftOutput[i] = __FIX10_6_TO_I(leftOutput - __FIX10_6_MULT(leftOutput, position.z >> _optical->maximumXViewDistancePower));

		fix10_6 rightOutput = maxOutputLevel - __FIX10_6_MULT(maxOutputLevel, __FIX10_6_DIV(rightDistance, _optical->horizontalViewPointCenter));
		this->fxRightOutput[i] = __FIX10_6_TO_I(rightOutput - __FIX10_6_MULT(rightOutput, position.z >> _optical->maximumXViewDistancePower));

		return true;
	}

	// no channel available
	return false;
}

// returns true if the sound is being played
int SoundManager::playingSound(const u16* fxSound)
{
	int i = 0;

	// find sound
	for(;i < __FXS && this->fxSound[i] != fxSound; i++);

	// if sound found
	if(i < __FXS)
	{
		return true;
	}

	return false;
}

/**
 * Stop all sound playback
 */
void SoundManager::stopAllSounds()
{
	int channel = 0;

	//disables sound on all channels
	for(channel = 0; channel < __TOTAL_CHANNELS + 1; channel++)
	{
		_soundRegistries[channel].SxINT = 0x00;
	}

	__SSTOP = 0x01;
}


static void SoundManager::printSound(Sound* sound, u8 soundChannel, u32 cursor, int x, int y)
{
	if(NULL == sound)
	{
		return;
	}

	int xDisplacement = 9;

//	PRINT_TEXT("Sound: ", x, ++y);
//	PRINT_HEX((u32)sound, x + xDisplacement, y);

	PRINT_TEXT("Loop: ", x, ++y);
	PRINT_TEXT(sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

	PRINT_TEXT("Length: ", x, ++y);
	PRINT_INT(sound->soundChannels[soundChannel]->length, x + xDisplacement, y);
	
	PRINT_TEXT("Note: ", x, ++y);
	u16 note = sound->soundChannels[soundChannel]->soundTrack[cursor];

	PRINT_HEX_EXT(sound->soundChannels[soundChannel]->soundTrack[cursor], x + xDisplacement, y, 2);
}

static void SoundManager::printChannel(Channel* channel, int x, int y)
{
	if(NULL == channel)
	{
		return;
	}

	int xDisplacement = 9;

	PRINT_TEXT("CHANNEL: ", x, y);
	PRINT_INT(channel->number, x + xDisplacement, y);

	PRINT_TEXT("Type: ", x, ++y);

	char* soundType = "?";
	switch(channel->soundChannelConfiguration.type)
	{
		case kMIDI:

			soundType = "MIDI";
			break;

		case kPCM:

			soundType = "PCM";
			break;
	}

	PRINT_TEXT(soundType, x + xDisplacement, y);

	PRINT_TEXT("Cursor: ", x, ++y);
	PRINT_INT(channel->cursor, x + xDisplacement, y);
return;
	PRINT_TEXT("Snd Chnl: ", x, ++y);
	PRINT_INT(channel->soundChannel, x + xDisplacement, y);

	PRINT_TEXT("SxINT: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxINT | (NULL == channel->sound ? 0 : 0x80), x + xDisplacement, y, 2);

	PRINT_TEXT("SxLRV: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxLRV, x + xDisplacement, y, 2);

	PRINT_TEXT("SxRAM: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxRAM, x + xDisplacement, y, 2);

	PRINT_TEXT("SxEV0: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);

	PRINT_TEXT("SxEV1: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxEV1, x + xDisplacement, y, 2);

	PRINT_TEXT("SxFQH: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);

	PRINT_TEXT("SxFQH: ", x, ++y);
	PRINT_HEX_EXT(channel->soundChannelConfiguration.SxFQL, x + xDisplacement, y, 2);
 
	if(NULL == channel->sound)
	{
		PRINT_TEXT("Sound:", x, ++y);
		PRINT_TEXT("None", x + xDisplacement, y);
	}
	else
	{
		SoundManager::printSound(channel->sound, channel->soundChannel, channel->cursor, x, y);
	}
}

void SoundManager::print()
{
	int i = 0;
	int x = 1;
	int y = 1;

	for(; i < __TOTAL_CHANNELS; i++)
	{
		SoundManager::printChannel(&this->channels[i], x, y);

		x += 16;

		if(x > 33)
		{
			x = 1;
			y += 15;
		}
	}
}