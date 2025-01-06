/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;
class Clock;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ClockManager
///
/// Inherits from Object
///
/// Manages the instances of Clock.
singleton class ClockManager : Object
{
	/// @protectedsection

	/// Linked list of Clocks
	VirtualList clocks;

	/// @publicsection

	/// Reset all the registered clocks.
	static void reset();

	/// Register a new clock
	/// @param clock: Clock to register
	static void register(Clock clock);

	/// Unregister clock.
	/// @param clock: Clock to unregister
	static void unregister(Clock clock);

	/// Update the clocks.
	/// @param elapsedMilliseconds: Milliseconds that passed since the previous call to this method
	static void update(uint32 elapsedMilliseconds);
}

#endif
