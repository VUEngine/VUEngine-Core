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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <HardwareManager.h>
#include <Game.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VIPManager.h>
#include <ClockManager.h>
#include <SpriteManager.h>
#include <Printing.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------

#ifdef __ALERT_STACK_OVERFLOW
extern u32 _bss_end;
#endif


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define HardwareManager_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* Timer manager */																				\
		TimerManager timerManager;																		\
		/* VPU manager */																				\
		VIPManager vipManager;																			\
		/* VPU manager */																				\
		KeypadManager keypadManager;																	\
		/* HW registry */																				\
		u8*  hwRegisters;																				\

/**
 * @class	HardwareManager
 * @extends Object
 */
__CLASS_DEFINITION(HardwareManager, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern u32 key_vector;
extern u32 tim_vector;
extern u32 cro_vector;
extern u32 com_vector;
extern u32 vpu_vector;


int _lp = 0;
int _sp = 0;

void TimerManager_interruptHandler(void);
void KeypadManager_interruptHandler(void);
void VIPManager_interruptHandler(void);

static void HardwareManager_constructor(HardwareManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(HardwareManager);

// class's constructor
static void __attribute__ ((noinline)) HardwareManager_constructor(HardwareManager this)
{
	ASSERT(this, "HardwareManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// set ROM waiting to 1 cycle
	_hardwareRegisters[__WCR] |= 0x0001;

	this->hwRegisters =	(u8*)0x02000000;
	this->timerManager = TimerManager_getInstance();
	this->vipManager = VIPManager_getInstance();
	this->keypadManager = KeypadManager_getInstance();
}

// class's destructor
void HardwareManager_destructor(HardwareManager this)
{
	ASSERT(this, "HardwareManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// cro's (expansion port) interrupt handler
void HardwareManager_croInterruptHandler(void)
{
	Printing_text(Printing_getInstance(), "EXP cron", 48 - 13, 0, NULL);
}

// com's (link port) interrupt handler
void HardwareManager_communicationInterruptHandler(void)
{
	Printing_text(Printing_getInstance(), "COM interrupt", 48 - 13, 0, NULL);
}

// setup interrupt vectors
void HardwareManager_setInterruptVectors(HardwareManager this __attribute__ ((unused)))
{
	key_vector = (u32)KeypadManager_interruptHandler;
	tim_vector = (u32)TimerManager_interruptHandler;
	cro_vector = (u32)HardwareManager_croInterruptHandler;
	com_vector = (u32)HardwareManager_communicationInterruptHandler;
	vpu_vector = (u32)VIPManager_interruptHandler;
}

// set interruption level
void HardwareManager_setInterruptLevel(HardwareManager this __attribute__ ((unused)), u8 level)
{
	ASSERT(this, "HardwareManager::setInterruptLevel: null this");

	asm(" \n\
		stsr	sr5,r6 \n\
		movhi	0xFFF1,r0,r7 \n\
		movea	0xFFFF,r7,r7 \n\
		and		r6,r7 \n\
		mov		%0,r6 \n\
		andi	0x000F,r6,r6 \n\
		shl		0x10,r6 \n\
		or		r7,r6 \n\
		ldsr	r6,sr5 \n\
		"
	: // Output
	: "r" (level) // Input
	: "r6", "r7" // Clobber
	);
}

// get interruption level
int HardwareManager_getInterruptLevel(HardwareManager this __attribute__ ((unused)))
{
	ASSERT(this, "HardwareManager::geInterruptLevel: null this");

	int level;

	asm(" \n\
		stsr	sr5,r6 \n\
		shr		0x10,r6 \n\
		andi	0x000F,r6,r6 \n\
		mov		r6,%0 \n\
	"
	: "=r" (level) // Output
	: // Input
	: "r6" // Clobber
	);

	return level;
}

// get PSW
inline int HardwareManager_getPSW(HardwareManager this __attribute__ ((unused)))
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
int HardwareManager_getStackPointer(HardwareManager this __attribute__ ((unused)))
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

// get stack pointer
int HardwareManager_getLPointer(HardwareManager this __attribute__ ((unused)))
{
	ASSERT(this, "HardwareManager::getStackPointer: null this");

	int lp;
	asm(" \
		mov		lp,%0  \
		"
	: "=r" (lp) // Output
	);
	return lp;
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

	VIPManager_clearScreen(this->vipManager);
}

// display on
void HardwareManager_displayOn(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOn: null this");

	VIPManager_displayOn(this->vipManager);
}

// display off
void HardwareManager_displayOff(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOff: null this");

	VIPManager_displayOff(this->vipManager);
}

// disable VPU interrupts
void HardwareManager_disableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::disableRendering: null this");

	// disable interrupt
	VIPManager_disableInterrupts(this->vipManager);
}

// enable VPU interrupts
void HardwareManager_enableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::enableRendering: null this");

	// turn on display
	VIPManager_displayOn(this->vipManager);
	VIPManager_enableDrawing(this->vipManager);
	VIPManager_enableInterrupt(VIPManager_getInstance(), __GAMESTART);
}

// make sure the brightness is ok
void HardwareManager_upBrightness(HardwareManager this)
{
	ASSERT(this, "HardwareManager::upBrightness: null this");

	VIPManager_upBrightness(this->vipManager);
}

// lower display brightness
void HardwareManager_lowerBrightness(HardwareManager this)
{
	ASSERT(this, "HardwareManager::lowerBrightness: null this");

	VIPManager_lowerBrightness(this->vipManager);
}

// setup default column table
void HardwareManager_setupColumnTable(HardwareManager this, ColumnTableDefinition* columnTableDefinition)
{
	ASSERT(this, "HardwareManager::setupColumnTable: null this");

	VIPManager_setupColumnTable(this->vipManager, columnTableDefinition);
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

// print hardware's states
void HardwareManager_print(HardwareManager this, int x, int y)
{
	ASSERT(this, "HardwareManager::print: null this");

	Printing_text(Printing_getInstance(), "HARDWARE STATUS", x, y++, NULL);

	int auxY = y;
	int xDisplacement = 6;

	// print registries' status to know the call source
	Printing_text(Printing_getInstance(), "PSW:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getPSW(this), x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "SP:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getStackPointer(this), x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "LP:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getLPointer(this), x + xDisplacement, auxY++, 8, NULL);

	Printing_text(Printing_getInstance(), "_hardwareRegisters", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "CCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CCR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "CCSR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CCSR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "CDTR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CDTR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "CDRR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CDRR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "SDLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SDLR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "SDHR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SDHR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "TLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__TLR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "THR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__THR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "TCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__TCR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "SCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SCR], x + xDisplacement, auxY, 8, NULL);

	auxY = y;
	x += 17;
	xDisplacement = 8;

	Printing_text(Printing_getInstance(), "_vipRegisters", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "INTPND:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTPND], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "INTENB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTENB], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "INTCLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTCLR], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "DPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__DPSTTS], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "DPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__DPCTRL], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "BRTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTA], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "BRTB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTB], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "BRTC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTC], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "REST:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__REST], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "FRMCYC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__FRMCYC], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "CTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__CTA], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "XPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__XPSTTS], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "XPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__XPCTRL], x + xDisplacement, auxY, 8, NULL);
	Printing_text(Printing_getInstance(), "VER:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__VER], x + xDisplacement, auxY, 8, NULL);

//	Printing_hex(Printing_getInstance(), HardwareManager_readKeypad(HardwareManager_getInstance()), 38, 5, 8, NULL);
}

