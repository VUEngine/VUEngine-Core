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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __MAXIMUM_VOLUME						0xF


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Sound track types
/// @memberof SoundTrack
enum SoundTrackTypes
{
	kTrackNative 			= 0,
	kTrackPCM
};

/// Sound events that the sound player recongnizes
/// @memberof SoundTrack
enum SoundEvents
{
	kSoundTrackEventEnd				=			1 << (0),
	kSoundTrackEventStart			= 			1 << (1),
	kSoundTrackEventSxINT		 	=			1 << (2),
	kSoundTrackEventSxLRV		 	=			1 << (3),
	kSoundTrackEventSxFQ			=			1 << (4),
	kSoundTrackEventSxEV0		 	=			1 << (5),
	kSoundTrackEventSxEV1		 	=			1 << (6),
	kSoundTrackEventSxRAM		 	=			1 << (7),
	kSoundTrackEventSxSWP		 	=			1 << (8)
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

} SoundTrackSpec;

/// A SoundTrack spec that is stored in ROM
/// @memberof Sound
typedef const SoundTrackSpec SoundTrackROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class SoundTrack
///
/// Inherits from Object
///
/// Implements a sound track.
/// @ingroup sound
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

	/// Total number of ticks
	uint32 ticks;

	/// Ticks before moving the cursor
	fix7_9_ext elapsedTicks;

	/// Next curst ticks target
	fix7_9_ext nextElapsedTicksTarget;

	/// If true, the playback is complete
	bool finished;

	/// @publicsection

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
	/// @param tickStep: Tick step per timer interrupt
	/// @param targetTimerResolutionFactor: Factor to apply to the tick step
	/// @param leftVolumeFactor: Factor to apply to the left speaker's volume
	/// @param rightVolumeFactor: Factor to apply to the right speaker's volume
	/// @param volumeReduction: Volume reduction used for fade effects
	/// @param volumenScalePower: 2's power to divide to the final volume value
	/// @return True if the playback is complete; false otherwise
	bool update(fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor, int8 volumeReduction, uint8 volumenScalePower);

	/// Retrieve the sound track's total ticks.
	/// @return Total number of ticks
	uint32 getTicks();
}


#endif
