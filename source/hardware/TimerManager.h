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

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __TIMER_COUNTER_DELTA		0

//use with 20us timer (range = 0 to 1300)
#define __TIME_US(n)				(((n) / TimerManager::getResolutionInUS(TimerManager::getInstance())) - __TIMER_COUNTER_DELTA)
#define __TIME_INVERSE_US(n)		((n + __TIMER_COUNTER_DELTA) * TimerManager::getResolutionInUS(TimerManager::getInstance()))

//use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define __TIME_MS(n)				((((n) * 1000) / TimerManager::getResolutionInUS(TimerManager::getInstance())) - __TIMER_COUNTER_DELTA)
#define __TIME_INVERSE_MS(n)		((n + __TIMER_COUNTER_DELTA) * TimerManager::getResolutionInUS(TimerManager::getInstance()) / 1000)

#define __TIMER_ENB			0x01
#define __TIMER_ZSTAT		0x02
#define __TIMER_ZCLR		0x04
#define __TIMER_INT			0x08
#define __TIMER_20US		0x10
#define __TIMER_100US		0x00

#define __MINIMUM_TIME_PER_INTERRUPT_US			(TimerManager::getResolutionInUS(TimerManager::getInstance()) * 1)
#define __MAXIMUM_TIME_PER_INTERRUPT_US 		10 * 1000
#define __MINIMUM_TIME_PER_INTERRUPT_MS			1
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
	uint32 milliseconds;
	uint32 microseconds;
	uint32 totalMilliseconds;
	uint16 resolution;
	uint16 timePerInterrupt;
	uint16 timePerInterruptUnits;
	uint16 minimumTimePerInterruptUS;
	uint16 minimumTimePerInterruptMS;
	uint16 maximumTimePerInterruptUS;
	uint16 maximumTimePerInterruptMS;
	uint8 tcrValue;

	/// @publicsection
	static TimerManager getInstance();
	static void interruptHandler();
	void reset();
	uint16 getResolution();
	uint16 getResolutionInUS();
	uint16 getTimePerInterrupt();
	float getTimePerInterruptInMS();
	uint32 getTimePerInterruptInUS();
	uint16 getTimerCounter();
	uint16 getTimePerInterruptUnits();
	uint16 getMinimumTimePerInterruptStep();
	void setResolution(uint16 resolution);
	void setTimePerInterrupt(uint16 timePerInterrupt);
	void setTimePerInterruptUnits(uint16 timePerInterruptUnits);
	void enable(bool flag);
	uint32 getMillisecondsElapsed();
	uint32 getTotalMillisecondsElapsed();
	uint32 resetMilliseconds();
	void configureTimerCounter();
	int32 getStat();
	void clearStat();
	void initialize();
	void wait(uint32 milliSeconds);
	void repeatMethodCall(uint32 callTimes, uint32 duration, Object object, void (*method)(Object, uint32));
	void print(int32 x, int32 y);
}


#endif
