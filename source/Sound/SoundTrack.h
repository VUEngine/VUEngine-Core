/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_TRACK_H_
#define SOUND_TRACK_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAXIMUM_VOLUME	 0xF

#define C_0					 0x00
#define CS0					 0x00
#define D_0					 0x00
#define DS0					 0x00
#define E_0					 0x00
#define F_0					 0x00
#define FS0					 0x00
#define G_0					 0x00
#define GS0					 0x00
#define A_0					 0x00
#define AS0					 0x00
#define B_0					 0x00
#define C_1					 0x00
#define CS1					 0x00
#define D_1					 0x00
#define DS1					 0x00
#define E_1					 0x00
#define F_1					 0x00
#define FS1					 0x00
#define G_1					 0x00
#define GS1					 0x00
#define A_1					 0x00
#define AS1					 0x00
#define B_1					 0x00
#define C_2					 0x00
#define CS2					 0x00
#define D_2					 0x00
// Beginning of VB audible range
#define DS2					 0x27
#define E_2					 0x98
#define F_2					 0x102
#define FS2					 0x167
#define G_2					 0x1C6
#define GS2					 0x21F
#define A_2					 0x274
#define AS2					 0x2C3
#define B_2					 0x30F
#define C_3					 0x356
#define CS3					 0x399
#define D_3					 0x3D8
#define DS3					 0x414
#define E_3					 0x44C
#define F_3					 0x481
#define FS3					 0x4B3
#define G_3					 0x4E3
#define GS3					 0x510
#define A_3					 0x53A
#define AS3					 0x562
#define B_3					 0x587
#define C_4					 0x5AB
#define CS4					 0x5CC
#define D_4					 0x5EC
#define DS4					 0x60B
#define E_4					 0x626
#define F_4					 0x640
#define FS4					 0x659
#define G_4					 0x672
#define GS4					 0x688
#define A_4					 0x69D
#define AS4					 0x6B1
#define B_4					 0x6C4
#define C_5					 0x6D5
#define CS5					 0x6E6
#define D_5					 0x6F6
#define DS5					 0x705
#define E_5					 0x713
#define F_5					 0x720
#define FS5					 0x72D
#define G_5					 0x739
#define GS5					 0x744
#define A_5					 0x74E
#define AS5					 0x758
#define B_5					 0x762
#define C_6					 0x76B
#define CS6					 0x773
#define D_6					 0x77B
#define DS6					 0x782
#define E_6					 0x78A
#define F_6					 0x790
#define FS6					 0x796
#define G_6					 0x79C
#define GS6					 0x7A2
#define A_6					 0x7A8
#define AS6					 0x7AC
#define B_6					 0x7B1
#define C_7					 0x7B5
#define CS7					 0x7BA
#define D_7					 0x7BD
#define DS7					 0x7C1
#define E_7					 0x7C5
#define F_7					 0x7C8
#define FS7					 0x7CB
#define G_7					 0x7CE
#define GS7					 0x7D1
#define A_7					 0x7D4
#define AS7					 0x7D6
#define B_7					 0x7D8
#define C_8					 0x7DB
#define CS8					 0x7DD
#define D_8					 0x7DF
// End of VB audible range
// (Higher sounds are audible, but will not produce a pure, specific note)
#define DS8					 0x7E1
#define E_8					 0x7E2
#define F_8					 0x7E4
#define FS8					 0x7E6
#define G_8					 0x7E7
#define GS8					 0x7E8
#define A_8					 0x7EA
#define AS8					 0x7EB
#define B_8					 0x7EC
#define C_9					 0x7ED
#define CS9					 0x00  // 8869,84
#define D_9					 0x00  // 9397,27
#define DS9					 0x00  // 9956,06
#define E_9					 0x00  // 10548,08
#define F_9					 0x00  // 11175,30
#define FS9					 0x00  // 11839,82
#define G_9					 0x00  // 12543,85
#define GS9					 0x00  // 13289,75
#define A_9					 0x00  // 14080,00
#define AS9					 0x00  // 14917,24
#define B_9					 0x00  // 15804,27
#define C_10				 0x00  // 16744,04
#define CS10				 0x00  // 17739,69
#define D_10				 0x00  // 18794,55
#define DS10				 0x00  // 19912,13
#define E_10				 0x00  // 21096,16
#define F_10				 0x00  // 22350,61
#define FS10				 0x00  // 23679,64
#define G_10				 0x00  // 25087,71

#define MINIMUM_AUDIBLE_NOTE DS2
#define MAXIMUM_AUDIBLE_NOTE D_8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Sound track types
/// @memberof SoundTrack
enum SoundTrackTypes
{
	kTrackNative = 0,
	kTrackPCM
};

