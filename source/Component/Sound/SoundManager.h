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

#include <ComponentManager.h>
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
/// Inherits from ComponentManager
///
/// Manages the Sound instances.
class SoundManager : ComponentManager
{
	/// @protectedsection

	/// List of playing sounds
	VirtualList sounds;

	/// If raised, no petitions to play or allocate sounds are processed
	bool lock;

	/// @publicsection

	/// Allocate sound defined by the provided spec.
	/// @param soundSpec: Spec that defines the sound to play
	/// @param soundReleaseListener: Callback method for when the sound is released
	/// @param scope: Object that will be notified of communication events
	static Sound getSound(const SoundSpec* soundSpec, ListenerObject scope);

	/// Class' constructor
	void constructor();

	/// Process an event that the instance is listening for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Retrieve the compoment type that the manager manages.
	/// @return Component type
	override uint32 getType();

	/// Enable the manager.
	override void enable();

	/// Disable the manager.
	override void disable();

	/// Create a sprite with the provided spec.
	/// @param owner: Object to which the sprite will attach to
	/// @param soundSpec: Spec to use to create the sound
	/// @return Created sound
	override Sound create(Entity owner, const SoundSpec* soundSpec);

	/// Force the purging of deleted components.
	override void purgeComponents();

	/// Update the sounds lists.
	void update();

	/// Check if a sound with the provided spec is playing.
	/// @param soundSpec: Sound spec to check for
	bool isPlayingSound(const SoundSpec* soundSpec);

	/// Retrive the first sound using the provided spec is playing.
	/// @param soundSpec: Sound spec to check for
	/// @return Sound using the provided spec 
	Sound getPlayingSound(const SoundSpec* soundSpec);

	/// Mute all playing sounds
	void muteAllSounds();

	/// Unmute all playing sounds
	void unmuteAllSounds();

	/// Rewind all playing sounds
	void rewindAllSounds();

	/// Pause any playing sound.
	void pauseSounds();

	/// Resume playback on all registerd sounds.
	void unpauseSounds();

	/// Stop all playing sounds.
	/// @param release: If true, sounds are not only stopped but released
	/// @param excludedSounds: Array of sound specs to not stop
	void stopAllSounds(bool release, SoundSpec** excludedSounds);

	/// Fade in or out the registered sounds
	/// @param playbackType: Specifies how the playback should start
	void fadeSounds(uint32 playbackType);

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