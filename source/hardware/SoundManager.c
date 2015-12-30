/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SoundManager.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

// some wave forms data
const static unsigned char sawSquareWave[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Saw + Square */
	0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x18, 0x20,
	0x28, 0x30, 0x38, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const static unsigned char glockenWave[32] =
{
	0x20, 0x35, 0x26, 0x2d, 0x32, 0x19, 0x1d, 0x2a,	/* Glocken */
	0x24, 0x30, 0x3e, 0x2e, 0x25, 0x21, 0x17, 0x18,
	0x20, 0x28, 0x29, 0x1f, 0x1c, 0x12, 0x02, 0x10,
	0x1c, 0x16, 0x23, 0x27, 0x0f, 0x13, 0x1a, 0x0b,
};

const static unsigned char square0Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const static unsigned char square1Wave[32] =
{
	0x26, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* Square Wave (Duty 75%) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1b, 0x2a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
};

const static unsigned char organWave[32] =
{
	0x20, 0x3f, 0x38, 0x38, 0x27, 0x23, 0x27, 0x11,	/* Organ (2+3)*/
	0x13, 0x26, 0x0c, 0x26, 0x23, 0x22, 0x1e, 0x00,
	0x20, 0x3f, 0x23, 0x1e, 0x1e, 0x1a, 0x35, 0x1a,
	0x2c, 0x2f, 0x1a, 0x1d, 0x1a, 0x08, 0x09, 0x00,
};

const static unsigned char sinAlphaWave[32] =
{
	0x20, 0x29, 0x30, 0x33, 0x33, 0x31, 0x31, 0x35,	/* Sin Wave + alpha */
	0x39, 0x3e, 0x3e, 0x3a, 0x33, 0x2a, 0x23, 0x20,
	0x20, 0x20, 0x1d, 0x16, 0x0e, 0x06, 0x02, 0x02,
	0x06, 0x0b, 0x0f, 0x0f, 0x0e, 0x0d, 0x10, 0x17,
};

const static unsigned char sawWave[32] =
{
	0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f,	/* Saw Wave */
	0x31, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f,
	0x11, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3d, 0x3f,
};

const static unsigned char sinWave[32] =
{
	0x00, 0x06, 0x0C, 0x11, 0x16, 0x1A, 0x1D, 0x1E,
	0x1F, 0x1E, 0x29, 0x1D, 0x16, 0x11, 0x0C, 0x06,
	0x00, 0x39, 0x33, 0x2E, 0x29, 0x25, 0x22, 0x21,
	0x20, 0x21, 0x22, 0x25, 0x29, 0x2E, 0x33, 0x39
};

typedef struct SOUNDREG
{
	//this table is for the most part untested, but looks to be accurate
	//                 |      D7      ||      D6      ||      D5      ||      D4      ||      D3      ||      D2      ||      D1      ||      D0      |
	u8 SxINT; //       [----Enable----][--XXXXXXXXXX--][-Interval/??--][--------------------------------Interval Data---------------------------------]
	u8 spacer1[3];
	u8 SxLRV; //       [---------------------------L Level----------------------------][---------------------------R Level----------------------------]
	u8 spacer2[3];
	u8 SxFQL; //       [------------------------------------------------------Frequency Low Byte------------------------------------------------------]
	u8 spacer3[3];
	u8 SxFQH; //       [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Frequency High Byte-------------]
	u8 spacer4[3];
	u8 SxEV0; //       [---------------------Initial Envelope Value-------------------][------U/D-----][-----------------Envelope Step----------------]
	u8 spacer5[3];
		 	 //Ch. 1-4 [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	         //Ch. 5   [--XXXXXXXXXX--][------E/D-----][----?/Short---][--Mod./Sweep--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 SxEV1; //Ch. 6  [--XXXXXXXXXX--][----------------------E/D---------------------][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	u8 spacer6[3];
	//Ch. 1-5 only (I believe address is only 3 bits, but may be 4, needs testing)
	u8 SxRAM; //       [--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Waveform RAM Address------------]
	u8 spacer7[3];
	//Ch. 5 only
	u8 S5SWP; //       [------CLK-----][-------------Sweep/Modulation Time------------][------U/D-----][----------------Number of Shifts--------------]
	u8 spacer8[35];
} SOUNDREG;

//reserved for world's background
static u8* const WAVEDATA1 =		(u8*)0x01000000;
static u8* const WAVEDATA2 =		(u8*)0x01000080;
//reserved for world's background
static u8* const WAVEDATA3 =		(u8*)0x01000100;
static u8* const WAVEDATA4 =		(u8*)0x01000180;
static u8* const WAVEDATA5 =		(u8*)0x01000200;
static u8* const MODDATA =			(u8*)0x01000280;
static SOUNDREG* const SND_REGS =	(SOUNDREG*)0x01000400; //(SOUNDREG*)0x010003C0;
#define SSTOP					   *(u8*)0x01000580

#define __MAXIMUM_OUTPUT_LEVEL		0xF


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define SoundManager_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* actual note of each sound being played*/															\
	int actualNote[__TOTAL_SOUNDS];																		\
																										\
	/* note delay for each sound being played */														\
	BYTE noteWait[__TOTAL_SOUNDS];																		\
																										\
	/* background music */																				\
	const u16 (*bgm)[__BGM_CHANNELS];																	\
																										\
	/* fx sound */																						\
	const u16* fxSound[__FXS];																			\
																										\
	/* space position of each fx */																		\
	VBVec2D fxPosition[__FXS];																			\


__CLASS_DEFINITION(SoundManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;

static void SoundManager_constructor(SoundManager this);
static void SoundManager_continuePlayingBGM(SoundManager this);
static void SoundManager_continuePlayingFxSounds(SoundManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(SoundManager);

// class's constructor
static void SoundManager_constructor(SoundManager this)
{
	ASSERT(this, "SoundManager::constructor: null this");

	__CONSTRUCT_BASE();

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

			this->fxPosition[i].parallax = -10000;
		}
	}
}

// class's destructor
void SoundManager_destructor(SoundManager this)
{
	ASSERT(this, "SoundManager::destructor: null this");

	__SINGLETON_DESTROY;
}

// load wave form data in the VB memory
void SoundManager_setWaveForm(SoundManager this)
{
	ASSERT(this, "SoundManager::setWaveForm: null this");

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

// load a bgm
void SoundManager_playBGM(SoundManager this, const u16 (*bgm)[])
{
	ASSERT(this, "SoundManager::loadBGM: null this");

	SoundManager_stopAllSound(this);
	this->bgm = bgm;
}

void SoundManager_playSounds(SoundManager this)
{
	ASSERT(this, "SoundManager::playSounds: null this");

	SoundManager_continuePlayingBGM(this);
	SoundManager_continuePlayingFxSounds(this);
}

// play background song loaded
static void SoundManager_continuePlayingBGM(SoundManager this)
{
	ASSERT(this, "SoundManager::playBGM: null this");

	int channel = 0;

	int note = 0;

	int i;

	//only if bgm loaded
	if(this->bgm != NULL)
	{
		//check if note's length has been played
		if(this->noteWait[0] > this->bgm[0][1])
		{
			//move to the next note
			this->actualNote[0]++;

			//initialize this->noteWait[0]
			this->noteWait[0] = 0;

			//if note is greater than song's length
			if(this->actualNote[0] >= this->bgm[0][0])
			{
				//rewind song
				this->actualNote[0] = 0;
			}
		}

		//if note has changed
		if(!this->noteWait[0])
		{
			for(channel = 0, i = 0; channel < 2; channel++)
			{
				//stop sound on the current channel
				/* There is a bug which makes the sound of
				 * SND_REGS 0 to not stop if not explicitly
				 * done.
				 */

				SND_REGS[channel].SxINT = 0x00;

				//grab note
				for(;i<2 && !this->bgm[this->actualNote[0]+3][i];i++);

				if(i<2)
				{
					note = this->bgm[this->actualNote[0] + 3][i];
				}

				//if note is not off
				if(note != 0)
				{
					//set note's output level
					SND_REGS[channel].SxLRV = this->bgm[0][2];

					//set note's frequency
					SND_REGS[channel].SxFQL = (note & 0xFF);
					SND_REGS[channel].SxFQH = (note >> 8);

					//set note's envelope
					SND_REGS[channel].SxEV0 = this->bgm[0][3];

					//set note's envelope mode
					SND_REGS[channel].SxEV1 = this->bgm[0][4];

					//set waveform source
					SND_REGS[channel].SxRAM = this->bgm[0][5];

					//output note
					SND_REGS[channel].SxINT = 0x80;
				}

				//not sure about this
				if(channel == 4)
				{
					SND_REGS[channel].S5SWP = this->bgm[0][5];
				}
			}
		}
		this->noteWait[0]++;
	}
}


// calculate sound volume according to its spatial position
static int SoundManager_calculateSoundPosition(SoundManager this, int fxS)
{
	ASSERT(this, "SoundManager::calculateSoundPosition: null this");

	float zMinus = 0;
	int output = 0x00;
	int maxOutputLevel = __MAXIMUM_OUTPUT_LEVEL;

	/* The maximum sound level for each side is 0xF
	 * In the center position the output level is the one
	 * defined in the sound's definition */
//	if(-10000 != this->fxPosition[fxS].parallax )
	{
		//zMinus = this->fxPosition[fxS].parallax ;//* this->zFactor;

		maxOutputLevel -= zMinus;

		if(maxOutputLevel > 0)
		{
			int leftDistance = abs(FIX19_13TOI(this->fxPosition[fxS].x) - __LEFT_EAR_CENTER);
			int rightDistance = abs(FIX19_13TOI(this->fxPosition[fxS].x) - __RIGHT_EAR_CENTER);
			int leftMinus = 0, rightMinus = 0;
			int leftOutput, rightOutput;

			//calculate the amount of sound that reachs each ear
			//xDistance / (384/15)
			leftMinus = leftDistance / ((1 << _optical->maximumViewDistancePower) / maxOutputLevel);
			rightMinus = rightDistance / ((1 << _optical->maximumViewDistancePower) / maxOutputLevel);

			leftOutput = maxOutputLevel - leftMinus;
			rightOutput = maxOutputLevel - rightMinus;

			if(0 < leftOutput)
			{
				output |= (((int)leftOutput) << 4);
			}

			if(0 < rightOutput)
			{
				output|=(((int)rightOutput));
			}
			else
			{
				output &= 0xF0;
			}
		}
	}

	return output;
}

// play sound
void SoundManager_continuePlayingFxSounds(SoundManager this)
{
	ASSERT(this, "SoundManager::playFxSounds: null this");

	int note = 0;
	int fxS = 0;

	for(; fxS < __FXS; fxS++)
	{
		//only if fx defined
		if(this->fxSound[fxS])
		{
			//check if note's length has been played
			if(this->noteWait[fxS + 1] > this->fxSound[fxS][1])
			{
				//move to the next note
				this->actualNote[fxS + 1]++;

				//initialize this->noteWait[0]
				this->noteWait[fxS + 1] = 0;

				//if note if greater than song's length
				if(this->actualNote[fxS+1] > this->fxSound[fxS][0])
				{
					//stop sound
					this->fxSound[fxS] = NULL;
					this->fxPosition[fxS].parallax = -10000;
					this->noteWait[fxS + 1] = 0;
					this->actualNote[fxS + 1] = 0;
					SND_REGS[fxS + 2].SxLRV = 0x00;
					SND_REGS[fxS + 2].SxINT = 0x00;

					continue;
				}
			}
			
			//if note has changed
			if(!this->noteWait[fxS + 1])
			{
				//stop sound on the current channel
				/* There is a bug which makes the sound of
				 * SND_REGS 0 to not stop if not explicitly
				 * done.
				 */
				SND_REGS[fxS + 2].SxINT = 0x00;

				//grab note
				note=this->fxSound[fxS][this->actualNote[fxS+1]+6];

				//if note is not off
				if(note != 0)
				{
					//if sound is positioned
					SND_REGS[fxS + 2].SxLRV = SoundManager_calculateSoundPosition(this, fxS);

					//set note's frequency
					SND_REGS[fxS + 2].SxFQL = (note & 0xFF);
					SND_REGS[fxS + 2].SxFQH = (note >> 8);

					//set note's envelope
					SND_REGS[fxS + 2].SxEV0 = this->fxSound[fxS][3];

					//set note's envelope mode
					SND_REGS[fxS + 2].SxEV1 = this->fxSound[fxS][4];

					//set waveform source
					SND_REGS[fxS + 2].SxRAM = this->fxSound[fxS][5];

					//output note
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

// load a fx sound to be played
// it is not guaranted that the sound has been loaded
int SoundManager_playFxSound(SoundManager this, const u16* fxSound, VBVec3D  position)
{
	ASSERT(this, "SoundManager::loadFxSound: null this");

	int i = 0;

	// try to find a free channel
	for(;i < __FXS && this->fxSound[i]; i++);

	// if a channel was available
	if(i < __FXS)
	{
		// record the fx definition's address
		this->fxSound[i] = fxSound;

		// set position inside screen coordinates
		__OPTICS_NORMALIZE(position);

		// save position for 3d sound
		__OPTICS_PROJECT_TO_2D(position, this->fxPosition[i]);
		return true;
	}

	// no channel available
	return false;
}

// returns true if the sound is being played
int SoundManager_playingSound(SoundManager this, const u16* fxSound)
{
	ASSERT(this, "SoundManager::playingSound: null this");

	int i = 0;

	// find sound
	for(;i<__FXS && this->fxSound[i] != fxSound; i++);

	// if sound found
	if(i<__FXS)
	{
		return true;
	}

	return false;
}

// stop sound
void SoundManager_stopSound(SoundManager this, BYTE *sound)
{
	ASSERT(this, "SoundManager::stopSound: null this");

	// TODO: complete implementation
	// disables sound on all channels
	/* if not explicitly done
	 * SND_REGS 0 doesn't get off
	 */
	SND_REGS[0].SxINT = 0x00;
	SND_REGS[1].SxINT = 0x00;
	SND_REGS[2].SxINT = 0x00;
	SND_REGS[3].SxINT = 0x00;
	SND_REGS[4].SxINT = 0x00;
	SND_REGS[5].SxINT = 0x00;
}

/*
// continue BGM play
void SoundManager_continueBGM(SoundManager this, BYTE *sound)
{
	ASSERT(this, "SoundManager::continueBGM: null this");

	// TODO:
}
*/

// stop all playing sounds
void SoundManager_stopAllSound(SoundManager this)
{
	ASSERT(this, "SoundManager::stopAllSound: null this");

	int channel = 0;

	//disables sound on all channels
	for(channel = 0; channel < 6; channel++)
	{
		SND_REGS[channel].SxINT = 0x00;
	}
	SSTOP=0x01;
}