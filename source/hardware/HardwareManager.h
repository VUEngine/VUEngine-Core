/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef HARDWARE_MANAGER_H_
#define HARDWARE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

static uint8* const _hardwareRegisters =			(uint8*)0x02000000;

// hardware register mnemonics
#define	__CCR		0x00	// Communication Control Register	(0x0200 0000)
#define	__CCSR		0x04	// COMCNT Control Register			(0x0200 0004)
#define	__CDTR		0x08	// Transmitted Data Register		(0x0200 0008)
#define	__CDRR		0x0C	// Received Data Register			(0x0200 000C)
#define	__SDLR		0x10	// Serial Data Low Register			(0x0200 0010)
#define	__SDHR		0x14	// Serial Data High Register		(0x0200 0014)
#define	__TLR		0x18	// Timer Low Register				(0x0200 0018)
#define	__THR		0x1C	// Timer High Register				(0x0200 001C)
#define	__TCR		0x20	// Timer Control Register			(0x0200 0020)
#define	__WCR		0x24	// Wait-state Control Register		(0x0200 0024)
#define	__SCR		0x28	// Serial Control Register			(0x0200 0028)

// cache management
#define CACHE_ENABLE	asm("mov 2,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_DISABLE	asm("ldsr r0,sr24")
#define CACHE_CLEAR		asm("mov 1,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_RESET		CACHE_DISABLE; CACHE_CLEAR; CACHE_ENABLE

extern bool _enabledInterrupts __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern int16 _suspendInterruptRequest __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

typedef struct ColumnTableSpec ColumnTableSpec;

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class HardwareManager : Object
{
	// HW registry
	uint8* hwRegisters;

	/// @publicsection
	static HardwareManager getInstance();
	static inline void setInterruptLevel(uint8 level);
	static inline void enableInterrupts();
    static inline void disableInterrupts();
	static inline void resumeInterrupts();
    static inline void suspendInterrupts();
    static inline void enableMultiplexedInterrupts();
    static inline void disableMultiplexedInterrupts();
    static inline int32 getStackPointer();
    static inline int32 getLinkPointer();
    static inline int32 getPSW();
	static void checkMemoryMap();
	static void printStackStatus(int32 x, int32 y, bool resumed);
	void clearScreen();
	void disableKeypad();
	void disableRendering();
	void displayOff();
	void displayOn();
	void enableKeypad();
	void enableRendering();
	void setupTimer(uint16 timerResolution, uint16 timePerInterrupt, uint16 timePerInterruptUnits);
	void lowerBrightness();
	void print(int32 x, int32 y);
	void setInterruptVectors();
	void setupColumnTable(ColumnTableSpec* columnTableSpec);
	void upBrightness();
	bool isDrawingAllowed();

	static inline void halt();
}

static inline void HardwareManager::halt()
{
	// Make sure that I don't halt forever
	HardwareManager::enableInterrupts();

    static const long code = 0x181F6800L;
    ((void(*)())&code)();
}

/**
 * Set the interrupt level
 *
 * @param level	 	Interrupt level
 */
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

/**
 * Enable interrupts
 */
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

/**
 * Disable interrupts
 */
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

/**
 * Enable interrupts
 */
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

/**
 * Disable interrupts
 */
static inline void HardwareManager::suspendInterrupts()
{
	if(_enabledInterrupts)
	{
		_suspendInterruptRequest++;
		asm("sei");
		HardwareManager::setInterruptLevel(5);
	}
}

/**
 * Enable multiplexed interrupts
 */
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

/**
 * Disable multiplexed interrupts
 */
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

/**
 * Retrieve the Stack Pointer's value
 */
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

/**
 * Retrieve the Link Pointer's value
 */
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

/**
 * Retrieve PSW
 * @return		 	PSW
 */
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

#endif
