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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class FrameRate
///
/// Inherits from ListenerObject
///
/// Keeps track of the program's frame rate.
singleton! class FrameRate : ListenerObject
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

	/// Register an object that will listen for events.
	/// @param listener: ListenerObject that listen for the event
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to listen for
	static void registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listening to a give code with the provided callback.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	static void unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Reset the state of the manager.
	static void reset();

	/// Set the target frames per second.
	/// @param targetFPS: Target frames per second
	static void setTarget(uint8 targetFPS);

	/// Update the elapsed frames during the current second.
	static void update();

	/// The next game frame cycle has started.
	/// @param gameCycleEnded: Flag that indicates if the previous game frame was completed before the current second has elapsed
	/// @param printFPS: True to print the FPS during the previous second
	static void gameFrameStarted(bool gameCycleEnded, bool printFPS);

	/// Print the frames per second statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);
}

#endif
