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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>
#include <Sound.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __DEFAULT_PCM_HZ					8000

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SoundManager
///
/// Inherits from ListenerObject
///
/// Manages the Sound instances.
singleton class SoundManager : ListenerObject
{
	/// @protectedsection

	/// List of playing sounds
	VirtualList sounds;

	/// Target PCM cycles per game cycle
	uint32 targetPCMUpdates;

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
	static void reset();

	/// Set the target refresh rate for PCM playback.
	/// @param pcmTargetPlaybackRefreshRate: Target refresh rate for PCM playback
	static void setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate);

	///  Check if a sound with the provided spec is playing.
	/// @param soundSpec: Sound spec to check for
	static bool isPlayingSound(const SoundSpec* soundSpec);

	/// Play a sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param position: Position for spatilly position sound
	/// @param playbackType: How to play the sound
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	static bool playSound
	(
		const SoundSpec* soundSpec, const Vector3D* position, uint32 playbackType, 
		EventListener soundReleaseListener, ListenerObject scope
	);

	/// Allocate sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	static Sound getSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope);

	/// Retrieve a previously allocated sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object on which to perform the callback
	static Sound findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope);

	/// Mute all playing sounds
	static void muteAllSounds();

	/// Unmute all playing sounds
	static void unmuteAllSounds();

	/// Rewind all playing sounds
	static void rewindAllSounds();

	/// Stop all playing sounds.
	/// @param release: If true, sounds are not only stopped but released
	/// @param excludedSounds: Array of sound specs to not stop
	static void stopAllSounds(bool release, SoundSpec** excludedSounds);

	/// Refuse petitions to play or allocate sounds are processed.
	static void lock();

	/// Allow petitions to play or allocate sounds are processed.
	static void unlock();

	/// Print the manager's status.
	static void print(int32 x, int32 y);

	/// Print playback time of the playing sounds.
	static void printPlaybackTime(int32 x, int32 y);
}

#endif
