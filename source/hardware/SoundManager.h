/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Sound.h>


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
	kPlayAsSoonAsPossible,			// Plays all tracks deallocating previous sound if necessary
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
singleton class SoundManager : ListenerObject
{
	VirtualList sounds;
	VirtualList soundsMIDI;
	VirtualList soundsPCM;
	VirtualNode soundMIDINode;
	VirtualList queuedSounds;
	Channel channels[__TOTAL_CHANNELS];
	Waveform waveforms[__TOTAL_WAVEFORMS];
	uint32 targetPCMUpdates;
	uint16 pcmTargetPlaybackFrameRate;
	uint16 MIDIPlaybackCounterPerInterrupt;
	bool lock;

	/// @publicsection
	static SoundManager getInstance();
	static void playSounds(uint32 elapsedMicroseconds);

	void reset();

	void setTargetPlaybackFrameRate(uint16 pcmTargetPlaybackFrameRate);

	void update();

	void stopAllSounds(bool release, SoundSpec** excludedSounds);
	void flushQueuedSounds();

	bool playSound(const SoundSpec* soundSpec, uint32 command, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, ListenerObject scope);
	Sound getSound(const SoundSpec* soundSpec, uint32 command, EventListener soundReleaseListener, ListenerObject scope);
	Sound findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope);

	void releaseSound(Sound sound);
	void releaseChannels(VirtualList channels);
	void deferMIDIPlayback(uint32 MIDIPlaybackCounterPerInterrupt);
	bool isPlayingSound(const SoundSpec* soundSpec);

	void lock();
	void unlock();

	void print();
	void printPlaybackTime();
}

extern SoundRegistry* const _soundRegistries;

#endif
