/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Stopwatch
///
/// Inherits from ListenerObject
///
/// Implements a stopwatch to keep track of time passage.
/// @ingroup base
class Stopwatch : ListenerObject
{
	/// @protectedsection

	/// Elapsed time in milliseconds
	uint32 milliSeconds;

	/// Interrupts counter
	uint32 interrupts;

	/// Timer counter's configuration value
	uint32 timerCounter;

	/// Last registered timer counter's configuration value
	uint32 previousTimerCounter;

	/// Ratio between elapsed time per interrupt and timer counter's value
	float timeProportion;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Class' destructor
	void destructor();

	/// Reset the state of the stopwatch.
	void reset();

	/// Update the state of the stopwatch.
	void update();

	/// Register a new lap.
	/// @return Elapsed time during the last lap
	float lap();
}


#endif
