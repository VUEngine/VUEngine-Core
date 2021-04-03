/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#ifndef __RELEASE
static int Error::triggerException(char* message, char* detail)
{
	static bool processingException = false;

	if(processingException)
	{
		return 0;
	}

	processingException = true;

	int lp = _vuengineLinkPointer;
	int sp = _vuengineStackPointer;
	int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

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
	Printing::hex(Printing::getInstance(), lp, x + 5, y, 8, NULL);
	Printing::text(Printing::getInstance(), " SP: 		                         " , x, ++y, NULL);
	Printing::hex(Printing::getInstance(), sp, x + 5, y, 8, NULL);

	if(message)
	{
		Printing::text(Printing::getInstance(), "                                                " , x, ++y + 1, NULL);
		Printing::text(Printing::getInstance(), " Message:                                       " , x, ++y, NULL);

		int stringMaxLenght = (__SCREEN_WIDTH_IN_CHARS) - 2;
		int rowsAvailable  = (__SCREEN_HEIGHT_IN_CHARS) - y;
		int stringLength = strnlen(message, stringMaxLenght * rowsAvailable) + 1;
		int lines = stringLength / stringMaxLenght + (stringLength % stringMaxLenght ? 1 : 0);
		int line = 0;

		for(; line < lines; line++, message += stringMaxLenght)
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
	u16 eipc;
	// Save EIPC
    asm("					\n\t"      \
		"stsr	eipc, r10	\n\t"      \
		"mov	r10, %0	\n\t"
    : // No Output
    : "r" (eipc)
	: "r10" // regs used
    );

	_vuengineLinkPointer = eipc;

	Error::triggerException("Zero division", NULL);
}