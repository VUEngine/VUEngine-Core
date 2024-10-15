/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class VirtualList;
class Clock;


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class ClockManager
///
/// Inherits from Object
///
/// Manages the instances of Clock.
/// @ingroup base
singleton class ClockManager : Object
{
	// Linked list of Clocks
	VirtualList clocks;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return ClockManager singleton
	static ClockManager getInstance();

	/// Register a new clock
	/// @param clock: Clock to register
	void register(Clock clock);

	/// Unregister clock.
	void unregister(Clock clock);

	/// Update the clocks.
	/// @param elapsedMilliseconds: Milliseconds that passed since the previous call to this method
	void update(uint32 elapsedMilliseconds);

	/// Reset all the registered clocks.
	void reset();
}


#endif
