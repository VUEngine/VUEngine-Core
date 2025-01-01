/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CLOCK_H_
#define CLOCK_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

enum ClockPrintPrecision
{
	kTimePrecision0 = 0,
	kTimePrecision1,
	kTimePrecision2,
	kTimePrecision3
};


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Clock
///
/// Inherits from ListenerObject
///
/// Implements simple clock that can keep track of time and print itself.
class Clock : ListenerObject
{
	/// @protectedsection

	/// Elapsed time in milliseconds
	uint32 milliseconds;

	/// Previous elapsed second
	uint32 previousSecond;

	/// Previous elapsed minute
	uint32 previousMinute;

	/// Flag to signal if the clock is paused or not
	bool paused;

	/// @publicsection

	/// Print time in MM::SS:XX format
	/// @param milliseconds: Total time to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param font: Pointer to font's name to use
	/// @param precision: Precision of the second decimals
	static void printTime(uint32 milliseconds, int32 x, int32 y, const char* font, uint32 precision);

	/// Print the total deciseconds in the elapsed time provided.
	/// @param milliSeconds: Total time to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param font: Pointer to font's name to use
	static void printDeciseconds(uint32 milliSeconds, int32 x, int32 y, const char* font);

	/// Print the total centiseconds in the elapsed time provided.
	/// @param milliSeconds: Total time to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param font: Pointer to font's name to use
	static void printCentiseconds(uint32 milliSeconds, int32 x, int32 y, const char* font);

	/// Print the total milliseconds in the elapsed time provided.
	/// @param milliSeconds: Total time to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param font: Pointer to font's name to use
	static void printMilliseconds(uint32 milliSeconds, int32 x, int32 y, const char* font);

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Class' destructor
	void destructor();

	/// Start the clock.
	void start();

	/// Stop the clock.
	void stop();

	/// Pause/unpause the clock
	/// @param pause: Flag to set the paused state of the clock
	void pause(bool pause);

	/// Reset the clock's elapsed time
	void reset();

	/// Update the clock's elapsed time
	/// @param elapsedMilliseconds: Milliseconds that have passed since the previous call to the update method
	void update(uint32 elapsedMilliseconds);

	/// Retrieve the clock's paused state
	/// @return True if the clock is paused
	bool isPaused();

	/// Retrieve the elapsed milliseconds.
	/// @return Elapsed milliseconds
	uint32 getMilliseconds();

	/// Retrieve the elapsed minutes.
	/// @return Elapsed minutes
	uint32 getMinutes();

	/// Retrieve the elapsed seconds.
	/// @return Elapsed seconds
	uint32 getSeconds();

	/// Print the clock's time in MM::SS:XX format
	/// @param col: Screen x coordinate where to print
	/// @param row: Screen y coordinate where to print
	/// @param font: Pointer to font's name to use
	void print(int32 col, int32 row, const char* font);
}

#endif
