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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
singleton class ClockManager : ListenerObject
{
	// registered clocks
	VirtualList clocks;

	/// @publicsection
	static ClockManager getInstance();
	void register(Clock clock);
	void reset();
	void unregister(Clock clock);
	void update(uint32 millisecondsElapsed);
}


#endif
