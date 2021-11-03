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

#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <MiscStructs.h>
#include <SoundWrapper.h>
#include <WaveForms.h>


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
	uint8 SxINT; //		[----Enable----][--XXXXXXXXXX--][-Interval/??--][--------------------------------Interval Data---------------------------------]
	uint8 spacer1[3];
	uint8 SxLRV; //		[---------------------------L Level----------------------------][---------------------------R Level----------------------------]
	uint8 spacer2[3];
	uint8 SxFQL; //		[------------------------------------------------------Frequency Low Byte------------------------------------------------------]
	uint8 spacer3[3];
	uint8 SxFQH; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Frequency High Byte-------------]
	uint8 spacer4[3];
	uint8 SxEV0; //		[---------------------Initial Envelope Value-------------------][------U/D-----][-----------------Envelope Step----------------]
	uint8 spacer5[3];
			 //Ch. 1-4 	[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
			 //Ch. 5	[--XXXXXXXXXX--][------E/D-----][----?/Short---][--Mod./Sweep--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	uint8 SxEV1; //Ch. 6	[--XXXXXXXXXX--][----------------------E/D---------------------][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	uint8 spacer6[3];
	//Ch. 1-5 only (I believe address is only 3 bits, but may be 4, needs testing)
	uint8 SxRAM; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Waveform RAM Address------------]
	uint8 spacer7[3];
	//Ch. 5 only
	uint8 S5SWP; //		[------CLK-----][-------------Sweep/Modulation Time------------][------U/D-----][----------------Number of Shifts--------------]
	uint8 spacer8[35];
} SoundRegistry;


enum SoundRequestMessages
{
	kPlayAll = 0, 					// Sound is not allocated if there are not enough free channels to play all the sound's tracks
	kPlayAny,						// Plays as many sound's tracks as there are free channels
	kPlayForceAny,					// Plays the priority tracks deallocating previous sound if necessary
	kPlayForceAll,					// Plays all tracks deallocating previous sound if necessary
};

/*
enum ChannelTypes
{
	kChannelNormal0 		= 0,
	kChannelNormal1 		= 1,
	kChannelNormal2 		= 2,
	kChannelNormal3 		= 3,
	kChannelNormal4 		= 4,
	kChannelModulation0 	= 5,
	kChannelNoise0 			= 6,
};
*/

#define __DEFAULT_PCM_HZ					8000
#define __TOTAL_CHANNELS					6
#define __TOTAL_MODULATION_CHANNELS			1
#define __TOTAL_NOISE_CHANNELS				1
#define __TOTAL_NORMAL_CHANNELS				(__TOTAL_CHANNELS - __TOTAL_MODULATION_CHANNELS - __TOTAL_NOISE_CHANNELS)
#define __TOTAL_POTENTIAL_NORMAL_CHANNELS	(__TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS)
#define __TOTAL_WAVEFORMS					__TOTAL_POTENTIAL_NORMAL_CHANNELS

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class SoundManager : Object
{
	VirtualList soundWrappers;
	VirtualList releasedSoundWrappers;
	VirtualNode soundWrapperMIDINode;
	VirtualList queuedSounds;
	Channel channels[__TOTAL_CHANNELS];
	Waveform waveforms[__TOTAL_WAVEFORMS];
	uint16 pcmPlaybackCycles;
	uint16 pcmTargetPlaybackFrameRate;
	int16 pcmPlaybackCyclesToSkip;
	uint16 MIDIPlaybackCounterPerInterrupt;
	bool hasPCMSounds;
	bool lock;
	bool lockSoundWrappersList;

	/// @publicsection
	static SoundManager getInstance();
	void reset();

	void setTargetPlaybackFrameRate(uint16 pcmTargetPlaybackFrameRate);

	void update();

	bool playMIDISounds(uint32 elapsedMicroseconds);
	bool playPCMSounds();
	void stopAllSounds(bool release);
	void flushQueuedSounds();

	void playSound(const Sound* sound, uint32 command, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, Object scope);
	SoundWrapper getSound(const Sound* sound, uint32 command, EventListener soundReleaseListener, Object scope);

	void releaseSoundWrapper(SoundWrapper soundWrapper);
	void deferMIDIPlayback(uint32 MIDIPlaybackCounterPerInterrupt);
	void startPCMPlayback();
	bool isPlayingSound(const Sound* sound);

	void lock();
	void unlock();

	void updateFrameRate();
	void print();
	void printPlaybackTime();
}


#endif
