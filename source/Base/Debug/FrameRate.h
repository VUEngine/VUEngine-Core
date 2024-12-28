/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef FRAMERATE_H_
#define FRAMERATE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class FrameRate
///
/// Inherits from ListenerObject
///
/// Keeps track of the program's frame rate.
singleton class FrameRate : ListenerObject
{
	/// @protectedsection

	/// Accumulated frames per second
	uint32 totalFPS;

	/// Accumulated uneven frames per second
	uint32 totalUnevenFPS;
	
	/// Accumualted elapssed seconds
	uint16 seconds;

	/// Current frames per second
	uint16 FPS;

	/// Total number of game frame starts
	uint16 gameFrameStarts;

	/// Uneven frames during the current second
	uint16 unevenFPS;

	/// The target frames per second
	uint8 targetFPS;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return FrameRate singleton
	static FrameRate getInstance();

	/// Reset the state of the manager.
	void reset();

	/// Set the target frames per second.
	/// @param targetFPS: Target frames per second
	void setTarget(uint8 targetFPS);

	/// Update the elapsed frames during the current second.
	void update();

	/// The next game frame cycle has started.
	/// @param gameCycleEnded: Flag that indicates if the previous game frame was completed before the current second has elapsed
	/// @param printFPS: True to print the FPS during the previous second
	void gameFrameStarted(bool gameCycleEnded, bool printFPS);

	/// Print the frames per second statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}


#endif
