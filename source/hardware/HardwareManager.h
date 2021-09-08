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

#ifndef HARDWARE_MANAGER_H_
#define HARDWARE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VIPManager.h>


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


extern bool _enabledInterrupts;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class HardwareManager : Object
{
	// Timer manager
	TimerManager timerManager;
	// VPU manager
	VIPManager vipManager;
	// VPU manager
	KeypadManager keypadManager;
	// HW registry
	uint8* hwRegisters;

	/// @publicsection
	static HardwareManager getInstance();
	static inline void enableInterrupts();
    static inline void disableInterrupts();
	static inline void resumeInterrupts();
    static inline void suspendInterrupts();
    static inline void enableMultiplexedInterrupts();
    static inline void disableMultiplexedInterrupts();
    static inline int getStackPointer();
    static inline int getLinkPointer();
    static inline int getPSW();
	static void checkMemoryMap();
	static void printStackStatus(int x, int y, bool resumed);
	void clearScreen();
	void disableKeypad();
	void disableRendering();
	void displayOff();
	void displayOn();
	void enableKeypad();
	void enableRendering();
	void setupTimer(uint16 timerResolution, uint16 timePerInterrupt, uint16 timePerInterruptUnits);
	void lowerBrightness();
	void print(int x, int y);
	void setInterruptLevel(uint8 level);
	void setInterruptVectors();
	void setupColumnTable(ColumnTableSpec* columnTableSpec);
	void upBrightness();
	bool isDrawingAllowed();

	static inline void halt();
}

static inline void HardwareManager::halt()
{
    static const long code = 0x181F6800L;
    ((void(*)())&code)();
}

/**
 * Enable interrupts
 */
static inline void HardwareManager::enableInterrupts()
{
	_enabledInterrupts = true;

	asm("cli");
}

/**
 * Disable interrupts
 */
static inline void HardwareManager::disableInterrupts()
{
	_enabledInterrupts = false;

	asm("sei");
}

/**
 * Enable interrupts
 */
static inline void HardwareManager::resumeInterrupts()
{
	if(_enabledInterrupts)
	{
		asm("cli");
	}
}

/**
 * Disable interrupts
 */
static inline void HardwareManager::suspendInterrupts()
{
	asm("sei");
}

/**
 * Enable multiplexed interrupts
 */
static inline void HardwareManager::enableMultiplexedInterrupts()
{
	uint32 psw;

	asm("			\n\
		stsr psw,%0	\n\
		"
		: "=r" (psw) // Output
	);

	psw &= 0xFFF0BFFF;

	asm(" 			\n\
		ldsr %0,psw	\n\
		cli			\n\
		"
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
	asm(" 			\n\
		sei			\n\
		"
		: // Output
		: // Input
		: // Clobber
	);
}

/**
 * Retrieve the Stack Pointer's value
 */
static inline int HardwareManager::getStackPointer()
{
	int sp;

	asm(" 			\n\
		mov sp,%0	\n\
		"
	: "=r" (sp) // Output
	);

	return sp;
}

/**
 * Retrieve the Link Pointer's value
 */
static inline int HardwareManager::getLinkPointer()
{
	int lp;

	asm(" 			\n\
		mov lp,%0	\n\
		"
	: "=r" (lp) // Output
	);

	return lp;
}

/**
 * Retrieve PSW
 * @return		 	PSW
 */
static inline int HardwareManager::getPSW()
{
	int psw;

	asm("			\n\
		stsr psw,%0	\n\
		"
	: "=r" (psw) // Output
	);
	return psw;
}

#endif
