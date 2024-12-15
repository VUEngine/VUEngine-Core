/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class ListenerObject;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __TIMER_COUNTER_DELTA						1

// Use with 20us timer (range = 0 to 1300)
#define __TIME_US(n)								(((n) / TimerManager::getResolutionInUS(TimerManager::getInstance())) - __TIMER_COUNTER_DELTA)
#define __TIME_INVERSE_US(n)						((n + __TIMER_COUNTER_DELTA) * TimerManager::getResolutionInUS(TimerManager::getInstance()))

// Use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define __TIME_MS(n)								((((n) * __MICROSECONDS_PER_MILLISECOND) / TimerManager::getResolutionInUS(TimerManager::getInstance())) - __TIMER_COUNTER_DELTA)
#define __TIME_INVERSE_MS(n)						((n + __TIMER_COUNTER_DELTA) * TimerManager::getResolutionInUS(TimerManager::getInstance()) / 1000)

#define __TIMER_ENB									0x01
#define __TIMER_ZSTAT								0x02
#define __TIMER_ZCLR								0x04
#define __TIMER_INT									0x08
#define __TIMER_20US								0x10
#define __TIMER_100US								0x00

#define __MINIMUM_TIME_PER_INTERRUPT_US_STEP		(TimerManager::getResolutionInUS(TimerManager::getInstance()))
#define __MINIMUM_TIME_PER_INTERRUPT_MS_STEP		1
#define __MINIMUM_TIME_PER_INTERRUPT_US				(TimerManager::getResolutionInUS(TimerManager::getInstance()) + TimerManager::getResolutionInUS(TimerManager::getInstance()) * __TIMER_COUNTER_DELTA)
#define __MAXIMUM_TIME_PER_INTERRUPT_US 			(1.3f * 1000)
#define __MINIMUM_TIME_PER_INTERRUPT_MS				1
#define __MAXIMUM_TIME_PER_INTERRUPT_MS 			49


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

enum TimerResolutionScales
{
	kUS = 0,			// Microseconds
	kMS,				// Milliseconds
};


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class TimerManager
///
/// Inherits from Object
///
/// Manages rumble effects.
singleton class TimerManager : Object
{
	/// Elapsed milliseconds since the last call to reset
	uint32 elapsedMilliseconds;

	/// Elapsed microseconds
	uint32 elapsedMicroseconds;

	/// Elapsed milliseconds since the start of the program
	uint32 totalElapsedMilliseconds;

	/// Timer's resolution
	uint16 resolution;

	/// Interrupts during the last secoond
	uint16 interruptsPerSecond;

	/// Interrupts during the last game frame
	uint16 interruptsPerGameFrame;

	/// Elapsed microseconds per interrupt
	uint32 elapsedMicrosecondsPerInterrupt;

	/// Target elapsed time per interrupt
	uint16 targetTimePerInterrupt;

	/// Units of the target time per interrupt
	uint16 targetTimePerInterrupttUnits;

	/// Last written value to the TCR registry
	uint8 tcrValue;

	/// @publicsection
	
	/// Method to retrieve the singleton instance
	/// @return TimerManager singleton
	static TimerManager getInstance();

	/// Interrupt handler for timer's interrupts
	static void interruptHandler();

	/// Reset the manager's state.
	void reset();

	/// Reset timer's counter.
	void resetTimerCounter();

	/// Configure the timer with the provided arguments.
	/// @param timerResolution: Timer's resolution (__TIMER_100US or __TIMER_20US)
	/// @param targetTimePerInterrupt: Target elapsed time between timer interrupts
	/// @param targetTimePerInterrupttUnits: Timer interrupt's target time units
	void configure(uint16 timerResolution, uint16 targetTimePerInterrupt, uint16 targetTimePerInterrupttUnits);

	/// Apply the settings to the hardware timer.
	/// @param enable: If true, the hardware timer is enabled
	void applySettings(bool enable);

	/// Enable the timer.
	void enable();

	/// Disable the timer.
	void disable();

	/// Set the timer's resolution.
	/// @param resolution: __TIMER_20US or __TIMER_100US
	void setResolution(uint16 resolution);

	/// Retrieve the timer's resolution.
	/// @return Timer's resolution
	uint16 getResolution();

	/// Retrieve the timer's resolution in microseconds.
	/// @return Timer's resolution in microseconds
	uint16 getResolutionInUS();

	/// Set the target time between interrupt calls.
	/// @param targetTimePerInterrupt: Target time between interrupt calls
	void setTargetTimePerInterrupt(uint16 targetTimePerInterrupt);

	/// Retrieve the target time between interrupt calls.
	/// @return Target time between interrupt calls
	uint16 getTargetTimePerInterrupt();

	/// Retrieve the target time in milliseconds between interrupt calls.
	/// @return Target time in milliseconds between interrupt calls
	float getTargetTimePerInterruptInMS();

	/// Retrieve the target time in microseconds between interrupt calls.
	/// @return Target time in microseconds between interrupt calls
	uint32 getTargetTimePerInterruptInUS();
	
	/// Set the target time units between interrupt calls.
	/// @param targetTimePerInterrupttUnits: Target time units between interrupt calls
	void setTargetTimePerInterruptUnits(uint16 targetTimePerInterrupttUnits);

	/// Retrieve the target time units between interrupt calls.
	/// @return Target time units between interrupt calls
	uint16 getTargetTimePerInterruptUnits();

	/// Retrieve the configured timer counter.
	/// @return Configured timer counter
	uint16 getTimerCounter();
	
	/// Retrieve the current timer counter.
	/// @return Current timer counter
	uint16 getCurrentTimerCounter();

	/// Retrieve the minimum timer per interrupt step.
	/// @return Minimum timer per interrupt step
	uint16 getMinimumTimePerInterruptStep();

	/// Retrieve elapsed milliseconds since the last call to reset.
	/// @return Elapsed milliseconds since the last call to reset
	uint32 getElapsedMilliseconds();

	/// Retrieve elapsed milliseconds since the start of the program.
	/// @return Elapsed milliseconds since the start of the program
	uint32 getTotalElapsedMilliseconds();

	/// Halt the program by the provided time.
	/// @param milliseconds: Time to halt the program
	void wait(uint32 milliseconds);

	/// Call a method on the provided scope a numer of time during a lapse of time.
	/// @param callTimes: Number of calls to produce during the total duration
	/// @param duration: Time that must take the callTimes
	/// @param object: Called method's scope
 	/// @param method: Method to call
	void repeatMethodCall(uint32 callTimes, uint32 duration, ListenerObject object, void (*method)(ListenerObject, uint32));

	/// Call when the next frame starts.
	/// @param elapsedMicroseconds: Elapsed microseconds between calls
	void frameStarted(uint32 elapsedMicroseconds);

	/// Call when the next second starts.
	void nextSecondStarted();

	/// Print the manager's configuration.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}


#endif
