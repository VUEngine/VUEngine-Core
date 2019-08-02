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

#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <MiscStructs.h>
#include <SoundWrapper.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------

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


enum SoundRequestMessages
{
	kPlayAll = 0, 					// Sound is not allocated if there are not enough free channels to play all the sound's tracks
	kPlayAny,						// Plays as many sound's tracks as there are free channels
	kPlayForce,						// Plays the priority tracks deallocating previous sound if necessary
};

#define __DEFAULT_PCM_HZ	8000

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class SoundManager : Object
{
	VirtualList soundWrappers;
	VirtualList releasedSoundWrappers;
	Channel channels[__TOTAL_CHANNELS];
	Waveform waveforms[__TOTAL_CHANNELS];
	u32 elapsedMicroseconds;
	u16 pcmPlaybackCycles;
	u16 pcmTargetPlaybackFrameRate;
	s16 pcmPlaybackCyclesToSkip;
	s16 pcmStablePlaybackCycles;
	bool pcmFrameRateIsStable;

	/// @publicsection
	static SoundManager getInstance();
	void reset();

	void setTargetPlaybackFrameRate(u16 pcmTargetPlaybackFrameRate);

	void playMIDISounds();
	void playPCMSounds();
	void stopAllSounds();

	SoundWrapper playSound(Sound* sound, bool forceAllChannels, const Vector3D* position);
	SoundWrapper getSound(Sound* sound, bool forceAllChannels);

	void releaseSoundWrapper(SoundWrapper soundWrapper);
	void startPCMPlayback();

	void updateFrameRate(u16 gameFrameDuration);
	void print();
}


#endif
