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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <HardwareManager.h>
#include <Game.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <CommunicationManager.h>
#include <VIPManager.h>
#include <ClockManager.h>
#include <SpriteManager.h>
#include <Utilities.h>
#include <debugConfig.h>


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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			HardwareManager::getInstance()
 * @memberof	HardwareManager
 * @public
 * @return		HardwareManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void HardwareManager::constructor()
{
	Base::constructor();

	// set ROM waiting to 1 cycle
	_hardwareRegisters[__WCR] |= 0x0001;

	this->hwRegisters =	(u8*)0x02000000;
	this->timerManager = TimerManager::getInstance();
	this->vipManager = VIPManager::getInstance();
	this->keypadManager = KeypadManager::getInstance();

	//setup timer interrupts
	HardwareManager::setInterruptVectors(this);
	HardwareManager::setInterruptLevel(this, 0);
}

/**
 * Class destructor
 */
void HardwareManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Check that the memory map is sane
 */
void HardwareManager::checkMemoryMap()
{
	if((u32)&_dram_data_start < __WORLD_SPACE_BASE_ADDRESS && (u32)&_dram_bss_end >= __WORLD_SPACE_BASE_ADDRESS)
	{
		MemoryPool::getInstance();
		Printing::setDebugMode(Printing::getInstance());
		int y = 15;
		u32 missingSpace = (u32)&_dram_bss_end - __WORLD_SPACE_BASE_ADDRESS;
		u32 recommendedDramStart = (u32)&_dram_data_start - missingSpace;
		u32 recommendedDramSize = (__WORLD_SPACE_BASE_ADDRESS - recommendedDramStart);
		u32 recommendedBgmapSegments = (recommendedDramStart - __BGMAP_SPACE_BASE_ADDRESS) / 8192;

		Printing::text(Printing::getInstance(), "Increase the dram section in the vb.ld file", 1, y++, NULL);
		Printing::text(Printing::getInstance(), "Missing space: ", 1, ++y, NULL);
		Printing::int(Printing::getInstance(), missingSpace, 17, y, NULL);
		Printing::text(Printing::getInstance(), "Bytes ", 17 + Utilities::intLength(missingSpace) + 1, y++, NULL);
		Printing::text(Printing::getInstance(), "WORLD space: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (u32)__WORLD_SPACE_BASE_ADDRESS, 17, y, 4, NULL);
		Printing::text(Printing::getInstance(), "DRAM start: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (u32)&_dram_data_start, 17, y, 4, NULL);
		Printing::text(Printing::getInstance(), "DRAM end: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (u32)&_dram_bss_end, 17, y++, 4, NULL);
		Printing::text(Printing::getInstance(), "Suggested DRAM start: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), recommendedDramStart, 25, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Suggested DRAM size: ", 1, ++y, NULL);
		Printing::int(Printing::getInstance(), recommendedDramSize, 25, y, NULL);
		Printing::text(Printing::getInstance(), "Bytes ", 25 + Utilities::intLength(recommendedDramSize) + 1, y++, NULL);
		Printing::text(Printing::getInstance(), "Maximum BGMAP segments: ", 1, ++y, NULL);
		Printing::int(Printing::getInstance(), recommendedBgmapSegments, 25, y, NULL);

		NM_ASSERT(false, "HardwareManager::checkMemoryMap: DRAM section overflow");
	}
}

/**
 * Expansion port interrupt handle
 */
static void HardwareManager::croInterruptHandler()
{
	Printing::resetWorldCoordinates(Printing::getInstance());
	Printing::text(Printing::getInstance(), "EXP cron", 48 - 13, 0, NULL);
}

/**
 * Setup interrupt vectors
 */
void HardwareManager::setInterruptVectors()
{
	keyVector = (u32)KeypadManager::interruptHandler;
	timVector = (u32)TimerManager::interruptHandler;
	croVector = (u32)HardwareManager::croInterruptHandler;
	comVector = (u32)CommunicationManager::interruptHandler;
	vipVector = (u32)VIPManager::interruptHandler;
}

/**
 * Set the interrupt level
 *
 * @param level	 	Interrupt level
 */
void HardwareManager::setInterruptLevel(u8 level)
{
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
 * @return		 	Interrupt level
 */
int HardwareManager::getInterruptLevel()
{
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
 * Initialize the timer
 */
void HardwareManager::initializeTimer()
{
	TimerManager::initialize(this->timerManager);
}

/**
 * Clear the CHAR and Param table memory
 */
void HardwareManager::clearScreen()
{
	VIPManager::clearScreen(this->vipManager);
}

/**
 * Turn the displays on
 */
void HardwareManager::displayOn()
{
	VIPManager::displayOn(this->vipManager);
}

/**
 * Turn the displays off
 */
void HardwareManager::displayOff()
{
	VIPManager::displayOff(this->vipManager);
}

/**
 * Disable rendering
 */
void HardwareManager::disableRendering()
{
	// disable interrupt
	VIPManager::disableInterrupts(this->vipManager);
	VIPManager::disableDrawing(this->vipManager);
}

/**
 * Enable rendering
 */
void HardwareManager::enableRendering()
{
	// turn on display
	VIPManager::displayOn(this->vipManager);
	VIPManager::enableInterrupt(this->vipManager, __FRAMESTART | __XPEND);
	VIPManager::enableDrawing(this->vipManager);
}

/**
 * Turn brightness all the way up
 */
void HardwareManager::upBrightness()
{
	VIPManager::upBrightness(this->vipManager);
}

/**
 * Turn brightness all the way down
 */
void HardwareManager::lowerBrightness()
{
	VIPManager::lowerBrightness(this->vipManager);
}

/**
 * Setup the column table
 *
 * @param columnTableDefinition		Definition to use
 */
void HardwareManager::setupColumnTable(ColumnTableDefinition* columnTableDefinition)
{
	VIPManager::setupColumnTable(this->vipManager, columnTableDefinition);
}

/**
 * Enable user input processing
 */
void HardwareManager::enableKeypad()
{
	KeypadManager::enable(this->keypadManager);
}

/**
 * Disable user input processing
 */
void HardwareManager::disableKeypad()
{
	KeypadManager::disable(this->keypadManager);
}

/**
 * Enable interrupts
 */
static void HardwareManager::enableInterrupts()
{
	asm("cli");
}

/**
 * Disable interrupts
 */
static void HardwareManager::disableInterrupts()
{
	asm("sei");
}

/**
 * Enable multiplexed interrupts
 */
static void HardwareManager::enableMultiplexedInterrupts()
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
 */
static void HardwareManager::disableMultiplexedInterrupts()
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
static int HardwareManager::getStackPointer()
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
static int HardwareManager::getLinkPointer()
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
static int HardwareManager::getPSW()
{
	int psw;

	asm("			\n\
		stsr psw,%0	\n\
		"
	: "=r" (psw) // Output
	);
	return psw;
}

/**
 * Print manager's state
 *
 * @param x			Camera's x coordinate
 * @param y			Camera's y coordinate
 */
void HardwareManager::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "HARDWARE STATUS", x, y++, NULL);

	int auxY = y;
	int xDisplacement = 6;

	// print registries' status to know the call source
	Printing::text(Printing::getInstance(), "PSW:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getPSW(), x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "SP:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getStackPointer(), x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "LP:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getLinkPointer(), x + xDisplacement, auxY++, 4, NULL);

	Printing::text(Printing::getInstance(), "_hardwareRegisters", x, ++auxY, NULL);
	auxY++;
	Printing::text(Printing::getInstance(), "WCR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "CCR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__CCR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "CCSR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__CCSR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "CDTR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__CDTR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "CDRR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__CDRR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "SDLR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__SDLR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "SDHR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__SDHR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "TLR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__TLR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "THR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__THR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "TCR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__TCR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "WCR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__WCR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "SCR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _hardwareRegisters[__SCR], x + xDisplacement, auxY, 4, NULL);

	auxY = y + 4;
	x += 19;
	xDisplacement = 8;

	Printing::text(Printing::getInstance(), "_vipRegisters", x, ++auxY, NULL);
	auxY++;
	Printing::text(Printing::getInstance(), "INTPND:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__INTPND], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "INTENB:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__INTENB], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "INTCLR:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__INTCLR], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "DPSTTS:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__DPSTTS], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "DPCTRL:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__DPCTRL], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "BRTA:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), (u8)_vipRegisters[__BRTA], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "BRTB:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), (u8)_vipRegisters[__BRTB], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "BRTC:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), (u8)_vipRegisters[__BRTC], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "REST:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__REST], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "FRMCYC:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__FRMCYC], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "CTA:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__CTA], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "XPSTTS:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__XPSTTS], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "XPCTRL:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__XPCTRL], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "VER:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), _vipRegisters[__VER], x + xDisplacement, auxY, 4, NULL);

