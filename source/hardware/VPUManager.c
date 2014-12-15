/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VPUManager.h>
#include <HardwareManager.h>

//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------

const static BYTE columnTable[128] =
{
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xe0, 0xbc,
	0xa6, 0x96, 0x8a, 0x82, 0x7a, 0x74, 0x6e, 0x6a,
	0x66, 0x62, 0x60, 0x5c, 0x5a, 0x58, 0x56, 0x54,
	0x52, 0x50, 0x50, 0x4e, 0x4c, 0x4c, 0x4a, 0x4a,
	0x48, 0x48, 0x46, 0x46, 0x46, 0x44, 0x44, 0x44,
	0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3c,
	0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
	0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c
};

/****** VIP Registers ******/
volatile u16* VIP_REGS = (u16*)0x0005F800;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define VPUManager_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// define the VPUManager
__CLASS_DEFINITION(VPUManager);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VPUManager_constructor(VPUManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(VPUManager);

// class's constructor
static void VPUManager_constructor(VPUManager this)
{
	ASSERT(this, "VPUManager::constructor: null this");

	__CONSTRUCT_BASE(Object);
}

// class's destructor
void VPUManager_destructor(VPUManager this)
{
	ASSERT(this, "VPUManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// wait frame
void VPUManager_waitForFrame(VPUManager this)
{
	ASSERT(this, "VPUManager::waitForFrame: null this");

	// disable interrupt
	VPUManager_disableInterrupt(this);

	//create an independant of software variable to point XPSTTS register
	unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

	//wait for screen to idle
	while (*xpstts & XPBSYR);
}

// disable interrupt
void VPUManager_disableInterrupt(VPUManager this)
{
	ASSERT(this, "VPUManager::disableInterrupt: null this");

	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
}

// enable interrupt
void VPUManager_enableInterrupt(VPUManager this)
{
	ASSERT(this, "VPUManager::enableInterrupt: null this");

	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	VIP_REGS[INTENB]= XPEND;
}

// turn display on
void VPUManager_displayOn(VPUManager this)
{
	ASSERT(this, "VPUManager::displayOn: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
	VIP_REGS[DPCTRL] = VIP_REGS[DPSTTS] | (SYNCE | RE | DISP);
	VIP_REGS[FRMCYC] = 0;
}

// turn display off
void VPUManager_displayOff(VPUManager this)
{
	ASSERT(this, "VPUManager::displayOff: null this");

	VIP_REGS[REST] = 0;
	VIP_REGS[XPCTRL] = 0;
	VIP_REGS[DPCTRL] = 0;
	VIP_REGS[FRMCYC] = 0;

	VPUManager_disableInterrupt(this);
}

// setup palettes
void VPUManager_setupPalettes(VPUManager this)
{
	ASSERT(this, "VPUManager::setupPalettes: null this");

	VIP_REGS[BRTA]  = __BRTA;
	VIP_REGS[BRTB]  = __BRTB;
	VIP_REGS[BRTC]  = __BRTC;
	VIP_REGS[GPLT0] = __GPLT0VALUE;	/* Set all eight palettes to: 11100100 */
	VIP_REGS[GPLT1] = __GPLT1VALUE;	/* (i.e. "Normal" dark to light progression.) */
	VIP_REGS[GPLT2] = __GPLT2VALUE;
	VIP_REGS[GPLT3] = __GPLT3VALUE;
	VIP_REGS[JPLT0] = __JPLT0VALUE;
	VIP_REGS[JPLT1] = __JPLT1VALUE;
	VIP_REGS[JPLT2] = __JPLT2VALUE;
	VIP_REGS[JPLT3] = __JPLT3VALUE;
	VIP_REGS[BKCOL] = __BKCOL;	/* Clear the screen to black before rendering */

}
// set brightness all the way up
void VPUManager_upBrightness(VPUManager this)
{
	ASSERT(this, "VPUManager::upBrightness: null this");

	VIP_REGS[BRTA] = 32;
	VIP_REGS[BRTB] = 64;
	VIP_REGS[BRTC] = 32;
}

// set brightness all way down
void VPUManager_displayHide(VPUManager this)
{
	ASSERT(this, "VPUManager::displayHide: null this");

	VIP_REGS[BRTA] = 0;
	VIP_REGS[BRTB] = 0;
	VIP_REGS[BRTC] = 0;

	// turn back the background
	VIP_REGS[BKCOL] = 0x00;
}


// clear screen
void VPUManager_clearScreen(VPUManager this)
{
	ASSERT(this, "VPUManager::clearScreen: null this");

	int i;
	//clear every bgmap segment
    for (i = 0; i < 14; i++)
	{
		Mem_clear((u16*)BGMap(i), 8192);
    }

	//clear every char segment
	Mem_clear ((u16*) CharSeg0, 8192);
	Mem_clear ((u16*) CharSeg1, 8192);
	Mem_clear ((u16*) CharSeg2, 8192);
	Mem_clear ((u16*) CharSeg3, 8192);
}

// clear bgmap
void VPUManager_clearBgmap(VPUManager this, int bgmap, int size)
{
	ASSERT(this, "VPUManager::clearBgmap: null this");

	Mem_clear((u16*)BGMap(bgmap), size);
}

// setup default column table
void VPUManager_setupColumnTable(VPUManager this)
{
	ASSERT(this, "VPUManager::setupColumnTable: null this");

	int i;
	for (i = 0; i < 128; i++)

	{
		CLMN_TBL[i] = columnTable[i];
		CLMN_TBL[i + 0x0080] = columnTable[127 - i];
		CLMN_TBL[i + 0x0100] = columnTable[i];
		CLMN_TBL[i + 0x0180] = columnTable[127 - i];
	}
}

