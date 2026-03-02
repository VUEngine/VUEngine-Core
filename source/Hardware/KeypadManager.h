/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef KEY_PAD_MANAGER_H_
#define KEY_PAD_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// User's input
/// @memberof KeypadManager
typedef struct UserInput
{
	/// All pressed key(s)
	uint16 allKeys;

	/// Pressed key(s) just in the last cycle
	uint16 pressedKey;

	/// Released key(s) just in the last cycle
	uint16 releasedKey;

	/// Held key(s)
	uint16 holdKey;

	/// How long the key(s) have been held (in game frames)
	uint32 holdKeyDuration;

	/// Previously pressed key(s)
	uint16 previousKey;

	/// Low power flag
	uint16 powerFlag;

	/// Dummy input to force user input processing even if
	/// there is not a real one
	uint16 dummyKey;

} UserInput;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class KeypadManager
///
/// Inherits from ListenerObject
///
/// Manages keypad inputs.
singleton class KeypadManager : ListenerObject
{
	/// @publicsection

	/// Interrupt handler for keypad's interrupts
	static void interruptHandler();

	/// Enable user input.
	static void enable();

	/// Disable user input.
	static void disable();

	/// Check if user input is enabled.
	/// @return True if user input is enabled
	static int32 isEnabled();

	/// Enable the dummy key to force user input processing.
	static void enableDummyKey();

	/// Disable the dummy key to not enforce user input processing.
	static void disableDummyKey();

	/// Register the user input according to the provided flags.
	/// @param inputToRegister: Flags to select which inputs to register and which to ignore
	static void registerInput(uint16 inputToRegister);

	/// Retrieve the user input during the last game frame
	/// @return User input struct with the key presses of the last game frame
	static UserInput getUserInput();

	/// Retrieve the accumulated sum of user inputs since the start of the program.
	/// @return The mathematical sum of all user's presses.
	static uint32 getAccumulatedUserInput();

	/// Print the last reads of user input.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);

	/// Reset the manager's state.
	static void reset();

	/// Reat the user input during the last game frame
	/// @param waitForStableReading: If true, wait for reading to be stable
	static void readUserInput(bool waitForStableReading);
}

#endif
