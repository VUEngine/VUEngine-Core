/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

#define __TIMER_COUNTER_DELTA		2

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

#define __MINIMUM_TIME_PER_INTERRUPT_US_STEP	(TimerManager::getResolutionInUS(TimerManager::getInstance()))
#define __MINIMUM_TIME_PER_INTERRUPT_US			(TimerManager::getResolutionInUS(TimerManager::getInstance()) + TimerManager::getResolutionInUS(TimerManager::getInstance()) * __TIMER_COUNTER_DELTA)
#define __MINIMUM_TIME_PER_INTERRUPT_MS_STEP	1
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

class ListenerObject;

/// @ingroup hardware
singleton class TimerManager : Object
{
	uint32 milliseconds;
	uint32 microseconds;
	uint32 totalMilliseconds;
	uint16 resolution;
	uint16 interruptsPerSecond;
	uint16 interruptsPerGameFrame;
	uint32 microsecondsPerInterrupt;
	uint16 timePerInterrupt;
	uint16 timePerInterruptUnits;
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
	void nextFrameStarted(uint32 elapsedMicroseconds);
	void nextSecondStarted();
	uint32 getMillisecondsElapsed();
	uint32 getTotalMillisecondsElapsed();
	uint32 resetMilliseconds();
	uint16 getCurrentTimerCounter();
	void configureTimerCounter();
	int32 getStat();
	void clearStat();
	void initialize();
	void wait(uint32 milliSeconds);
	void repeatMethodCall(uint32 callTimes, uint32 duration, ListenerObject object, void (*method)(ListenerObject, uint32));
	void print(int32 x, int32 y);
	void print(int32 x, int32 y);
}


#endif
