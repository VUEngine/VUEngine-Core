/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef HARDWARE_MANAGER_H_
#define HARDWARE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VIP.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VPUManager.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

static u8* const HW_REGS =			(u8*)0x02000000;

// Hardware Register Mnemonics
#define	CCR		0x00	// Communication Control Register	(0x0200 0000)
#define	CCSR	0x04	// COMCNT Control Register			(0x0200 0004)
#define	CDTR	0x08	// Transmitted Data Register		(0x0200 0008)
#define	CDRR	0x0C	// Received Data Register			(0x0200 000C)
#define	SDLR	0x10	// Serial Data Low Register			(0x0200 0010)
#define	SDHR	0x14	// Serial Data High Register		(0x0200 0014)
#define	TLR		0x18	// Timer Low Register				(0x0200 0018)
#define	THR		0x1C	// Timer High Register				(0x0200 001C)
#define	TCR		0x20	// Timer Control Register			(0x0200 0020)
#define	WCR		0x24	// Wait-state Control Register		(0x0200 0024)
#define	SCR		0x28	// Serial Control Register			(0x0200 0028)

// Cache Management
#define CACHE_ENABLE    asm("mov 2,r1 \n  ldsr r1,sr24": /* No Output */: /* No Input */: "r1" /* Reg r1 Used */)
#define CACHE_DISABLE   asm("ldsr r0,sr24")


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define HardwareManager_METHODS																			\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define HardwareManager_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(HardwareManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
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
u16 HardwareManager_readKeypad(HardwareManager this);
void HardwareManager_print(HardwareManager this, int x, int y);
#ifdef __ALERT_STACK_OVERFLOW
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);
#endif


#endif