#ifdef __ALERT_STACK_OVERFLOW
void HardwareManager_checkStackStatus(HardwareManager this __attribute__ ((unused)))
{
	ASSERT(this, "HardwareManager::checkStackStatus: null this");

	int sp;
	asm(" mov sp,%0  ": "=r" (sp));

	if((0x05000000 & sp) && sp < (int)&_bss_end)
	{
		HardwareManager_printStackStatus(HardwareManager_getInstance(), 1, 15, false);
		NM_ASSERT(false, "HardwareManager::checkStackStatus: stack overflown!");
	}
}

void HardwareManager_printStackStatus(HardwareManager this __attribute__ ((unused)), int x, int y, bool resumed)
{
	ASSERT(this, "HardwareManager::print: null this");

	int sp;
	asm(" mov sp,%0  ": "=r" (sp));

	int room = sp - (int)&_bss_end;

	if(resumed)
	{
		if((__SCREEN_WIDTH >> 3) < x + Utilities_intLength(room) + 13)
		{
			x = (__SCREEN_WIDTH >> 3) - Utilities_intLength(room) - 13;
		}

		Printing_text(Printing_getInstance(), "   STACK'S ROOM        " , x - 3, y, NULL);
		Printing_int(Printing_getInstance(), room, x + 13, y, NULL);
	}
	else
	{
		if((__SCREEN_WIDTH >> 3) - 1 < Utilities_intLength(room) + 10)
		{
			x = (__SCREEN_WIDTH >> 3) - 1 - Utilities_intLength(room) - 10;
		}

		Printing_text(Printing_getInstance(), "   STACK'S STATUS" , x - 3, y, NULL);
		Printing_text(Printing_getInstance(), "Pointer:" , x, ++y, NULL);
		Printing_hex(Printing_getInstance(), sp, x + 10, y, 8, NULL);
		Printing_text(Printing_getInstance(), "Bss' end:" , x, ++y, NULL);
		Printing_hex(Printing_getInstance(), (int)&_bss_end, x + 10, y, 8, NULL);
		Printing_text(Printing_getInstance(), "Room:           " , x, ++y, NULL);
		Printing_int(Printing_getInstance(), room, x + 10, y, NULL);
	}
}
#endif
