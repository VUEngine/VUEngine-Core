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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class TimerManager
///
/// Inherits from Object
///
/// Manages the platform's clock.
singleton class TimerManager : ListenerObject
{
	/// Interrupt handler for timer's interrupts
	static void interruptHandler();

	/// Configure the timer with the provided arguments.
	/// @param timerResolution: Timer's resolution (__TIMER_100US or __TIMER_20US)
	/// @param targetTimePerInterrupt: Target elapsed time between timer interrupts
	/// @param targetTimePerInterrupttUnits: Timer interrupt's target time units
	static void configure(uint16 timerResolution, uint16 targetTimePerInterrupt, uint16 targetTimePerInterrupttUnits);

	/// Apply the settings to the hardware timer.
	/// @param enable: If true, the hardware timer is enabled
	static void applySettings(bool enable);

	/// Enable the timer.
	static void enable();

	/// Disable the timer.
	static void disable();

	/// Reset timer's counter.
	static void resetTimerCounter();

	/// Set the timer's resolution.
	/// @param resolution: __TIMER_20US or __TIMER_100US
	static void setResolution(uint16 resolution);

	/// Retrieve the timer's resolution.
	/// @return Timer's resolution
	static uint16 getResolution();

	/// Retrieve the timer's resolution in microseconds.
	/// @return Timer's resolution in microseconds
	static uint16 getResolutionInUS();

	/// Set the target time between interrupt calls.
	/// @param targetTimePerInterrupt: Target time between interrupt calls
	static void setTargetTimePerInterrupt(uint16 targetTimePerInterrupt);

	/// Retrieve the target time between interrupt calls.
	/// @return Target time between interrupt calls
	static uint16 getTargetTimePerInterrupt();

	/// Retrieve the target time in milliseconds between interrupt calls.
	/// @return Target time in milliseconds between interrupt calls
	static float getTargetTimePerInterruptInMS();

	/// Retrieve the target time in microseconds between interrupt calls.
	/// @return Target time in microseconds between interrupt calls
	static uint32 getTargetTimePerInterruptInUS();
	
	/// Set the target time units between interrupt calls.
	/// @param targetTimePerInterrupttUnits: Target time units between interrupt calls
	static void setTargetTimePerInterruptUnits(uint16 targetTimePerInterrupttUnits);

	/// Retrieve the target time units between interrupt calls.
	/// @return Target time units between interrupt calls
	static uint16 getTargetTimePerInterruptUnits();

	/// Retrieve the configured timer counter.
	/// @return Configured timer counter
	static uint16 getTimerCounter();
	
	/// Retrieve the current timer counter.
	/// @return Current timer counter
	static uint16 getCurrentTimerCounter();

	/// Retrieve the minimum timer per interrupt step.
	/// @return Minimum timer per interrupt step
	static uint16 getMinimumTimePerInterruptStep();

	/// Retrieve elapsed milliseconds since the last call to reset.
	/// @return Elapsed milliseconds since the last call to reset
	static uint32 getElapsedMilliseconds();

	/// Retrieve elapsed milliseconds since the start of the program.
	/// @return Elapsed milliseconds since the start of the program
	static uint32 getTotalElapsedMilliseconds();

	/// Halt the program by the provided time.
	/// @param milliseconds: Time to halt the program
	static void wait(uint32 milliseconds);

	/// Call a method on the provided scope a numer of time during a lapse of time.
	/// @param callTimes: Number of calls to produce during the total duration
	/// @param duration: Time that must take the callTimes
	/// @param object: Called method's scope
 	/// @param method: Method to call
	static void repeatMethodCall(uint32 callTimes, uint32 duration, ListenerObject object, void (*method)(ListenerObject, uint32));

	/// Print the manager's configuration.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);

	/// Print the stats related to the interrupts.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void printInterruptStats(int x, int y);

	/// Reset the manager's state.
	static void reset();

	/// Call when the next frame starts.
	/// @param elapsedMicroseconds: Elapsed microseconds between calls
	static void frameStarted(uint32 elapsedMicroseconds);

	/// Compute the factor between the currently configured timer's resolution and 
	/// a target timer resolution in US.
	/// @param targetTimerResolutionUS: Target timer resolution in US
	/// @param targetUSPerTick: Target US per timer's tick
	/// @return Factor between the currently resolution and the target
	static fix7_9_ext computeTimerResolutionFactor(uint32 targetTimerResolutionUS, uint32 targetUSPerTick);
}

#endif
