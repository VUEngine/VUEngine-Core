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

#include <CommunicationManager.h>
#include <DebugConfig.h>
#include <KeypadManager.h>
#include <Printing.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VIPManager.h>

#include "HardwareManager.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
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
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::checkMemoryMap()
{
	if((uint32)&_dramDataStart < __WORLD_SPACE_BASE_ADDRESS && (uint32)&_dramBssEnd >= __WORLD_SPACE_BASE_ADDRESS)
	{
		MemoryPool::getInstance();
		Printing::setDebugMode(Printing::getInstance());
		int32 y = 15;
		uint32 missingSpace = (uint32)&_dramBssEnd - __WORLD_SPACE_BASE_ADDRESS;
		uint32 recommendedDramStart = (uint32)&_dramDataStart - missingSpace;
		uint32 recommendedDramSize = (__WORLD_SPACE_BASE_ADDRESS - recommendedDramStart);
		uint32 recommendedBgmapSegments = (recommendedDramStart - __BGMAP_SPACE_BASE_ADDRESS) / 8192;

		Printing::text(Printing::getInstance(), "Increase the dram section in the vb.ld file", 1, y++, NULL);
		Printing::text(Printing::getInstance(), "Missing space: ", 1, ++y, NULL);
		Printing::int32(Printing::getInstance(), missingSpace, 17, y, NULL);
		Printing::text(Printing::getInstance(), "Bytes ", 17 + Math::getDigitsCount(missingSpace) + 1, y++, NULL);
		Printing::text(Printing::getInstance(), "WORLD space: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (uint32)__WORLD_SPACE_BASE_ADDRESS, 17, y, 4, NULL);
		Printing::text(Printing::getInstance(), "DRAM start: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (uint32)&_dramDataStart, 17, y, 4, NULL);
		Printing::text(Printing::getInstance(), "DRAM end: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), (uint32)&_dramBssEnd, 17, y++, 4, NULL);
		Printing::text(Printing::getInstance(), "Suggested DRAM start: ", 1, ++y, NULL);
		Printing::hex(Printing::getInstance(), recommendedDramStart, 25, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Suggested DRAM size: ", 1, ++y, NULL);
		Printing::int32(Printing::getInstance(), recommendedDramSize, 25, y, NULL);
		Printing::text(Printing::getInstance(), "Bytes ", 25 + Math::getDigitsCount(recommendedDramSize) + 1, y++, NULL);
		Printing::text(Printing::getInstance(), "Maximum BGMAP segments: ", 1, ++y, NULL);
		Printing::int32(Printing::getInstance(), recommendedBgmapSegments, 25, y, NULL);

		NM_ASSERT(false, "HardwareManager::checkMemoryMap: DRAM section overflow");
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::croInterruptHandler()
{
	Printing::resetCoordinates(Printing::getInstance());
	Printing::text(Printing::getInstance(), "EXP cron", 48 - 13, 0, NULL);
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
		"stsr	sr5, r6			\n\t"      \
		"shr	0x10, r6		\n\t"      \
		"andi	0x000F, r6, r6	\n\t"      \
		"mov	r6, %0			\n\t"
		: "=r" (level) // Output
		: // Input
		: "r6" // Clobber
	);

	return level;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::initialize()
{
	// set ROM waiting to 1 cycle
	_hardwareRegisters[__WCR] |= 0x0001;

	// check memory map before anything else
	HardwareManager::checkMemoryMap();

	//setup timer interrupts
	HardwareManager::setInterruptVectors();
	HardwareManager::setInterruptLevel(0);
	HardwareManager::setExceptionVectors();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "HARDWARE STATUS", x, y++, NULL);

	int32 auxY = y;
	int32 xDisplacement = 5;

	// print registries' status to know the call source
	Printing::text(Printing::getInstance(), "PSW:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getPSW(), x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "SP:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getStackPointer(), x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "LP:" , x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), HardwareManager::getLinkPointer(), x + xDisplacement, auxY++, 4, NULL);

	Printing::text(Printing::getInstance(), "Hardware\nRegisters", x, ++auxY, NULL);
	auxY+=2;
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
	x += 11;
	xDisplacement = 8;

	Printing::text(Printing::getInstance(), "VIP\nRegisters", x, ++auxY, NULL);
	auxY+=2;
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
	Printing::hex(Printing::getInstance(), (uint8)_vipRegisters[__BRTA], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "BRTB:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), (uint8)_vipRegisters[__BRTB], x + xDisplacement, auxY, 4, NULL);
	Printing::text(Printing::getInstance(), "BRTC:", x, ++auxY, NULL);
	Printing::hex(Printing::getInstance(), (uint8)_vipRegisters[__BRTC], x + xDisplacement, auxY, 4, NULL);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void HardwareManager::printStackStatus(int32 x, int32 y, bool resumed)
{
	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	
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

		Printing::text(Printing::getInstance(), "   STACK'S ROOM        " , x - 3, y, NULL);
		Printing::int32(Printing::getInstance(), room, x + 13, y, NULL);
	}
	else
	{
		if((__SCREEN_WIDTH_IN_CHARS) - 1 < Math::getDigitsCount(room) + 15)
		{
			x = (__SCREEN_WIDTH_IN_CHARS) - 1 - Math::getDigitsCount(room) - 11;
		}

		Printing::text(Printing::getInstance(), "   STACK'S STATUS" , x - 3, y, NULL);
		Printing::text(Printing::getInstance(), "Bss' end:" , x, ++y, NULL);
		Printing::hex(Printing::getInstance(), (int32)&_bssEnd, x + 15, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Stack Pointer:" , x, ++y, NULL);
		Printing::hex(Printing::getInstance(), sp, x + 15, y, 4, NULL);
		Printing::text(Printing::getInstance(), "Minimum Room:           " , x, ++y, NULL);
		Printing::int32(Printing::getInstance(), __STACK_HEADROOM, x + 15, y, NULL);
		Printing::text(Printing::getInstance(), "Actual Room:           " , x, ++y, NULL);
		Printing::int32(Printing::getInstance(), room, x + 15, y, NULL);
		Printing::text(Printing::getInstance(), "Overflow:           " , x, ++y, NULL);
		Printing::int32(Printing::getInstance(), __STACK_HEADROOM - room, x + 15, y, NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

