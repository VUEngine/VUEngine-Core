/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STOPWATCH_MANAGER_H_
#define STOPWATCH_MANAGER_H_


//=========================================================================================================
//												INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Stopwatch;


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class StopwatchManager
///
/// Inherits from Object
///
/// Manages the instances of Stopwatch.
/// @ingroup base
singleton class StopwatchManager : Object
{
	// Linked list of Stopwatches
	VirtualList stopwatchs;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return FrameRate singleton
	static StopwatchManager getInstance();

	/// Register a new stopwatch
	/// @param clock: Stopwatch to register
	void register(Stopwatch clock);

	/// Unregister stopwatch.
	/// @param clock: Stopwatch to unregister
	void unregister(Stopwatch clock);

	/// Update the stopwatches.
	void update();

	/// Reset all the registered stopwatches.
	void reset();
}


#endif
