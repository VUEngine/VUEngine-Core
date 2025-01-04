/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ERROR_H_
#define ERROR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Error
///
/// Inherits from Object
///
/// Catches assertions and hardware exceptions.
singleton class Error : Object
{
	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return Error singleton
	static Error getInstance();

	/// Locks the program in a screen with the details of what caused the exception.
	/// @param message: Pointer to a brief description of the exception
	/// @param detail: Pointer to a detailed description of the exception
	static void triggerException(char* message, char* detail);

	/// Handles hardware's zero division exception.
	static void zeroDivisionException();

	/// Handles hardware's invalid opcode exception.
	static void invalidOpcodeException();

	/// Handles hardware's floating point exception.
	static void floatingPointException();
}

#endif
