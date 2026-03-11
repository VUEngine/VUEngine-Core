/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_UNIT_H_
#define SOUND_UNIT_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SoundUnit
///
/// Inherits from Object
///
/// Manages the platform's sound hardware.
singleton class SoundUnit : Object
{
	/// @publicsection

	/// Play the allocated sounds.
	/// @param elapsedMicroseconds: Elapsed time between call
	static void playSounds(uint32 elapsedMicroseconds);

	/// Apply a sound source configuration to a VSU sound source with the provided data.
	/// @param soundSourceConfigurationRequest: VSU sound source configuration
	static void applySoundSourceConfiguration(const SoundSourceConfigurationRequest* soundSourceConfigurationRequest);

	/// Stop sound output in the sound sources in use by the requester Id.
	/// @param requester: Id of of the user of a sound source
	static void stopSoundSourcesUsedBy(uint32 requesterId);

	/// Print the manager's status.
	static void print(int32 x, int32 y);

	/// Print waveforms.
	static void printWaveFormStatus(int32 x, int32 y);

	/// Print channels' status.
	static void printChannels(int32 x, int32 y);

	/// Reset the manager's state.
	static void reset();

	/// Update the manager.
	static void update();

	/// Stop all sound sources.
	static void stopAllSounds();

	/// Enable queueing petitions to play sounds.
	static void enableQueue();

	/// Disable queueing petitions to play sounds (if there are no
	/// sound sources availables at the time of request, the petition
	/// is ignored).
	static void disableQueue();

	/// Flush all pending sound requests.
	static void flushQueuedSounds();
}

#endif
