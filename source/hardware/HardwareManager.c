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

#include <HardwareManager.h>
#include <Game.h>
#include <ClockManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define HardwareManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* Timer manager */															\
	TimerManager timerManager;													\
																				\
	/* VPU manager */															\
	VPUManager vpuManager;														\
																				\
	/* VPU manager */															\
	KeypadManager keypadManager;												\
																				\
	/* HW registry */															\
	u8*  hwRegisters;															\

// define the HardwareManager
__CLASS_DEFINITION(HardwareManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern u32 key_vector;
extern u32 tim_vector;
extern u32 cro_vector;
extern u32 com_vector;
extern u32 vpu_vector;

static HardwareManager _hardwareManager = NULL;
static TimerManager _timerManager;
static ClockManager _clockManager;


// class's constructor
static void HardwareManager_constructor(HardwareManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(HardwareManager);

// class's constructor
static void HardwareManager_constructor(HardwareManager this)
{
	ASSERT(this, "HardwareManager::constructor: null this");

	__CONSTRUCT_BASE();

	// set ROM waiting to 1 cycle
	HW_REGS[WCR] |= 0x0001;

	this->hwRegisters =	(u8*)0x02000000;
	this->timerManager = TimerManager_getInstance();
	this->vpuManager = VPUManager_getInstance();
	this->keypadManager = KeypadManager_getInstance();

	_hardwareManager = this;
	_timerManager = TimerManager_getInstance();
	_clockManager = ClockManager_getInstance();
}

// class's destructor
void HardwareManager_destructor(HardwareManager this)
{
	ASSERT(this, "HardwareManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// timer's interrupt handler
void HardwareManager_timerInterruptHandler()
{
	ASSERT(_hardwareManager, "HardwareManager::timerInterruptHandler: null _hardwareManager");

	//disable interrupts
	TimerManager_setInterrupt(_timerManager, false);

	// update clocks
	ClockManager_update(_clockManager, __TIMER_RESOLUTION);

	// enable interrupts
	TimerManager_setInterrupt(_timerManager, true);
}

// keypad's interrupt handler
void HardwareManager_keypadInterruptHandler(void)
{
	// broadcast keypad event
	Printing_text(Printing_getInstance(), "KYP interrupt", 48 - 13, 0, NULL);
}

// cro's interrupt handler
void HardwareManager_croInterruptHandler(void)   // Expansion Port Interrupt Handler
{
	Printing_text(Printing_getInstance(), "EXP interrupt", 48 - 13, 0, NULL);
}

// com's interrupt handler
void HardwareManager_communicationInterruptHandler(void)   // Link Port Interrupt Handler
{
	Printing_text(Printing_getInstance(), "COM interrupt", 48 - 13, 0, NULL);
}

// vpu's interrupt handler
void HardwareManager_vpuInterruptHandler(void)
{
	// don't use these interrupt, they introduce a strange behavior in the machine
	VPUManager_disableInterrupt(VPUManager_getInstance());

	Printing_text(Printing_getInstance(), "VPU interrupt", 48 - 13, 0, NULL);

}

// setup interrupt vectors
void HardwareManager_setInterruptVectors(HardwareManager this)
{
	key_vector = (u32)HardwareManager_keypadInterruptHandler;
	tim_vector = (u32)HardwareManager_timerInterruptHandler;
	cro_vector = (u32)HardwareManager_croInterruptHandler;
	com_vector = (u32)HardwareManager_communicationInterruptHandler;
	vpu_vector = (u32)HardwareManager_vpuInterruptHandler;
}

// set interruption level
void HardwareManager_setInterruptLevel(HardwareManager this, u8 level)
{
	ASSERT(this, "HardwareManager::setInterruptLevel: null this");

	asm(" \n\
		stsr	sr5,r5 \n\
		movhi	0xFFF1,r0,r6 \n\
		movea	0xFFFF,r6,r6 \n\
		and		r5,r6 \n\
		mov		%0,r5 \n\
		andi	0x000F,r5,r5 \n\
		shl		0x10,r5 \n\
		or		r6,r5 \n\
		ldsr	r5,sr5 \n\
		"
	: // Output
	: "r" (level) // Input
	: "r5", "r6" // Clobber
	);
}

// get interruption level
inline int HardwareManager_getInterruptLevel(HardwareManager this)
{
	ASSERT(this, "HardwareManager::geInterruptLevel: null this");

	int level;

	asm(" \n\
		stsr	sr5,r5 \n\
		shr		0x10,r5 \n\
		andi	0x000F,r5,r5 \n\
		mov		r5,%0 \n\
	"
	: "=r" (level) // Output
	: // Input
	: "r5" // Clobber
	);

	return level;
}

// get PSW
inline int HardwareManager_getPSW(HardwareManager this)
{
	ASSERT(this, "HardwareManager::getPSW: null this");

	int psw;
	asm(" \n\
		stsr	psw,%0  \n\
		"
	: "=r" (psw) // Output
	);
	return psw;
}

// get stack pointer
int HardwareManager_getStackPointer(HardwareManager this)
{
	ASSERT(this, "HardwareManager::getStackPointer: null this");

	int sp;
	asm(" \
		mov		sp,%0  \
		"
	: "=r" (sp) // Output
	);
	return sp;
}

// initialize timer
void HardwareManager_initializeTimer(HardwareManager this)
{
	ASSERT(this, "HardwareManager::initializeTimer: null this");

	TimerManager_initialize(this->timerManager);
}

// clear screen
void HardwareManager_clearScreen(HardwareManager this)
{
	ASSERT(this, "HardwareManager::clearScreen: null this");

	VPUManager_clearScreen(this->vpuManager);
}

// display on
void HardwareManager_displayOn(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOn: null this");

	VPUManager_displayOn(this->vpuManager);
	VPUManager_setupPalettes(this->vpuManager);
}

// display off
void HardwareManager_displayOff(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOff: null this");

	VPUManager_displayOff(this->vpuManager);
}

// disable VPU interrupts
void HardwareManager_disableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::disableRendering: null this");

	// disable interrupt
	VPUManager_disableInterrupt(this->vpuManager);
}

// enable VPU interrupts
void HardwareManager_enableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::enableRendering: null this");

	// turn on display
	VPUManager_displayOn(this->vpuManager);
}

// make sure the brigtness is ok
void HardwareManager_upBrightness(HardwareManager this)
{
	ASSERT(this, "HardwareManager::upBrightness: null this");

	VPUManager_upBrightness(this->vpuManager);
}

// setup default column table
void HardwareManager_setupColumnTable(HardwareManager this)
{
	ASSERT(this, "HardwareManager::setupColumnTable: null this");

	VPUManager_setupColumnTable(this->vpuManager);
}

// enable key pad
void HardwareManager_enableKeypad(HardwareManager this)
{
	ASSERT(this, "HardwareManager::enableKeypad: null this");

	KeypadManager_enable(this->keypadManager);
}

// disable key pad
void HardwareManager_disableKeypad(HardwareManager this)
{
	ASSERT(this, "HardwareManager::disableKeypad: null this");

	KeypadManager_disable(this->keypadManager);
}

// read keypad
u16 HardwareManager_readKeypad(HardwareManager this)
{
	ASSERT(this, "HardwareManager::readKeypad: null this");

	return KeypadManager_read(this->keypadManager);
}

// print hardware's states
void HardwareManager_print(HardwareManager this, int x, int y)
{
	Printing_text(Printing_getInstance(), "HARDWARE'S STATUS", x, y++, NULL);

	int auxY = y;
	int xDisplacement = 6;

	Printing_text(Printing_getInstance(), "   HW_REGS", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[WCR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "CCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[CCR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "CCSR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[CCSR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "CDTR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[CDTR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "CDRR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[CDRR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "SDLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[SDLR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "SDHR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[SDHR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "TLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[TLR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "THR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[THR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "TCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[TCR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[WCR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "SCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HW_REGS[SCR], x + xDisplacement, auxY, NULL);
/*
#define		0x01
#define		0x02
#define		0x10
#define		0x11
#define		0x12
#define	BRTB	0x13
#define	BRTC	0x14
#define		0x15
#define		0x17
#define			0x18
#define		0x20
#define		0x21
#define			0x22
*/
	auxY = y;
	x += 17;
	xDisplacement = 8;

	Printing_text(Printing_getInstance(), "   VIP_REGS", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "INTPND:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[INTPND], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "INTENB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[INTENB], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "INTCLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[INTCLR], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "DPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[DPSTTS], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "DPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[DPCTRL], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "BRTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[BRTA], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "BRTB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[BRTB], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "BRTC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[BRTC], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "REST:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[REST], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "FRMCYC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[FRMCYC], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "CTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[CTA], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "XPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[XPSTTS], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "XPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[XPCTRL], x + xDisplacement, auxY, NULL);
	Printing_text(Printing_getInstance(), "VER:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), VIP_REGS[VER], x + xDisplacement, auxY, NULL);

//	Printing_hex(Printing_getInstance(), HardwareManager_readKeypad(HardwareManager_getInstance()), 38, 5, NULL);
}