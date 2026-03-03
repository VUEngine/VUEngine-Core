/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef HARDWARE_MANAGER_H_
#define HARDWARE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Platform.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern uint8* const _hardwareRegisters;
extern bool _enabledInterrupts __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern int16 _suspendInterruptRequest __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Hardware
///
/// Inherits from Object
///
/// Centralizes the management of the hardware.
static class Hardware : Object
{
	/// @publicsection

	/// Initialize the basic hardware registries.
	static void initialize();

	/// Reset the hardware managers.
	static void reset();

	/// Halt the CPU.
	static inline void halt();

	/// Set the interrupt level.
	/// @param level: Interrupt level
	static inline void setInterruptLevel(uint8 level);

	/// Enable interrupts.
	static inline void enableInterrupts();

	/// Disable interrupts.
	static inline void disableInterrupts();

	/// Resume interrupts.
	static inline void resumeInterrupts();

	/// Suspend interrupts.
	static inline void suspendInterrupts();

	/// Enable multiplexed interrupts.
	static inline void enableMultiplexedInterrupts();

	/// Disable multiplexed interrupts.
	static inline void disableMultiplexedInterrupts();

	/// Check the status of the interrupts.
	/// @return True if interrupts are not disabled nor suspended
	static inline bool areInterruptsSuspended();

	/// Print the status of the stack.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param resumed: If true, print only minimum information
	static void printStackStatus(int32 x, int32 y, bool resumed);

	/// Print the status of the hardware registries.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::halt()
{
	// Make sure that I don't halt forever
	Hardware::enableInterrupts();

	__CPU_HALT;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::setInterruptLevel(uint8 level)
{
	__CPU_SET_INTERRUPT_LEVEL(level);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::enableInterrupts()
{
	if(!_enabledInterrupts || 0 != _suspendInterruptRequest)
	{
		_enabledInterrupts = true;
		_suspendInterruptRequest = 0;

		__CPU_ENABLE_INTERRUPTS;
		Hardware::setInterruptLevel(0);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::disableInterrupts()
{
	if(_enabledInterrupts)
	{
		_enabledInterrupts = false;
		_suspendInterruptRequest = 0;

		__CPU_SUSPEND_INTERRUPTS;
		Hardware::setInterruptLevel(5);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::resumeInterrupts()
{
	if(_enabledInterrupts)
	{
		if(0 >= --_suspendInterruptRequest)
		{
			_suspendInterruptRequest = 0;
			__CPU_ENABLE_INTERRUPTS;
			Hardware::setInterruptLevel(0);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::suspendInterrupts()
{
	if(_enabledInterrupts)
	{
		_suspendInterruptRequest++;
		__CPU_SUSPEND_INTERRUPTS;
		Hardware::setInterruptLevel(5);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::enableMultiplexedInterrupts()
{
	__CPU_MULTIPLEX_INTERRUPTS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Hardware::disableMultiplexedInterrupts()
{
	__CPU_SUSPEND_INTERRUPTS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline bool Hardware::areInterruptsSuspended()
{
	return !_enabledInterrupts || 0 < _suspendInterruptRequest;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
