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

static u8* const _hardwareRegisters =			(u8*)0x02000000;

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
#define CACHE_DISABLE	asm("ldsr r0,sr24")


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class HardwareManager : Object
{
	static HardwareManager getInstance();
	void clearScreen();
	void disableKeypad();
	void disableRendering();
	void displayOff();
	void displayOn();
	void enableKeypad();
	void enableRendering();
	void initializeTimer();
	void lowerBrightness();
	void print(int x, int y);
	void setInterruptLevel(u8 level);
	void setInterruptVectors();
	void setupColumnTable(ColumnTableDefinition* columnTableDefinition);
	void upBrightness();
	void checkStackStatus();
	void printStackStatus(int x, int y, bool resumed);
}

/**
 * Enable interrupts
 *
 * @memberof					HardwareManager
 * @public
 *
 */
inline void HardwareManager_enableInterrupts()
{
	asm("cli");
}

/**
 * Disable interrupts
 *
 * @memberof					HardwareManager
 * @public
 *
 */
inline void HardwareManager_disableInterrupts()
{
	asm("sei");
}

/**
 * Enable multiplexed interrupts
 *
 * @memberof					VIPManager
 * @public
 *
 */
inline void HardwareManager_enableMultiplexedInterrupts()
{
	u32 psw;

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
 *
 * @memberof					VIPManager
 * @public
 *
 */
inline void HardwareManager_disableMultiplexedInterrupts()
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
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 *
 */
inline int HardwareManager_getStackPointer()
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
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 *
 */
inline int HardwareManager_getLinkPointer()
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
 *
 * @memberof		HardwareManager
 * @public
 *
 * @return		 	PSW
 */
inline int HardwareManager_getPSW()
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
