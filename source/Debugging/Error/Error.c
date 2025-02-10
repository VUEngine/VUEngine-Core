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

#include <string.h>

#include <BgmapTextureManager.h>
#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Printer.h>
#include <Sprite.h>
#include <TimerManager.h>
#include <VIPManager.h>

#include "Error.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static WorldAttributes* const _worldAttributesBaseAddress = (WorldAttributes*)__WORLD_SPACE_BASE_ADDRESS;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __DIMM_VALUE_1	0x54
#define __DIMM_VALUE_2	0x50

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool _triggeringException = false;
uint32 _vuengineEIPC = 0;
uint32 _vuengineFEPC = 0;
uint32 _vuengineECR = 0;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Error::triggerException(char* message __attribute__((unused)), char* detail __attribute__((unused)))
{
#ifndef __SHIPPING
	static bool processingException = false;

	if(processingException)
	{
		return;
	}

	processingException = true;

	int32 lp = _vuengineLinkPointer;
	int32 sp = _vuengineStackPointer;
	int32 eipc = _vuengineEIPC;
	int32 fepc = _vuengineFEPC;
	int32 ecr = _vuengineECR;

	int32 x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int32 y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	// Disable timer
	TimerManager::disable();

	// Turn on the display
	VIPManager::allowInterrupts(VIPManager::getInstance(), false);
	VIPManager::startDisplaying(VIPManager::getInstance());
	VIPManager::startDrawing(VIPManager::getInstance());

	// Disable interrupts
	HardwareManager::disableInterrupts();

	// Make sure the brightness is ok
	Brightness brightness =
	{
		32,
		64,
		32
	};
	
	VIPManager::configureBrightness(&brightness);

	VIPManager::configureBackgroundColor(__COLOR_BLACK);

	// Make sure there are fonts to show the exception
	Printer::setDebugMode();

	//print error message to screen
	if(0 < y)
	{
		Printer::text("                                             ", x, y - 1, NULL);
	}

	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08 EXCEPTION "
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" , x, y++, NULL
	);

	Printer::text("                                                " , x, y++, NULL);
	Printer::text(" LP:                                  " , x, y, NULL);
	Printer::hex(lp, x + 8, y, 8, NULL);
	Printer::text(" SP: 		                         " , x, ++y, NULL);
	Printer::hex(sp, x + 8, y, 8, NULL);

	Printer::text(" EIPC:                                  " , x, ++y, NULL);
	Printer::hex(eipc, x + 8, y, 8, NULL);
	Printer::text(" FEPC: 		                         " , x, ++y, NULL);
	Printer::hex(fepc, x + 8, y, 8, NULL);
	Printer::text(" ECR: 		                         " , x, ++y, NULL);
	Printer::hex(ecr, x + 8, y, 8, NULL);

	if(message)
	{
		Printer::text("                                                " , x, ++y + 1, NULL);
		Printer::text(" Message:                                       " , x, ++y, NULL);

		int32 stringMaxLength = (__SCREEN_WIDTH_IN_CHARS) - 2;
		int32 rowsAvailable  = (__SCREEN_HEIGHT_IN_CHARS) - y;
		int32 stringLength = strnlen(message, stringMaxLength * rowsAvailable) + 1;
		int32 lines = stringLength / stringMaxLength + (stringLength % stringMaxLength ? 1 : 0);
		int32 line = 0;

		for(; line < lines; line++, message += stringMaxLength)
		{
			char messageLine[stringLength];
			strncpy(messageLine, message, stringLength);

			// TODO: fix me, termination character not working
			messageLine[stringLength - 1] = (char)0;
			Printer::text("                                                " , x, ++y, NULL);
			Printer::text(messageLine, x + 1, y, NULL);
		}

		if(detail)
		{
			Printer::text(detail, x + 1, ++y, NULL);
		}

		if(y < (__SCREEN_HEIGHT_IN_CHARS) - 1)
		{
			Printer::text("                                             ", x, y + 3, NULL);
		}
	}

#ifdef __SHOW_STACK_OVERFLOW_ALERT
	HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 10, 0, true);