//	Printing::hex(Printing::getInstance(), HardwareManager::readKeypad(HardwareManager::getInstance()), 38, 5, 4, NULL);
}

#ifdef __ALERT_STACK_OVERFLOW

/**
 * Check for stack overflows
 */
static void HardwareManager::checkStackStatus()
{
	int sp;
	asm(" mov sp,%0  ": "=r" (sp));

	if((0x05000000 & sp) && sp < (int)&_bss_end)
	{
		HardwareManager::printStackStatus(HardwareManager::getInstance(), 1, 15, false);
		NM_ASSERT(false, "HardwareManager::checkStackStatus: stack overflown!");
	}
}

/**
 * Print the Stack Pointer's status
 *
 * @param x			Camera's x coordinate
 * @param y			Camera's y coordinate
 * @param resumed	Flag to print resumed or detailed info
 */
void HardwareManager::printStackStatus(int x, int y, bool resumed)
{
	int sp;
	asm(" mov sp,%0  ": "=r" (sp));

	int room = sp - (int)&_bss_end;

	static int lowestRoom = 65536;

	if(room < lowestRoom)
	{
		lowestRoom = room;
	}

	if(resumed)
	{
		if((__SCREEN_WIDTH_IN_CHARS) < x + Utilities::intLength(room) + 13)
		{
			x = (__SCREEN_WIDTH_IN_CHARS) - Utilities::intLength(room) - 13;
		}

		Printing::text(Printing::getInstance(), "   STACK'S ROOM        " , x - 3, y, NULL);
		Printing::int(Printing::getInstance(), room, x + 13, y, NULL);
	}
	else
	{
		if((__SCREEN_WIDTH_IN_CHARS) - 1 < Utilities::intLength(room) + 10)
		{
			x = (__SCREEN_WIDTH_IN_CHARS) - 1 - Utilities::intLength(room) - 11;
		}

		Printing::text(Printing::getInstance(), "   STACK'S STATUS" , x - 3, y, NULL);
		Printing::text(Printing::getInstance(), "Pointer:" , x, ++y, NULL);
		Printing::hex(Printing::getInstance(), sp, x + 10, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Bss' end:" , x, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int)&_bss_end, x + 10, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Room:           " , x, ++y, NULL);
		Printing::int(Printing::getInstance(), room, x + 10, y, NULL);
		Printing::text(Printing::getInstance(), "Lowest Room:           " , x, ++y, NULL);
		Printing::int(Printing::getInstance(), lowestRoom, x + 10, y, NULL);
	}
}
#endif
