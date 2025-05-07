/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_H_
#define SOUND_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>
#include <SoundTrack.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Sound playback types
/// @memberof Sound
enum SoundPlaybackTypes
{
	kSoundPlaybackNone = 0,
	kSoundPlaybackNormal,
	kSoundPlaybackFadeIn,
	kSoundPlaybackFadeOut,
	kSoundPlaybackFadeOutAndRelease
};

/// Sound state
/// @memberof Sound
enum SoundState
{
	kSoundOff = 0,
	kSoundPaused,
	kSoundPlaying,
	kSoundFinished,
	kSoundRelease,
};

/// A Sound spec
/// @memberof Sound
typedef struct SoundSpec
{
	/// Name
	char* name;

	/// Play in loop
	bool loop;

	/// Tick duration in US
	uint16 targetTimerResolutionUS;

	/// Tracks
	SoundTrackSpec** soundTrackSpecs;

} SoundSpec;

/// A Sound spec that is stored in ROM
/// @memberof Sound
typedef const SoundSpec SoundROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Sound
///
/// Inherits from ListenerObject
///
/// Implements sound playback.
class Sound : ListenerObject
{
	/// @protectedsection

	/// Pointer to the spec that defines how to initialize the sound 
	const SoundSpec* soundSpec;

	/// Pointer to vector for spatial positioning of the sound
	const Vector3D* position;

	/// List of sound tracks
	VirtualList soundTracks;

	/// Main sound track
	SoundTrack mainSoundTrack;

	/// Playback speed
	fix7_9_ext speed;

	/// Sound's state
	uint32 state;

	/// Factor to apply to the tick step
	fix7_9_ext targetTimerResolutionFactor;

	/// Tick step per timer interrupt
	fix7_9_ext tickStep;

	/// Elapsed ticks in the previous update
	uint32 previouslyElapsedTicks;

	/// Total elapsed ticks since the track started to play
	fix7_9_ext totalElapsedTicks;

	/// Total playback time
	uint32 totalPlaybackMilliseconds;

	/// Target refresh rate for PCM playback
	uint16 pcmTargetPlaybackRefreshRate;

	/// Delta added to the frequency registers
	uint16 frequencyDelta;

	/// Multiplier used for fade effects
	uint16 volumeReductionMultiplier;

	/// Volume reduction used for fade effects
	int8 volumeReduction;

	/// 2's power to divide to the final volume value
	uint8 volumenScalePower;

	/// Type of playback to perform (SoundPlaybackTypes)
	uint8 playbackType;

	/// If true, sound is not muted
	bool unmute;

	/// If true, the sound is released when playback is complete
	bool autoReleaseOnFinish;

	/// If locked, it cannot be released by external calls
	bool locked;

	/// @publicsection

	/// Mirror the spatial positioning of the sound.
	/// @param mirror: Struct with a flag for each axis to mirror
	static void setMirror(Mirror mirror);

	/// Class' constructor
	/// @param soundSpec: Specification that determines how to configure the sound
	/// @param soundReleaseListener: Callback for when the sound is released
	/// @param scope: Object that will be notified of sound events
	void constructor(const SoundSpec* soundSpec, ListenerObject scope);

	/// Retrieve the spec pointer that defined how to initialized the sound
	/// @return Sound spec pointer
	const SoundSpec* getSpec();

	/// Play the sound.
	/// @param position: Pointer to the spatial position of the sound
	/// @param playbackType: Specifies how the playback should start
	void play(const Vector3D* position, uint32 playbackType);

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

	/// Mute the sound.
	void mute();

	/// Unmute the sound.
	void unmute();

	/// Rewind the playack
	void rewind();

	/// Release this sound.
	void release();

	/// Prevent other requests to get a sound to steal this sound's sources
	void lock();

	/// Allow other requests to get a sound to steal this sound's sources
	void unlock();

	/// Set the flag that allows the sound to auto release itself when playback is complete.
	/// @param autoReleaseOnFinish: If true, the sound is released when playaback is complete
	void autoReleaseOnFinish(bool autoReleaseOnFinish);

	/// Set the playback's speed.
	/// @param speed: Target playback speed
	void setSpeed(fix7_9_ext speed);

	/// Retrieve the playback's speed.
	/// @return Target playback speed
	fix7_9_ext getSpeed();

	/// Set the factor (2's power) by which the final volume is reduced.
	/// @param volumenScalePower: Factor by which the final volume is reduced
	void setVolumenScalePower(uint8 volumenScalePower);

	/// Set the frequency delta to be added to the VSU's frequency registers.
	/// @param frequencyDelta: Delta to be added to the frequency
	void setFrequencyDelta(uint16 frequencyDelta);

	/// Retrieve the frequency delta added to the VSU's frequency registers.
	uint16 getFrequencyDelta();

	/// Retrieve the sound track's elapsed ticks.
	/// @return Elapsed ticks since playback started
	uint32 getTotalElapsedTicks();

	/// Check if the sound is playing.
	/// @return True if playback is going on
	bool isPlaying();

	/// Check if the sound is paused.
	/// @return True if playback is paused
	bool isPaused();

	/// Check if the sound is fading in.
	/// @return True if playback fading in
	bool isFadingIn();

	/// Check if the sound is fading out.
	/// @return True if playback fading out
	bool isFadingOut();

	/// Update the sound playback state.
	/// @return False if the sound has been released
	bool updatePlaybackState();

	/// Advance the playback on the sound's native tracks.
	/// @param elapsedMicroseconds: Elapsed time since the last call
	/// @param targetPCMUpdates: Ideal Elapsed time since the last call
	void update(uint32 elapsedMicroseconds, uint32 targetPCMUpdates);

	/// Print the sounds's properties.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// Print the sounds's playback time.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printPlaybackTime(int32 x, int32 y);

	/// Print the sounds's playback progress.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printPlaybackProgress(int32 x, int32 y);
}

#endif
