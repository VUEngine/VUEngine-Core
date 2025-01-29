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

	/// Play a sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param position: Position for spatilly position sound
	/// @param playbackType: How to play the sound
	/// @param scope: Object on which to perform the callback
	static bool playSound
	(
		const SoundSpec* soundSpec, const Vector3D* position, uint32 playbackType, ListenerObject scope
	);

	/// Allocate sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object that will be notified of communication events
	static Sound getSound(const SoundSpec* soundSpec, ListenerObject scope);

	/// Retrieve a previously allocated sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object that will be notified of communication events
	static Sound findSound(const SoundSpec* soundSpec, ListenerObject scope);

	/// Update the sounds lists.
	void updateSounds();

	/// Play the allocated sounds.
	/// @param elapsedMicroseconds: Elapsed time between call
	void playSounds(uint32 elapsedMicroseconds);

	/// Reset the manager's state.
	void reset();

	/// Set the target refresh rate for PCM playback.
	/// @param pcmTargetPlaybackRefreshRate: Target refresh rate for PCM playback
	void setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate);

	///  Check if a sound with the provided spec is playing.
	/// @param soundSpec: Sound spec to check for
	bool isPlayingSound(const SoundSpec* soundSpec);

	/// Mute all playing sounds
	void muteAllSounds();

	/// Unmute all playing sounds
	void unmuteAllSounds();

	/// Rewind all playing sounds
	void rewindAllSounds();

	/// Stop all playing sounds.
	/// @param release: If true, sounds are not only stopped but released
	/// @param excludedSounds: Array of sound specs to not stop
	void stopAllSounds(bool release, SoundSpec** excludedSounds);

	/// Refuse petitions to play or allocate sounds are processed.
	void lock();

	/// Allow petitions to play or allocate sounds are processed.
	void unlock();

	/// Print the manager's status.
	void print(int32 x, int32 y);

	/// Print playback time of the playing sounds.
	void printPlaybackTime(int32 x, int32 y);
}

#endif
