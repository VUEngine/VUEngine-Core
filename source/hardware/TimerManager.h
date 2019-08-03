/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

//use with 20us timer (range = 0 to 1300)
#define __TIME_US(n)		(((n)/20)-1)

//use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define __TIME_MS(n)		(((n)*10)-1)

#define __TIMER_ENB			0x01
#define __TIMER_ZSTAT		0x02
#define __TIMER_ZCLR		0x04
#define __TIMER_INT			0x08
#define __TIMER_20US		0x10
#define __TIMER_100US		0x00

#define __MINIMUM_TIME_PER_INTERRUPT_US			25
#define __MINIMUM_TIME_PER_INTERRUPT_MS			1
#define __MAXIMUM_TIME_PER_INTERRUPT_US 		10 * 1000
#define __MAXIMUM_TIME_PER_INTERRUPT_MS 		49

enum TimerResolutionScales
{
	kUS = 0,			// Microseconds
	kMS,				// Milliseconds
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class TimerManager : Object
{
	u32 milliseconds;
	u32 microseconds;
	u32 totalMilliseconds;
	u16 resolution;
	u16 timePerInterrupt;
	u16 timePerInterruptUnits;
	u8 tcrValue;

	/// @publicsection
	static TimerManager getInstance();
	static void interruptHandler();
	void reset();
	u16 getResolution();
	u16 getResolutionInUS();
	u16 getTimePerInterrupt();
	u32 getTimePerInterruptInMS();
	u32 getTimePerInterruptInUS();
	u16 getTimerCounter();
	u16 getTimePerInterruptUnits();
	u16 getMinimumTimePerInterruptStep();
	void setResolution(u16 resolution);
	void setTimePerInterrupt(u16 timePerInterrupt);
	void setTimePerInterruptUnits(u16 timePerInterruptUnits);
	void enable(bool flag);
	u32 getMillisecondsElapsed();
	u32 getTotalMillisecondsElapsed();
	u32 resetMilliseconds();
	int getStat();
	void clearStat();
	void initialize();
	void wait(u32 milliSeconds);
	void repeatMethodCall(u32 callTimes, u32 duration, Object object, void (*method)(Object, u32));
	void print(int x, int y);
}


#endif
