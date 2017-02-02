/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#define CACHE_ENABLE	asm("mov 2,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_DISABLE	asm("ldsr r0,sr24")


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define HardwareManager_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define HardwareManager_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(HardwareManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

HardwareManager HardwareManager_getInstance();

void HardwareManager_destructor(HardwareManager this);
void HardwareManager_setInterruptVectors(HardwareManager this);
void HardwareManager_setInterruptLevel(HardwareManager this, u8 level);
int HardwareManager_getPSW(HardwareManager this);
int HardwareManager_getStackPointer(HardwareManager this);
int HardwareManager_getLPointer(HardwareManager this);
void HardwareManager_initializeTimer(HardwareManager this);
void HardwareManager_clearScreen(HardwareManager this);
void HardwareManager_displayOn(HardwareManager this);
void HardwareManager_displayOff(HardwareManager this);
void HardwareManager_disableRendering(HardwareManager this);
void HardwareManager_enableRendering(HardwareManager this);
void HardwareManager_upBrightness(HardwareManager this);
void HardwareManager_lowerBrightness(HardwareManager this);
void HardwareManager_setupColumnTable(HardwareManager this, ColumnTableDefinition* columnTableDefinition);
void HardwareManager_enableKeypad(HardwareManager this);
void HardwareManager_disableKeypad(HardwareManager this);
void HardwareManager_print(HardwareManager this, int x, int y);
#ifdef __ALERT_STACK_OVERFLOW
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);
#endif


#endif
