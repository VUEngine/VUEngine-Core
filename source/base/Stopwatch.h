/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
class Stopwatch : Object
{
	// time elapsed
	uint32 milliSeconds;
	// register
	uint32 interrupts;
	uint32 timerCounter;
	uint32 previousTimerCounter;
	float timeProportion;

	/// @publicsection
	void constructor();
	float lap();
	void reset();
	void update();
}


#endif
