/**
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
//											CLASS'S DECLARATION
//=========================================================================================================

class Stopwatch;

/// @ingroup base
singleton class StopwatchManager : Object
{
	VirtualList stopwatchs;

	/// @publicsection
	static StopwatchManager getInstance();
	void register(Stopwatch clock);
	void reset();
	void unregister(Stopwatch clock);
	void update();
}


#endif