#endif

	// Prevent VIP's interrupts
	HardwareManager::disableInterrupts();

	// Error display message
	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[__EXCEPTIONS_WORLD];

	worldPointer->mx = __PRINTING_BGMAP_X_OFFSET;
	worldPointer->mp = __PRINTING_BGMAP_PARALLAX_OFFSET;
	worldPointer->my = __PRINTING_BGMAP_Y_OFFSET;
	worldPointer->gx = 0;
	worldPointer->gp = 0;
	worldPointer->gy = 0;
	worldPointer->w = __SCREEN_WIDTH;
	worldPointer->h = __SCREEN_HEIGHT;
	worldPointer->head = 
		__WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	_worldAttributesBaseAddress[__EXCEPTIONS_WORLD - 1].head = __WORLD_END;

	// Dimm game
	PaletteConfig paletteConfig =
	{
		{0xE4, __DIMM_VALUE_2, __DIMM_VALUE_2, __DIMM_VALUE_2},
		{__DIMM_VALUE_2, __DIMM_VALUE_2, __DIMM_VALUE_2, __DIMM_VALUE_2}
	};
	
	VIPManager::configurePalettes(&paletteConfig);

	// Trap the game here
	while(true);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Error::zeroDivisionException()
{
#ifndef __SHIPPING
	uint32 eipc = 0;
	// Save EIPC
    asm
	(
		"stsr	eipc, r10		\n\t"      \
		"mov	r10, %[eipc]	\n\t"
		: [eipc] "=r" (eipc)
		: // No Input
		: "r10" // regs used
    );

	_vuengineEIPC = eipc;

	Error::triggerException("Zero division", NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Error::invalidOpcodeException()
{
#ifndef __SHIPPING

	asm
	(
		"mov	sp, %0"
		: "=r" (_vuengineStackPointer)
	);

	asm
	(
		"mov lp,%0  "
		: "=r" (_vuengineLinkPointer)
	);

	uint32 eipc = 0;
	// Save EIPC
    asm
	(
		"stsr	eipc, r10		\n\t"      \
		"mov	r10, %[eipc]	\n\t"
		: [eipc] "=r" (eipc)
		: // No Input
		: "r10" // regs used
    );

	uint32 fepc = 0;
	// Save FEPC
    asm
	(
		"stsr	fepc, r11		\n\t"      \
		"mov	r11, %[fepc]	\n\t"
		: [fepc] "=r" (fepc)
		: // No Input
		: "r11" // regs used
    );

	uint32 ecr = 0;
	// Save ECR
    asm
	(
		"stsr	ecr, r12		\n\t"      \
		"mov	r12, %[ecr]		\n\t"
		: [ecr] "=r" (ecr)
		: // No Input
		: "r12" // regs used
    );

	_vuengineEIPC = eipc;
	_vuengineFEPC = fepc;
	_vuengineECR = ecr;

	Error::triggerException("Invalid opcode", NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Error::floatingPointException()
{
#ifndef __SHIPPING

	asm
	(
		"mov sp,%0  "
		: "=r" (_vuengineStackPointer)
	);

	asm
	(
		"mov lp,%0  "
		: "=r" (_vuengineLinkPointer)
	);

	uint32 eipc = 0;
	// Save EIPC
    asm
	(
		"stsr	eipc, r10		\n\t"      \
		"mov	r10, %[eipc]	\n\t"
		: [eipc] "=r" (eipc)
		: // No Input
		: "r10" // regs used
    );

	uint32 fepc = 0;
	// Save FEPC
    asm
	(
		"stsr	fepc, r11		\n\t"      \
		"mov	r11, %[fepc]	\n\t"
		: [fepc] "=r" (fepc)
		: // No Input
		: "r11" // regs used
    );

	uint32 ecr = 0;
	// Save ECR
    asm
	(
		"stsr	ecr, r12		\n\t"      \
		"mov	r12, %[ecr]		\n\t"
		: [ecr] "=r" (ecr)
		: // No Input
		: "r12" // regs used
    );

	_vuengineEIPC = eipc;
	_vuengineFEPC = fepc;
	_vuengineECR = ecr;

	Error::triggerException("Floating point exception", NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
