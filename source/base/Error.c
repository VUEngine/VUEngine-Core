/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Error.h>
#include <VIPManager.h>
#include <BgmapTextureManager.h>
#include <Globals.h>
#include <Game.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <VIPManager.h>
#include <TimerManager.h>
#include <Fonts.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __DIMM_VALUE_1	0x54
#define __DIMM_VALUE_2	0x50


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

bool _triggeringException = false;


uint32 _vuengineEIPC = 0;
uint32 _vuengineFEPC = 0;
uint32 _vuengineECR = 0;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Error::getInstance()
 * @memberof	Error
 * @public
 * @return		Error instance
 */


/**
 * Class constructor
 *
 * @private
 */
void Error::constructor()
{
	Base::constructor();
}

/**
 * Class destructor
 *
 * @private
 */
void Error::destructor()
{
	Base::destructor();
}

/**
 * Setup the error message and lock program here
 *
 * @param message
 * @param detail
 */
#ifndef __SHIPPING
static int32 Error::triggerException(char* message, char* detail)
{
	static bool processingException = false;

	if(processingException)
	{
		return 0;
	}

	processingException = true;

	int32 lp = _vuengineLinkPointer;
	int32 sp = _vuengineStackPointer;
	int32 eipc = _vuengineEIPC;
	int32 fepc = _vuengineFEPC;
	int32 ecr = _vuengineECR;

	int32 x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int32 y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	// disable vip interrupts
	_vipRegisters[__INTENB]= 0;
	_vipRegisters[__INTCLR] = _vipRegisters[__INTPND];

	// disable timer
	_hardwareRegisters[__TCR] &= ~(__TIMER_ENB | __TIMER_INT);

	// turn on the display
	_vipRegisters[__REST] = 0;
	_vipRegisters[__DPCTRL] = _vipRegisters[__DPSTTS] | (__SYNCE | __RE | __DISP);
	_vipRegisters[__FRMCYC] = 0;
	_vipRegisters[__XPCTRL] = _vipRegisters[__XPSTTS] | __XPEN;

	// make sure the brightness is ok
	_vipRegisters[__BRTA] = 32;
	_vipRegisters[__BRTB] = 64;
	_vipRegisters[__BRTC] = 32;

	VIPManager::setBackgroundColor(VIPManager::getInstance(), __COLOR_BLACK);

	// make sure there are fonts to show the exception
	Printing::setDebugMode(Printing::getInstance());

	//print error message to screen
	if(0 < y)
	{
		Printing::text(Printing::getInstance(), "                                             ", x, y - 1, NULL);
	}

	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08 EXCEPTION \x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" , x, y++, NULL);
	Printing::text(Printing::getInstance(), "                                                " , x, y++, NULL);
	Printing::text(Printing::getInstance(), " Last process:                                  ", x, y, NULL);
	Printing::text(Printing::getInstance(), Game::isConstructed() ? Game::getLastProcessName(Game::getInstance()) : "constructor", x + 15, y++, NULL);
	Printing::text(Printing::getInstance(), " LP:                                  " , x, y, NULL);
	Printing::hex(Printing::getInstance(), lp, x + 8, y, 8, NULL);
	Printing::text(Printing::getInstance(), " SP: 		                         " , x, ++y, NULL);
	Printing::hex(Printing::getInstance(), sp, x + 8, y, 8, NULL);

	Printing::text(Printing::getInstance(), " EIPC:                                  " , x, ++y, NULL);
	Printing::hex(Printing::getInstance(), eipc, x + 8, y, 8, NULL);
	Printing::text(Printing::getInstance(), " FEPC: 		                         " , x, ++y, NULL);
	Printing::hex(Printing::getInstance(), fepc, x + 8, y, 8, NULL);
	Printing::text(Printing::getInstance(), " ECR: 		                         " , x, ++y, NULL);
	Printing::hex(Printing::getInstance(), ecr, x + 8, y, 8, NULL);

	if(message)
	{
		Printing::text(Printing::getInstance(), "                                                " , x, ++y + 1, NULL);
		Printing::text(Printing::getInstance(), " Message:                                       " , x, ++y, NULL);

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
			Printing::text(Printing::getInstance(), "                                                " , x, ++y, NULL);
			Printing::text(Printing::getInstance(), messageLine, x + 1, y, NULL);
		}

		if(detail)
		{
			Printing::text(Printing::getInstance(), detail, x + 1, ++y, NULL);
		}

		if(y < (__SCREEN_HEIGHT_IN_CHARS) - 1)
		{
			Printing::text(Printing::getInstance(), "                                             ", x, y + 3, NULL);
		}
	}

#ifdef __SHOW_STACK_OVERFLOW_ALERT
	HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 10, 0, true);
#endif

	// Prevent VIP's interrupts
	HardwareManager::disableInterrupts();

	// error display message
	WorldAttributes* worldPointer = &_worldAttributesBaseAddress[__EXCEPTIONS_WORLD];

	worldPointer->mx = __PRINTING_BGMAP_X_OFFSET;
	worldPointer->mp = __PRINTING_BGMAP_PARALLAX_OFFSET;
	worldPointer->my = __PRINTING_BGMAP_Y_OFFSET;
	worldPointer->gx = 0;
	worldPointer->gp = 0;
	worldPointer->gy = 0;
	worldPointer->w = __SCREEN_WIDTH;
	worldPointer->h = __SCREEN_HEIGHT;
	worldPointer->head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	_worldAttributesBaseAddress[__EXCEPTIONS_WORLD - 1].head = __WORLD_END;

	// dimm game
	_vipRegisters[__GPLT0] = 0xE4;
	_vipRegisters[__GPLT1] = __DIMM_VALUE_2;
	_vipRegisters[__GPLT2] = __DIMM_VALUE_1;
	_vipRegisters[__GPLT3] = __DIMM_VALUE_1;
	_vipRegisters[__JPLT0] = __DIMM_VALUE_1;
	_vipRegisters[__JPLT1] = __DIMM_VALUE_1;
	_vipRegisters[__JPLT2] = __DIMM_VALUE_1;
	_vipRegisters[__JPLT3] = __DIMM_VALUE_1;

	// trap the game here
	while(true);

	return false;
}
#endif

static void Error::zeroDivisionException()
{
#ifndef __SHIPPING
	uint32 eipc = 0;
	// Save EIPC
    asm("					\n\t"      \
		"stsr	eipc, r10	\n\t"      \
		"mov	r10, %[eipc]	\n\t"
    : [eipc] "=r" (eipc)
    : // No Input
	: "r10" // regs used
    );

	_vuengineEIPC = eipc;

	Error::triggerException("Zero division", NULL);
#endif
}

static void Error::invalidOpcodeException()
{
#ifndef __SHIPPING

	asm(" mov sp,%0  ": "=r" (_vuengineStackPointer));
	asm(" mov lp,%0  ": "=r" (_vuengineLinkPointer));

	uint32 eipc = 0;
	// Save EIPC
    asm("						\n\t"      \
		"stsr	eipc, r10		\n\t"      \
		"mov	r10, %[eipc]	\n\t"
    : [eipc] "=r" (eipc)
    : // No Input
	: "r10" // regs used
    );

	uint32 fepc = 0;
	// Save FEPC
    asm("						\n\t"      \
		"stsr	fepc, r11		\n\t"      \
		"mov	r11, %[fepc]	\n\t"
    : [fepc] "=r" (fepc)
    : // No Input
	: "r11" // regs used
    );

	uint32 ecr = 0;
	// Save ECR
    asm("						\n\t"      \
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