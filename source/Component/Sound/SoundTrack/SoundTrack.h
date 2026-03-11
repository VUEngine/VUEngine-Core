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
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Sound events that the sound player recongnizes
/// @memberof SoundTrack
enum SoundEvents
{
	kSoundTrackEventEnd = 1 << (0),
	kSoundTrackEventStart = 1 << (1)
};

/// A sound keyframe
/// @memberof SoundTrack
typedef struct SoundTrackKeyframe
{
	/// Amount of ticks that the sound effect must be active
	uint16 tick;

	/// Events flag
	uint16 events;

} SoundTrackKeyframe;

/// A Sound Track
/// @memberof SoundTrack
typedef struct SoundTrackSpec
{
	/// Class' allocator
	AllocatorPointer allocator;

	/// Priority for sound channel usage
	uint8 priority;

	/// Skip if no sound source available?
	bool skip;

	/// Loop back point (cursor)
	uint32 loopPointCursor;

	/// Keyframes that define the track
	SoundTrackKeyframe* trackKeyframes;

} SoundTrackSpec;

/// A SoundTrack spec that is stored in ROM
/// @memberof SoundTrack
typedef const SoundTrackSpec SoundTrackROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SoundTrack
///
/// Inherits from Object
///
/// Implements a sound track.
abstract class SoundTrack : Object
{
	/// @protectedsection

	/// Pointer to the spec that defines how to initialize the soundtrack
	const SoundTrackSpec* soundTrackSpec;

	/// Identifier provided to the SoundUnit
	uint32 id;

	/// Channel's effective length
	uint32 samples;

	/// Position within the sound track
	uint32 cursor;

	/// Total number of ticks
	uint32 ticks;

	/// Ticks before moving the cursor
	fix7_9_ext elapsedTicks;

	/// Next ticks target
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

	/// Loop the track.
	/// @return Total elapsed ticks from the beginning of the track to the looping cursor's position
	fix7_9_ext loop();

	/// Advance the playback on the sound's MIDI tracks.
	/// @param tickStep: Tick step per timer interrupt
	/// @param targetTimerResolutionFactor: Factor to apply to the tick step
	/// @param maximumVolume: Maximum volume for the sound track's playback
	/// @param leftVolumeReduction: Volume reduction to apply to the left speaker's volume
	/// @param rightVolumeReduction: Volume reduction to apply to the right speaker's volume
	/// @param volumeReduction: Volume reduction used for fade effects
	/// @param frequencyDelta: added to the frequency registers
	/// @return True if the playback is complete; false otherwise
	bool update
	(
		fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, uint8 maximumVolume, uint8 leftVolumeReduction,
		uint8 rightVolumeReduction, uint8 volumeReduction, uint16 frequencyDelta
	);

	/// Retrieve the sound track's total ticks.
	/// @return Total number of ticks
	uint32 getTicks();

	/// Retrieve the number of elapsed this for the track.
	/// @return Total elapsed ticks so far
	fix7_9_ext getElapsedTicks();

	/// Retrieve the sound track's percentage of elapsed ticks.
	/// @return Percentage of elapsed ticks
	float getElapsedTicksPercentage();

	/// Retrieve the total time of playback in milliseconds.
	/// @param targetTimerResolutionUS: Target timer resolution in US
	/// @return The total time of playback in milliseconds
	uint32 getTotalPlaybackMilliseconds(uint16 targetTimerResolutionUS);

	/// Print the indexes of the track.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int16 x, int16 y);

	/// Reset the sound track.
	virtual void reset();

	/// Update the sound track cursors.
	virtual void updateCursors();	

	/// Send the sound request to the sound unit.
	/// @param targetTimerResolutionFactor: Factor to apply to the tick step
	/// @param maximumVolume: Maximum volume for the sound track's playback
	/// @param leftVolumeReduction: Volume reduction to apply to the left speaker's volume
	/// @param rightVolumeReduction: Volume reduction to apply to the right speaker's volume
	/// @param volumeReduction: Volume reduction used for fade effects
	/// @param frequencyDelta: added to the frequency registers
	/// @return True if the playback is complete; false otherwise
	virtual void sendSoundRequest
	(
		fix7_9_ext tickStep, fix7_9_ext targetTimerResolutionFactor, uint8 maximumVolume, uint8 leftVolumeReduction,
		uint8 rightVolumeReduction, uint8 volumeReduction, uint16 frequencyDelta
	);
}

#endif