/// Sound events that the sound player recongnizes
/// @memberof SoundTrack
enum SoundEvents
{
	kSoundTrackEventEnd = 1 << (0),
	kSoundTrackEventStart = 1 << (1),
	kSoundTrackEventSxINT = 1 << (2),
	kSoundTrackEventSxLRV = 1 << (3),
	kSoundTrackEventSxFQ = 1 << (4),
	kSoundTrackEventSxEV0 = 1 << (5),
	kSoundTrackEventSxEV1 = 1 << (6),
	kSoundTrackEventSxRAM = 1 << (7),
	kSoundTrackEventSxSWP = 1 << (8),
	kSoundTrackEventSxMOD = 1 << (9),
	kSoundTrackEventSweepMod = 1 << (10),
	kSoundTrackEventNoise = 1 << (11)
};

/// A sound keyframe
/// @memberof SoundTrack
typedef struct SoundTrackKeyframe
{
	/// Tick
	uint16 tick;

	/// Events flag
	uint16 events;

} SoundTrackKeyframe;

/// A Sound Track
/// @memberof SoundTrack
typedef struct SoundTrackSpec
{
	/// kTrackNative, kTrackPCM
	uint32 trackType;

	/// Skip if no sound source available?
	bool skippable;

	/// Total number of samples
	uint32 samples;

	/// Keyframes that define the track
	SoundTrackKeyframe* trackKeyframes;

	/// SxINT values
	uint8* SxINT;

	/// SxLRV values
	uint8* SxLRV;

	/// SxFQH and SxFQL values
	uint16* SxFQ;

	/// SxEV0 values
	uint8* SxEV0;

	/// SxEV1 values
	uint8* SxEV1;

	/// SxRAM pointers
	int8** SxRAM;

	/// SxSWP values
	uint8* SxSWP;

	/// SxMOD pointers
	int8** SxMOD;

} SoundTrackSpec;

/// A SoundTrack spec that is stored in ROM
/// @memberof SoundTrack
typedef const SoundTrackSpec SoundTrackROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class SoundTrack
///
/// Inherits from Object
///
/// Implements a sound track.
class SoundTrack : Object
{
	/// @protectedsection

	/// Pointer to the spec that defines how to initialize the soundtrack
	const SoundTrackSpec* soundTrackSpec;

	/// Channel's effective length
	uint32 samples;

	/// Position within the sound track
	uint32 cursor;

	/// SxINT cursor
	uint32 cursorSxINT;

	/// SxLRV cursor
	uint32 cursorSxLRV;

	/// SxFQ cursor
	uint32 cursorSxFQ;

	/// SxEV0 cursor
	uint32 cursorSxEV0;

	/// SxEV1 cursor
	uint32 cursorSxEV1;

	/// SxRAM cursor
	uint32 cursorSxRAM;

	/// SxSWP cursor
	uint32 cursorSxSWP;

	/// SxMOD cursor
	uint32 cursorSxMOD;

	/// Total number of ticks
	uint32 ticks;

	/// Ticks before moving the cursor
	fix7_9_ext elapsedTicks;

	/// Next ticks target
	fix7_9_ext nextElapsedTicksTarget;

	/// If true, the playback is complete
	bool finished;

	/// @publicsection

	/// Set the target refresh rate for PCM playback.
	/// @param pcmTargetPlaybackRefreshRate: Target refresh rate for PCM playback
	static void setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate);

	/// Class' constructor
	/// @param soundTrackSpec: Specification that determines how to configure the sound track
	void constructor(const SoundTrackSpec* soundTrackSpec);

	/// Start the playback.
	/// @param wasPaused: If true, the playback is resuming
	void start(bool wasPaused);

	/// Stop the playback.
	void stop();

	/// Pause the playback.
	void pause();

	/// Unpause the playback.
	void unpause();

	/// Suspend the output of sound.
	void suspend();

	/// Resume the output of sound.
	void resume();

	/// Rewind the track.
	void rewind();

	/// Advance the playback on the sound's MIDI tracks.
	/// @param elapsedMicroseconds: Elapsed time since the last call
	/// @param targetPCMUpdates: Ideal Elapsed time since the last call
	/// @param tickStep: Tick step per timer interrupt
	/// @param targetTimerResolutionFactor: Factor to apply to the tick step
	/// @param leftVolumeFactor: Factor to apply to the left speaker's volume
	/// @param rightVolumeFactor: Factor to apply to the right speaker's volume
	/// @param volumeReduction: Volume reduction used for fade effects
	/// @param volumenScalePower: 2's power to divide to the final volume value
	/// @param frequencyDelta: added to the frequency registers
	/// @return True if the playback is complete; false otherwise
	bool update(
		uint32 elapsedMicroseconds, uint32 targetPCMUpdates, fix7_9_ext tickStep,
		fix7_9_ext targetTimerResolutionFactor, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor,
		int8 volumeReduction, uint8 volumenScalePower, uint16 frequencyDelta
	);

	/// Retrieve the sound track's total ticks.
	/// @return Total number of ticks
	uint32 getTicks();

	/// Retrieve the sound track's percentage of elapsed ticks.
	/// @return Percentaje of elapsed ticks
	float getElapsedTicksPercentaje();

	/// Retrieve the total time of playback in milliseconds.
	/// @param targetTimerResolutionUS: Target timer resolution in US
	/// @return The total time of playback in milliseconds
	uint32 getTotalPlaybackMilliseconds(uint16 targetTimerResolutionUS);
}

#endif
