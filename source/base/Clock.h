/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CLOCK_H_
#define CLOCK_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

enum ClockPrintPrecision
{
	kTimePrecision0 = 0,
	kTimePrecision1,
	kTimePrecision2,
	kTimePrecision3
};

/// @ingroup base
class Clock : Object
{
	// time elapsed
	uint32 milliSeconds;
	// register
	uint32 previousSecond;
	// register
	uint32 previousMinute;
	// flag to pause the clock
	bool paused;

	static void printTime(uint32 milliseconds, int32 col, int32 row, const char* font, uint32 precision);
	static void printDeciseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);
	static void printCentiseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);
	static void printMilliseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);

	/// @publicsection
	void constructor();
	uint32 getElapsedTime();
	uint32 getMilliSeconds();
	uint32 getMinutes();
	uint32 getSeconds();
	uint32 getTime();
	int32 getTimeInCurrentSecond();
	bool isPaused();
	void pause(bool paused);
	void print(int32 col, int32 row, const char* font);
	void reset();
	void setTime(int32 hours, int32 minutes, int32 seconds);
	void setTimeInMilliSeconds(uint32 milliSeconds);
	void setTimeInSeconds(float totalSeconds);
	void start();
	void stop();
	void update(uint32 millisecondsElapsed);
}


#endif
