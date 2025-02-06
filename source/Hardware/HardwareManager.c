/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTexture.h>
#include <CommunicationManager.h>
#include <FrameBufferManager.h>
#include <DebugConfig.h>
#include <KeypadManager.h>
#include <Printer.h>
#include <RumbleManager.h>
#include <SoundManager.h>
#include <Sprite.h>
#include <SRAMManager.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VIPManager.h>
#include <VSUManager.h>

#include "HardwareManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern uint32 keyVector;
extern uint32 timVector;
extern uint32 croVector;
extern uint32 comVector;
extern uint32 vipVector;
extern uint32 zeroDivisionVector;
extern uint32 invalidOpcodeVector;
extern uint32 floatingPointVector;

extern uint32 _dramBssEnd;
extern uint32 _dramDataStart;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef const struct ROMInfo
{
	// Game Title
	char title[20];

	// Reserved
	BYTE reserved[5];

	// Published ID
	char publisherID[2];

	// Published ID
	char gameID[4];

	// ROM Version
	BYTE version;

} ROMInfo;

ROMInfo romInfo __attribute__((section(".rominfo"))) =
{
	__GAME_TITLE,
	{0x00, 0x00, 0x00, 0x00, 0x00},
	__MAKER_CODE,
	__GAME_CODE,
	__ROM_VERSION
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8* const _hardwareRegisters = (uint8*)0x02000000;
int32 _vuengineLinkPointer = 0;
int32 _vuengineStackPointer = 0;
bool _stackHeadroomViolation = false;
bool _enabledInterrupts __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = false;
int16 _suspendInterruptRequest __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 0;
uint32 _wramSample __attribute__((section(".dram_dirty"))) __attribute__((unused));
uint32 _sramSample __attribute__((section(".dram_dirty"))) __attribute__((unused));

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::initialize()
{
	HardwareManager::disableInterrupts();

	// Set ROM waiting to 1 cycle
	_hardwareRegisters[__WCR] |= 0x0001;

	// Check memory map before anything else
	HardwareManager::checkMemoryMap();

	// Setup interrupts
	HardwareManager::setInterruptVectors();
	HardwareManager::setInterruptLevel(0);
	HardwareManager::setExceptionVectors();

	// Reset the hardware managers
	HardwareManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::reset()
{
	HardwareManager::suspendInterrupts();

	// Reset hardware managers
	CommunicationManager::reset(CommunicationManager::getInstance());
	FrameBufferManager::reset(FrameBufferManager::getInstance());
	KeypadManager::reset(KeypadManager::getInstance());
	RumbleManager::reset(RumbleManager::getInstance());
	VSUManager::reset(VSUManager::getInstance());
	SRAMManager::reset(SRAMManager::getInstance());
	TimerManager::reset(TimerManager::getInstance());
	VIPManager::reset(VIPManager::getInstance());

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::print(int32 x, int32 y)
{
	Printer::text("HARDWARE STATUS", x, y++, NULL);

	int32 auxY = y;
	int32 xDisplacement = 5;

	// Print registries' status to know the call source
	Printer::text("PSW:" , x, ++auxY, NULL);
	Printer::hex(HardwareManager::getPSW(), x + xDisplacement, auxY, 4, NULL);
	Printer::text("SP:" , x, ++auxY, NULL);
	Printer::hex(HardwareManager::getStackPointer(), x + xDisplacement, auxY, 4, NULL);
	Printer::text("LP:" , x, ++auxY, NULL);
	Printer::hex(HardwareManager::getLinkPointer(), x + xDisplacement, auxY++, 4, NULL);

	Printer::text("Hardware\nRegisters", x, ++auxY, NULL);
	auxY+=2;
	Printer::text("WCR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__WCR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("CCR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__CCR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("CCSR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__CCSR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("CDTR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__CDTR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("CDRR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__CDRR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("SDLR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__SDLR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("SDHR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__SDHR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("TLR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__TLR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("THR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__THR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("TCR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__TCR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("WCR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__WCR], x + xDisplacement, auxY, 4, NULL);
	Printer::text("SCR:", x, ++auxY, NULL);
	Printer::hex(_hardwareRegisters[__SCR], x + xDisplacement, auxY, 4, NULL);

	auxY = y + 4;
	x += 11;
	xDisplacement = 8;

	VIPManager::print(x, y);

//	Printer::hex(HardwareManager::readKeypad(HardwareManager::getInstance()), 38, 5, 4, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::printStackStatus(int32 x, int32 y, bool resumed)
{
	int32 sp;
	asm
	(
		"mov	sp, %0"
		: "=r" (sp)
	);

	int32 room = sp - (int32)&_bssEnd;

	if(resumed)
	{
		if((__SCREEN_WIDTH_IN_CHARS) < x + Math::getDigitsCount(room) + 13)
		{
			x = (__SCREEN_WIDTH_IN_CHARS) - Math::getDigitsCount(room) - 13;
		}

		Printer::text("   STACK'S ROOM		" , x - 3, y, NULL);
		Printer::int32(room, x + 13, y, NULL);
	}
	else
	{
		if((__SCREEN_WIDTH_IN_CHARS) - 1 < Math::getDigitsCount(room) + 15)
		{
			x = (__SCREEN_WIDTH_IN_CHARS) - 1 - Math::getDigitsCount(room) - 11;
		}

		Printer::text("   STACK'S STATUS" , x - 3, y, NULL);
		Printer::text("Bss' end:" , x, ++y, NULL);
		Printer::hex((int32)&_bssEnd, x + 15, y, 4, NULL);
		Printer::text("Stack Pointer:" , x, ++y, NULL);
		Printer::hex(sp, x + 15, y, 4, NULL);
		Printer::text("Minimum Room:		   " , x, ++y, NULL);
		Printer::int32(__STACK_HEADROOM, x + 15, y, NULL);
		Printer::text("Actual Room:		   " , x, ++y, NULL);
		Printer::int32(room, x + 15, y, NULL);
		Printer::text("Overflow:		   " , x, ++y, NULL);
		Printer::int32(__STACK_HEADROOM - room, x + 15, y, NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::checkMemoryMap()
{
	if((uint32)&_dramDataStart < __WORLD_SPACE_BASE_ADDRESS && (uint32)&_dramBssEnd >= __WORLD_SPACE_BASE_ADDRESS)
	{
		Printer::setDebugMode();
		int32 y = 15;
		uint32 missingSpace = (uint32)&_dramBssEnd - __WORLD_SPACE_BASE_ADDRESS;
		uint32 recommendedDramStart = (uint32)&_dramDataStart - missingSpace;
		uint32 recommendedDramSize = (__WORLD_SPACE_BASE_ADDRESS - recommendedDramStart);
		uint32 recommendedBgmapSegments = (recommendedDramStart - __BGMAP_SPACE_BASE_ADDRESS) / 8192;

		Printer::text("Increase the dram section in the vb.ld file", 1, y++, NULL);
		Printer::text("Missing space: ", 1, ++y, NULL);
		Printer::int32(missingSpace, 17, y, NULL);
		Printer::text("Bytes ", 17 + Math::getDigitsCount(missingSpace) + 1, y++, NULL);
		Printer::text("WORLD space: ", 1, ++y, NULL);
		Printer::hex((uint32)__WORLD_SPACE_BASE_ADDRESS, 17, y, 4, NULL);
		Printer::text("DRAM start: ", 1, ++y, NULL);
		Printer::hex((uint32)&_dramDataStart, 17, y, 4, NULL);
		Printer::text("DRAM end: ", 1, ++y, NULL);
		Printer::hex((uint32)&_dramBssEnd, 17, y++, 4, NULL);
		Printer::text("Suggested DRAM start: ", 1, ++y, NULL);
		Printer::hex(recommendedDramStart, 25, y, 4, NULL);
		Printer::text("Suggested DRAM size: ", 1, ++y, NULL);
		Printer::int32(recommendedDramSize, 25, y, NULL);
		Printer::text("Bytes ", 25 + Math::getDigitsCount(recommendedDramSize) + 1, y++, NULL);
		Printer::text("Maximum BGMAP segments: ", 1, ++y, NULL);
		Printer::int32(recommendedBgmapSegments, 25, y, NULL);

		NM_ASSERT(false, "HardwareManager::checkMemoryMap: DRAM section overflow");
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::croInterruptHandler()
{
	Printer::text("EXP cron", 48 - 13, 0, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::setInterruptVectors()
{
	keyVector = (uint32)KeypadManager::interruptHandler;
	timVector = (uint32)TimerManager::interruptHandler;
	croVector = (uint32)HardwareManager::croInterruptHandler;
	comVector = (uint32)CommunicationManager::interruptHandler;
	vipVector = (uint32)VIPManager::interruptHandler;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::setExceptionVectors()
{
	zeroDivisionVector = (uint32)Error::zeroDivisionException;
	invalidOpcodeVector = (uint32)Error::invalidOpcodeException;
	floatingPointVector = (uint32)Error::floatingPointException;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 HardwareManager::getInterruptLevel()
{
	int32 level;

	asm
	(
		"stsr	sr5, r6			\n\t"	  \
		"shr	0x10, r6		\n\t"	  \
		"andi	0x000F, r6, r6	\n\t"	  \
		"mov	r6, %0			\n\t"
		: "=r" (level) // Output
		: // Input
		: "r6" // Clobber
	);

	return level;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
