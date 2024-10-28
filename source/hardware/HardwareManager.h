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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

// Hardware register mnemonics
#define	__CCR				0x00	// Communication Control Register	(0x0200 0000)
#define	__CCSR				0x04	// COMCNT Control Register			(0x0200 0004)
#define	__CDTR				0x08	// Transmitted Data Register		(0x0200 0008)
#define	__CDRR				0x0C	// Received Data Register			(0x0200 000C)
#define	__SDLR				0x10	// Serial Data Low Register			(0x0200 0010)
#define	__SDHR				0x14	// Serial Data High Register		(0x0200 0014)
#define	__TLR				0x18	// Timer Low Register				(0x0200 0018)
#define	__THR				0x1C	// Timer High Register				(0x0200 001C)
#define	__TCR				0x20	// Timer Control Register			(0x0200 0020)
#define	__WCR				0x24	// Wait-state Control Register		(0x0200 0024)
#define	__SCR				0x28	// Serial Control Register			(0x0200 0028)

// Cache management
#define CACHE_ENABLE		asm("mov 2,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_DISABLE		asm("ldsr r0,sr24")
#define CACHE_CLEAR			asm("mov 1,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_RESET			CACHE_DISABLE; CACHE_CLEAR; CACHE_ENABLE


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

extern uint8* const _hardwareRegisters;
extern bool _enabledInterrupts __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern int16 _suspendInterruptRequest __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class HardwareManager
///
/// Inherits from Object
///
/// Centralizes the management of the hardware.
/// @ingroup hardware
static class HardwareManager : Object
{
	/// @publicsection

	/// Initialize hardware registries.
	static void initialize();

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

	/// Retrieve the Stack Pointer's value.
	/// @return Stack pointer
	static inline int32 getStackPointer();

	/// Retrieve the Link Pointer's value.
	/// @return Link pointer
	static inline int32 getLinkPointer();

	/// Retrieve the PSW
	/// @return PSW
	static inline int32 getPSW();

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

//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::halt()
{
	// Make sure that I don't halt forever
	HardwareManager::enableInterrupts();

	asm("halt"::);
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::setInterruptLevel(uint8 level)
{
	asm
	(
		"stsr	sr5, r6			\n\t"	\
		"movhi	0xFFF1, r0, r7	\n\t"	\
		"movea	0xFFFF, r7, r7	\n\t"	\
		"and	r6, r7			\n\t"	\
		"mov	%0,r6			\n\t"	\
		"andi	0x000F, r6, r6	\n\t"	\
		"shl	0x10, r6		\n\t"	\
		"or		r7, r6			\n\t"	\
		"ldsr	r6, sr5			\n\t"
		: // Output
		: "r" (level) // Input
		: "r6", "r7" // Clobber
	);
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::enableInterrupts()
{
	if(!_enabledInterrupts || 0 != _suspendInterruptRequest)
	{
		_enabledInterrupts = true;
		_suspendInterruptRequest = 0;

		asm("cli");
		HardwareManager::setInterruptLevel(0);
	}
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::disableInterrupts()
{
	if(_enabledInterrupts)
	{
		_enabledInterrupts = false;
		_suspendInterruptRequest = 0;

		asm("sei");
		HardwareManager::setInterruptLevel(5);
	}
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::resumeInterrupts()
{
	if(_enabledInterrupts)
	{
		if(0 >= --_suspendInterruptRequest)
		{
			_suspendInterruptRequest = 0;
			asm("cli");
			HardwareManager::setInterruptLevel(0);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::suspendInterrupts()
{
	if(_enabledInterrupts)
	{
		_suspendInterruptRequest++;
		asm("sei");
		HardwareManager::setInterruptLevel(5);
	}
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::enableMultiplexedInterrupts()
{
	uint32 psw;

	asm
	(
		"stsr	psw, %0"
		: "=r" (psw) // Output
	);

	psw &= 0xFFF0BFFF;

	asm
	(
		"ldsr	%0, psw	\n\t"	\
		"cli	\n\t"
		: // Output
		: "r" (psw) // Input
		: // Clobber
	);
}
//---------------------------------------------------------------------------------------------------------
static inline void HardwareManager::disableMultiplexedInterrupts()
{
	asm
	(
		"sei"
		: // Output
		: // Input
		: // Clobber
	);
}
//---------------------------------------------------------------------------------------------------------
static inline int32 HardwareManager::getStackPointer()
{
	int32 sp;

	asm
	(
		"mov	sp, %0"
		: "=r" (sp) // Output
	);

	return sp;
}
//---------------------------------------------------------------------------------------------------------
static inline int32 HardwareManager::getLinkPointer()
{
	int32 lp;

	asm
	(
		"mov	lp, %0"
		: "=r" (lp) // Output
	);

	return lp;
}
//---------------------------------------------------------------------------------------------------------
static inline int32 HardwareManager::getPSW()
{
	int32 psw;

	asm
	(
		"stsr	psw, %0"
		: "=r" (psw) // Output
	);

	return psw;
}
//---------------------------------------------------------------------------------------------------------

#endif
