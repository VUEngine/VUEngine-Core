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
#include <KeyPadManager.h>
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
 * @ingroup hardware
 */
__CLASS_DEFINITION(HardwareManager, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern u32 keyVector;
extern u32 timVector;
extern u32 croVector;
extern u32 comVector;
extern u32 vipVector;

#ifdef __ALERT_STACK_OVERFLOW
extern u32 _bss_end;
#endif

extern u32 _dram_bss_end;
extern u32 _dram_data_start;


int _lp = 0;
int _sp = 0;

void TimerManager_interruptHandler(void);
void KeypadManager_interruptHandler(void);
void VIPManager_interruptHandler(void);

static void HardwareManager_constructor(HardwareManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			HardwareManager_getInstance()
 * @memberof	HardwareManager
 * @public
 *
 * @return		HardwareManager instance
 */
__SINGLETON(HardwareManager);

/**
 * Class constructor
 *
 * @memberof	HardwareManager
 * @private
 *
 * @param this	Function scope
 */
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

/**
 * Class destructor
 *
 * @memberof	HardwareManager
 * @public
 *
 * @param this	Function scope
 */
void HardwareManager_destructor(HardwareManager this)
{
	ASSERT(this, "HardwareManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}


/**
 * Check that the memory map is sane
 *
 * @memberof	HardwareManager
 * @public
 */
void HardwareManager_checkMemoryMap()
{
	if((u32)&_dram_data_start < __WORLD_SPACE_BASE_ADDRESS && (u32)&_dram_bss_end >= __WORLD_SPACE_BASE_ADDRESS)
	{
		MemoryPool_getInstance();
		Printing_setDebugMode(Printing_getInstance());
		int y = 15;
		u32 missingSpace = (u32)&_dram_bss_end - __WORLD_SPACE_BASE_ADDRESS;
		u32 recommendedDramStart = (u32)&_dram_data_start - missingSpace;
		u32 recommendedDramSize = (__WORLD_SPACE_BASE_ADDRESS - recommendedDramStart);
		u32 recommendedBgmapSegments = (recommendedDramStart - __BGMAP_SPACE_BASE_ADDRESS) / 8192;

		Printing_text(Printing_getInstance(), "Increase the dram section in the vb.ld file", 1, y++, NULL);
		Printing_text(Printing_getInstance(), "Missing space: ", 1, ++y, NULL);
		Printing_int(Printing_getInstance(), missingSpace, 17, y, NULL);
		Printing_text(Printing_getInstance(), "Bytes ", 17 + Utilities_intLength(missingSpace) + 1, y++, NULL);
		Printing_text(Printing_getInstance(), "WORLD space: ", 1, ++y, NULL);
		Printing_hex(Printing_getInstance(), (u32)__WORLD_SPACE_BASE_ADDRESS, 17, y, 5, NULL);
		Printing_text(Printing_getInstance(), "DRAM start: ", 1, ++y, NULL);
		Printing_hex(Printing_getInstance(), (u32)&_dram_data_start, 17, y, 5, NULL);
		Printing_text(Printing_getInstance(), "DRAM end: ", 1, ++y, NULL);
		Printing_hex(Printing_getInstance(), (u32)&_dram_bss_end, 17, y++, 5, NULL);
		Printing_text(Printing_getInstance(), "Suggested DRAM start: ", 1, ++y, NULL);
		Printing_hex(Printing_getInstance(), recommendedDramStart, 25, y, 5, NULL);
		Printing_text(Printing_getInstance(), "Suggested DRAM size: ", 1, ++y, NULL);
		Printing_int(Printing_getInstance(), recommendedDramSize, 25, y, NULL);
		Printing_text(Printing_getInstance(), "Bytes ", 25 + Utilities_intLength(recommendedDramSize) + 1, y++, NULL);
		Printing_text(Printing_getInstance(), "Maximum BGMAP segments: ", 1, ++y, NULL);
		Printing_int(Printing_getInstance(), recommendedBgmapSegments, 25, y, NULL);

		NM_ASSERT(false, "HardwareManager::checkMemoryMap: DRAM section overflow");
	}
}

/**
 * Expansion port interrupt handle
 *
 * @memberof	HardwareManager
 * @public
 */
void HardwareManager_croInterruptHandler(void)
{
	Printing_resetWorldCoordinates(Printing_getInstance());
	Printing_text(Printing_getInstance(), "EXP cron", 48 - 13, 0, NULL);
}

/**
 * Communication port interrupt handle
 *
 * @memberof	HardwareManager
 * @public
 */
void HardwareManager_communicationInterruptHandler(void)
{
	Printing_resetWorldCoordinates(Printing_getInstance());
	Printing_text(Printing_getInstance(), "COM interrupt", 48 - 13, 0, NULL);
}

/**
 * Setup interrupt vectors
 *
 * @memberof	HardwareManager
 * @public
 *
 * @param this	Function scope
 */
void HardwareManager_setInterruptVectors(HardwareManager this __attribute__ ((unused)))
{
	keyVector = (u32)KeypadManager_interruptHandler;
	timVector = (u32)TimerManager_interruptHandler;
	croVector = (u32)HardwareManager_croInterruptHandler;
	comVector = (u32)HardwareManager_communicationInterruptHandler;
	vipVector = (u32)VIPManager_interruptHandler;
}

/**
 * Set the interrupt level
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 * @param level	 	Interrupt level
 */
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

/**
 * Get the interrupt level
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 *
 * @return		 	Interrupt level
 */
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

/**
 * Retrieve PSW
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 * @return		 	PSW
 */
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

/**
 * Retrieve the Stack Pointer's value
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 *
 * @return		 	Stack Pointer's value
 */
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

/**
 * Retrieve the Link Pointer's value
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 *
 * @return		 	Link Pointer's value
 */
int HardwareManager_getLinkPointer(HardwareManager this __attribute__ ((unused)))
{
	ASSERT(this, "HardwareManager::getLinkPointer: null this");

	int lp;
	asm(" \
		mov		lp,%0  \
		"
	: "=r" (lp) // Output
	);
	return lp;
}

/**
 * Initialize the timer
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_initializeTimer(HardwareManager this)
{
	ASSERT(this, "HardwareManager::initializeTimer: null this");

	TimerManager_initialize(this->timerManager);
}

/**
 * Clear the CHAR and Param table memory
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_clearScreen(HardwareManager this)
{
	ASSERT(this, "HardwareManager::clearScreen: null this");

	VIPManager_clearScreen(this->vipManager);
}

/**
 * Turn the displays on
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_displayOn(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOn: null this");

	VIPManager_displayOn(this->vipManager);
}

/**
 * Turn the displays off
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_displayOff(HardwareManager this)
{
	ASSERT(this, "HardwareManager::displayOff: null this");

	VIPManager_displayOff(this->vipManager);
}

/**
 * Disable rendering
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_disableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::disableRendering: null this");

	// disable interrupt
	VIPManager_disableInterrupts(this->vipManager);
	VIPManager_disableDrawing(this->vipManager);
}

/**
 * Enable rendering
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_enableRendering(HardwareManager this)
{
	ASSERT(this, "HardwareManager::enableRendering: null this");

	// turn on display
	VIPManager_displayOn(this->vipManager);
	VIPManager_enableInterrupt(this->vipManager, __FRAMESTART | __XPEND);
	VIPManager_enableDrawing(this->vipManager);
	VIPManager_resetFrameStarted(this->vipManager);
}

/**
 * Turn brightness all the way up
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_upBrightness(HardwareManager this)
{
	ASSERT(this, "HardwareManager::upBrightness: null this");

	VIPManager_upBrightness(this->vipManager);
}

/**
 * Turn brightness all the way down
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_lowerBrightness(HardwareManager this)
{
	ASSERT(this, "HardwareManager::lowerBrightness: null this");

	VIPManager_lowerBrightness(this->vipManager);
}

/**
 * Setup the column table
 *
 * @memberof						HardwareManager
 * @public
 *
 * @param this						Function scope
 * @param columnTableDefinition		Definition to use
 */
void HardwareManager_setupColumnTable(HardwareManager this, ColumnTableDefinition* columnTableDefinition)
{
	ASSERT(this, "HardwareManager::setupColumnTable: null this");

	VIPManager_setupColumnTable(this->vipManager, columnTableDefinition);
}

/**
 * Enable user input processing
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_enableKeypad(HardwareManager this)
{
	ASSERT(this, "HardwareManager::enableKeypad: null this");

	KeypadManager_enable(this->keypadManager);
}

/**
 * Disable user input processing
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
void HardwareManager_disableKeypad(HardwareManager this)
{
	ASSERT(this, "HardwareManager::disableKeypad: null this");

	KeypadManager_disable(this->keypadManager);
}

/**
 * Print manager's state
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 * @param x			Screen's x coordinate
 * @param y			Screen's y coordinate
 */
void HardwareManager_print(HardwareManager this, int x, int y)
{
	ASSERT(this, "HardwareManager::print: null this");

	Printing_text(Printing_getInstance(), "HARDWARE STATUS", x, y++, NULL);

	int auxY = y;
	int xDisplacement = 6;

	// print registries' status to know the call source
	Printing_text(Printing_getInstance(), "PSW:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getPSW(this), x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "SP:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getStackPointer(this), x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "LP:" , x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), HardwareManager_getLinkPointer(this), x + xDisplacement, auxY++, 5, NULL);

	Printing_text(Printing_getInstance(), "_hardwareRegisters", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "CCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CCR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "CCSR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CCSR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "CDTR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CDTR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "CDRR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__CDRR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "SDLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SDLR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "SDHR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SDHR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "TLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__TLR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "THR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__THR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "TCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__TCR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "WCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "SCR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _hardwareRegisters[__SCR], x + xDisplacement, auxY, 5, NULL);

	auxY = y;
	x += 17;
	xDisplacement = 8;

	Printing_text(Printing_getInstance(), "_vipRegisters", x, ++auxY, NULL);
	Printing_text(Printing_getInstance(), "INTPND:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTPND], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "INTENB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTENB], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "INTCLR:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__INTCLR], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "DPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__DPSTTS], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "DPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__DPCTRL], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "BRTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTA], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "BRTB:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTB], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "BRTC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), (u8)_vipRegisters[__BRTC], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "REST:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__REST], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "FRMCYC:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__FRMCYC], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "CTA:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__CTA], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "XPSTTS:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__XPSTTS], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "XPCTRL:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__XPCTRL], x + xDisplacement, auxY, 5, NULL);
	Printing_text(Printing_getInstance(), "VER:", x, ++auxY, NULL);
	Printing_hex(Printing_getInstance(), _vipRegisters[__VER], x + xDisplacement, auxY, 5, NULL);

//	Printing_hex(Printing_getInstance(), HardwareManager_readKeypad(HardwareManager_getInstance()), 38, 5, 5, NULL);
}

#ifdef __ALERT_STACK_OVERFLOW

/**
 * Check for stack overflows
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 */
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

/**
 * Print the Stack Pointer's status
 *
 * @memberof		HardwareManager
 * @public
 *
 * @param this		Function scope
 * @param x			Screen's x coordinate
 * @param y			Screen's y coordinate
 * @param resumed	Flag to print resumed or detailed info
 */
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
		Printing_hex(Printing_getInstance(), sp, x + 10, y, 5, NULL);
		Printing_text(Printing_getInstance(), "Bss' end:" , x, ++y, NULL);
		Printing_hex(Printing_getInstance(), (int)&_bss_end, x + 10, y, 5, NULL);
		Printing_text(Printing_getInstance(), "Room:           " , x, ++y, NULL);
		Printing_int(Printing_getInstance(), room, x + 10, y, NULL);
	}
}
#endif
