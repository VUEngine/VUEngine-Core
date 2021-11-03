/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef FRAMERATE_H_
#define FRAMERATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Stopwatch.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
singleton class FrameRate : Object
{
	Stopwatch stopwatch;
	// elapsed time in current 50hz cycle
	float gameFrameTotalTime;
	// Frames per second
	uint16 fps;
	uint16 unevenFps;

	/// @publicsection
	static FrameRate getInstance();
	uint16 getFps();
	void update();
	void print(int32 col, int32 row);
	void reset();
}


#endif
