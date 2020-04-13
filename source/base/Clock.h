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

/// @ingroup base
class Clock : Object
{
	// time elapsed
	u32 milliSeconds;
	// register
	u32 previousSecond;
	// register
	u32 previousMinute;
	// flag to pause the clock
	bool paused;

	/// @publicsection
	void constructor();
	u32 getElapsedTime();
	u32 getMilliSeconds();
	u32 getMinutes();
	u32 getSeconds();
	u32 getTime();
	int getTimeInCurrentSecond();
	bool isPaused();
	void pause(bool paused);
	void print(int col, int row, const char* font);
	void reset();
	void setTime(int hours, int minutes, int seconds);
	void setTimeInMilliSeconds(u32 milliSeconds);
	void setTimeInSeconds(float totalSeconds);
	void start();
	void stop();
	virtual void update(u32 millisecondsElapsed);
}


#endif
