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

// some wave forms data
static const unsigned char sawSquareWave[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Saw + Square */
	0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x18, 0x20,
	0x28, 0x30, 0x38, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

static const unsigned char glockenWave[32] =
{
	0x20, 0x35, 0x26, 0x2d, 0x32, 0x19, 0x1d, 0x2a,	/* Glocken */
	0x24, 0x30, 0x3e, 0x2e, 0x25, 0x21, 0x17, 0x18,
	0x20, 0x28, 0x29, 0x1f, 0x1c, 0x12, 0x02, 0x10,
	0x1c, 0x16, 0x23, 0x27, 0x0f, 0x13, 0x1a, 0x0b,
};

static const unsigned char square0Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

static const unsigned char square1Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave (Duty 75%) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

static const unsigned char organWave[32] =
{
	0x20, 0x3f, 0x38, 0x38, 0x27, 0x23, 0x27, 0x11,	/* Organ (2+3)*/
	0x13, 0x26, 0x0c, 0x26, 0x23, 0x22, 0x1e, 0x00,
	0x20, 0x3f, 0x23, 0x1e, 0x1e, 0x1a, 0x35, 0x1a,
	0x2c, 0x2f, 0x1a, 0x1d, 0x1a, 0x08, 0x09, 0x00,
};

static const unsigned char sinAlphaWave[32] =
{
	0x20, 0x29, 0x30, 0x33, 0x33, 0x31, 0x31, 0x35,	/* Sin Wave + alpha */
	0x39, 0x3e, 0x3e, 0x3a, 0x33, 0x2a, 0x23, 0x20,
	0x20, 0x20, 0x1d, 0x16, 0x0e, 0x06, 0x02, 0x02,
	0x06, 0x0b, 0x0f, 0x0f, 0x0e, 0x0d, 0x10, 0x17,
};

static const unsigned char sawWave[32] =
{
	0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f,	/* Saw Wave */
	0x31, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f,
	0x11, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3d, 0x3f,
};

static const unsigned char sinWave[32] =
{
	0x00, 0x06, 0x0C, 0x11, 0x16, 0x1A, 0x1D, 0x1E,
	0x1F, 0x1E, 0x29, 0x1D, 0x16, 0x11, 0x0C, 0x06,
	0x00, 0x39, 0x33, 0x2E, 0x29, 0x25, 0x22, 0x21,
	0x20, 0x21, 0x22, 0x25, 0x29, 0x2E, 0x33, 0x39
};

/**
 * Sound Registry
 *
 */
typedef struct SOUNDREG
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
} SOUNDREG;

// reserved for world's background
static u8* const WAVEDATA1 =		(u8*)0x01000000;
static u8* const WAVEDATA2 =		(u8*)0x01000080;
// reserved for world's background
static u8* const WAVEDATA3 =		(u8*)0x01000100;
static u8* const WAVEDATA4 =		(u8*)0x01000180;
static u8* const WAVEDATA5 =		(u8*)0x01000200;
static u8* const MODDATA =			(u8*)0x01000280;
static SOUNDREG* const SND_REGS =	(SOUNDREG*)0x01000400; //(SOUNDREG*)0x010003C0;
#define SSTOP						*(u8*)0x01000580

#define __MAXIMUM_OUTPUT_LEVEL		0xF


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

	// reset all records
	for(i = 0; i < __TOTAL_SOUNDS; i++)
	{
		this->noteWait[i] = 0;
		this->actualNote[i] = 0;
	}

	this->bgm = NULL;

	for( i = 0; i < __FXS; i++)
	{
		this->fxSound[i] = NULL;

		this->fxLeftOutput[i] = 0;
		this->fxRightOutput[i] = 0;
	}

	SoundManager::setWaveForm(this);
}

/**
 * Load wave form data
 */
void SoundManager::setWaveForm()
{
	int i;
	for(i = 0; i < 32; i++)
	{
		WAVEDATA1[i*4] = organWave[i];
		WAVEDATA2[i*4] = sinWave[i];
		WAVEDATA3[i*4] = glockenWave[i];
		WAVEDATA4[i*4] = square1Wave[i];
		WAVEDATA5[i*4] = sinAlphaWave[i];
	}
}

