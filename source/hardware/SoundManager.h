/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>
#include <Sound.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Sound Registry
typedef struct SoundRegistry
{
	// This table is for the most part untested, but looks to be accurate
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


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __DEFAULT_PCM_HZ					8000
#define __TOTAL_CHANNELS					6
#define __TOTAL_MODULATION_CHANNELS			1
#define __TOTAL_NOISE_CHANNELS				1
#define __TOTAL_NORMAL_CHANNELS				(__TOTAL_CHANNELS - __TOTAL_MODULATION_CHANNELS - __TOTAL_NOISE_CHANNELS)
#define __TOTAL_POTENTIAL_NORMAL_CHANNELS	(__TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS)
#define __TOTAL_WAVEFORMS					__TOTAL_POTENTIAL_NORMAL_CHANNELS


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class SoundManager
///
/// Inherits from ListenerObject
///
/// Manages the VSU.
/// @ingroup hardware
singleton class SoundManager : ListenerObject
{
	/// @protectedsection

	/// List of playing sounds
	VirtualList sounds;

	/// List of playing sounds with MIDI tracks
	VirtualList soundsMIDI;

	/// List of playing sounds with PCM tracks
	VirtualList soundsPCM;

	/// List of sounds pending playback
	VirtualList queuedSounds;

	/// Mapping of VSU channels
	Channel channels[__TOTAL_CHANNELS];

	/// Mapping of waveworms
	Waveform waveforms[__TOTAL_WAVEFORMS];

	/// Target PCM cycles per game cycle
	uint32 targetPCMUpdates;

	/// Target refresh rate for PCM playback 
	uint16 pcmTargetPlaybackRefreshRate;

	/// If raised, no petitions to play or allocate sounds are processed
	bool lock;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return SoundManager singleton
	static SoundManager getInstance();

	/// Play the allocated sounds.
	/// @param elapsedMicroseconds: Elapsed time between call
	static void playSounds(uint32 elapsedMicroseconds);

	/// Reset the manager's state.
	void reset();

	/// Update the manager.
	void update();

	/// Set the target refresh rate for PCM playback.
	/// @param pcmTargetPlaybackRefreshRate: Target refresh rate for PCM playback
	void setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate);

	///  Check if a sound with the provided spec is playing.
	/// @param soundSpec: Sound spec to check for
	bool isPlayingSound(const SoundSpec* soundSpec);

	/// Play a sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param command: Command for sound allocation priority
	/// @param position: Position for spatilly position sound
	/// @param playbackType: How to play the sound
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	bool playSound(const SoundSpec* soundSpec, uint32 command, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, ListenerObject scope);

	/// Allocate sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param command: Command for sound allocation priority
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	Sound getSound(const SoundSpec* soundSpec, uint32 command, EventListener soundReleaseListener, ListenerObject scope);

	/// Retrieve a previously allocated sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	Sound findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope);

	/// Mute all playing sounds of the provided type (MIDI or PCM).
	/// @param type: Type of sounds to mute
	void muteAllSounds(uint32 type);

	/// Unmute all playing sounds of the provided type (MIDI or PCM).
	/// @param type: Type of sounds to unmute
	void unmuteAllSounds(uint32 type);

	/// Rewind all playing sounds of the provided type (MIDI or PCM).
	/// @param type: Type of sounds to rewind
	void rewindAllSounds(uint32 type);

	/// Stop all playing sounds.
	/// @param excludedSounds: Array of sound specs to not stop
	void stopAllSounds(bool release, SoundSpec** excludedSounds);

	/// Release sound channels.
	/// @param channels: List of channels to release
	void releaseChannels(VirtualList channels);

	/// Refuse petitions to play or allocate sounds are processed.
	void lock();

	/// Allow petitions to play or allocate sounds are processed.
	void unlock();

	/// Discard sounds pending playback.
	void flushQueuedSounds();

	/// Print the manager's status.
	void print(int32 x, int32 y);

	/// Print playback time of the playing sounds.
	void printPlaybackTime(int32 x, int32 y);

	/// Print waveforms.
	void printWaveFormStatus(int32 x, int32 y);
}


#endif
