/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Utilities.h>
#include <Timer.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define TERMINAL_TIME				Terminal::print(Utilities::itoa(Timer::getTotalElapsedMilliseconds(), 10, 1));
#define TERMINAL_INT(n)			Terminal::print(Utilities::itoa(n, 10, 1));
#define TERMINAL_TEXT(t)			Terminal::print(t);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Terminal
///
/// Inherits from Object
///
/// Allows printing to supporting emulators.
static class Terminal : Object
{
	/// @publicsection

	/// Prints the provided text to the emulator's Terminal.
	/// @param text: Pointer to the string to print
	static void print(char* text);
}

#endif