/**
 * Update sound playback
 */
void SoundManager::playSounds()
{
	SoundManager::continuePlayingBGM(this);
	SoundManager::continuePlayingFxSounds(this);
}

/**
 * Update background music playback
 *
 * @private
 */
void SoundManager::continuePlayingBGM()
{
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
		 * SND_REGS 0 to not stop if not explicitly
		 * done.
		 */

		SND_REGS[channel].SxINT = 0x00;

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
			SND_REGS[channel].SxLRV = this->bgm[0][2];

			// set note's frequency
			SND_REGS[channel].SxFQL = (note & 0xFF);
			SND_REGS[channel].SxFQH = (note >> 8);

			// set note's envelope
			SND_REGS[channel].SxEV0 = this->bgm[0][3];

			// set note's envelope mode
			SND_REGS[channel].SxEV1 = this->bgm[0][4];

			// set waveform source
			SND_REGS[channel].SxRAM = this->bgm[0][5];

			// output note
			SND_REGS[channel].SxINT = 0x80;
		}

		// not sure about this
		if(channel == 4)
		{
			SND_REGS[channel].S5SWP = this->bgm[0][5];
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
	 * defined in the sound's definition */
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
					// stop sound
					this->fxSound[fxS] = NULL;
					this->fxLeftOutput[fxS] = 0;
					this->fxRightOutput[fxS] = 0;
					this->noteWait[fxS + 1] = 0;
					this->actualNote[fxS + 1] = 0;
					SND_REGS[fxS + 2].SxLRV = 0x00;
					SND_REGS[fxS + 2].SxINT = 0x00;

					continue;
				}
			}

			// if note has changed
			if(!this->noteWait[fxS + 1])
			{
				// stop sound on the current channel
				/* There is a bug which makes the sound of
				 * SND_REGS 0 to not stop if not explicitly
				 * done.
				 */
				SND_REGS[fxS + 2].SxINT = 0x00;

				// grab note
				note = this->fxSound[fxS][this->actualNote[fxS + 1] + 6];

				// if note is not off
				if(note != 0)
				{
					// if sound is positioned
					SND_REGS[fxS + 2].SxLRV = SoundManager::calculateSoundPosition(this, fxS);

					// set note's frequency
					SND_REGS[fxS + 2].SxFQL = (note & 0xFF);
					SND_REGS[fxS + 2].SxFQH = (note >> 8);

					// set note's envelope
					SND_REGS[fxS + 2].SxEV0 = this->fxSound[fxS][3];

					// set note's envelope mode
					SND_REGS[fxS + 2].SxEV1 = this->fxSound[fxS][4];

					// set waveform source
					SND_REGS[fxS + 2].SxRAM = this->fxSound[fxS][5];

					// output note
					SND_REGS[fxS + 2].SxINT = 0x80;
				}
			}

			this->noteWait[fxS + 1]++;
		}
		else
		{
			SND_REGS[fxS + 2].SxLRV = 0x00;

			SND_REGS[fxS + 2].SxINT = 0x00;
		}
	}
}

/**
 * Start playback of background music
 *
 * @param bgm	Background music
 */
void SoundManager::playBGM(const u16 (*bgm)[])
{
	SoundManager::stopAllSound(this);
	this->bgm = bgm;
}

/**
 * Start playback of fx sound.
 * If all fx channels are in use, it is not guaranteed that the sound will be played.
 *
 * @param fxSound	Fx sound to play
 * @param position	3D position
 * @return 			True if playback started
 */
int SoundManager::playFxSound(const u16* fxSound, Vector3D position)
{
	int i = 0;

	// try to find a free channel
	for(;i < __FXS && this->fxSound[i]; i++);

	// if a channel was available
	if(i < __FXS)
	{
		// record the fx definition's address
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
void SoundManager::stopAllSound()
{
	int channel = 0;

	//disables sound on all channels
	for(channel = 0; channel < 6; channel++)
	{
		SND_REGS[channel].SxINT = 0x00;
	}

	SSTOP = 0x01;
}
