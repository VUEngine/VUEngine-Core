/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_TEST_H_
#define SOUND_TEST_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Tool.h>
#include <Sound.h>
#include <SoundManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class SoundTest
///
/// Inherits from Tool
///
/// Implements a tool to test sounds.
singleton class SoundTest : Tool
{
	/// Currently tested sound
	Sound sound;

	/// Index of the selected sound
	uint16 soundIndex;

	/// @publicsection

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Update the tool's state.
	override void update();

	/// Show the tool.
	override void show();

	/// Hide the tool.
	override void hide();

	/// Process the provided user pressed key.
	/// @param pressedKey: User pressed key
	override void processUserInput(uint16 pressedKey);
}

#endif
