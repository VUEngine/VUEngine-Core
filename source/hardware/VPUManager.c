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

#define VPUManager_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\

// define the VPUManager
__CLASS_DEFINITION(VPUManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VPUManager_constructor(VPUManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

unsigned int volatile* _xpstts =	NULL;

__SINGLETON(VPUManager);

// class's constructor
static void VPUManager_constructor(VPUManager this)
{
	ASSERT(this, "VPUManager::constructor: null this");

	__CONSTRUCT_BASE();
	
	_xpstts = (unsigned int *)&VIP_REGS[XPSTTS];
}

// class's destructor
void VPUManager_destructor(VPUManager this)
{
	ASSERT(this, "VPUManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// disable interrupt
void VPUManager_disableInterrupt(VPUManager this)
{
	ASSERT(this, "VPUManager::disableInterrupt: null this");

	VIP_REGS[INTENB]= 0;
	VIP_REGS[INTCLR] = VIP_REGS[INTPND];
}

void VPUManager_enableDrawing(VPUManager this)
{
	ASSERT(this, "VPUManager::enableDrawing: null this");

	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN;
}

void VPUManager_idleDrawing(VPUManager this)
{
	ASSERT(this, "VPUManager::idleDrawing: null this");

	VIP_REGS[XPCTRL] |= XPRST;
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
	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS];
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

// setup backgorund color
void VPUManager_setupPalettes(VPUManager this, PaletteConfig* paletteConfig)
{
	ASSERT(this, "VPUManager::setupPalettes: null this");

	VIP_REGS[GPLT0] = paletteConfig->bgmap.gplt0;
	VIP_REGS[GPLT1] = paletteConfig->bgmap.gplt1;
	VIP_REGS[GPLT2] = paletteConfig->bgmap.gplt2;
	VIP_REGS[GPLT3] = paletteConfig->bgmap.gplt3;

	VIP_REGS[JPLT0] = paletteConfig->object.jplt0;
	VIP_REGS[JPLT1] = paletteConfig->object.jplt1;
	VIP_REGS[JPLT2] = paletteConfig->object.jplt2;
	VIP_REGS[JPLT3] = paletteConfig->object.jplt3;
	
	VIP_REGS[BKCOL] = paletteConfig->backgroundColor <= __COLOR_BRIGHT_RED? paletteConfig->backgroundColor: __COLOR_BRIGHT_RED;
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
void VPUManager_lowerBrightness(VPUManager this)
{
	ASSERT(this, "VPUManager::displayHide: null this");

	VIP_REGS[BRTA] = 0;
	VIP_REGS[BRTB] = 0;
	VIP_REGS[BRTC] = 0;

	VPUManager_setBackgroundColor(this, __COLOR_BLACK);
}


// clear screen
void VPUManager_clearScreen(VPUManager this)
{
	ASSERT(this, "VPUManager::clearScreen: null this");

	int i;
	//clear every bgmap segment
    for(i = 0; i < 14; i++)
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
	for(i = 0; i < 128; i++)
	{
		CLMN_TBL[i] = columnTable[i];
		CLMN_TBL[i + 0x0080] = columnTable[127 - i];
		CLMN_TBL[i + 0x0100] = columnTable[i];
		CLMN_TBL[i + 0x0180] = columnTable[127 - i];
	}
}

// set background color
void VPUManager_setBackgroundColor(VPUManager this, u8 color)
{
	ASSERT(this, "VPUManager::setBackgroundColor: null this");

    if (color > __COLOR_BRIGHT_RED)
    {
        color = __COLOR_BRIGHT_RED;
    }

	VIP_REGS[BKCOL] = color;
}
