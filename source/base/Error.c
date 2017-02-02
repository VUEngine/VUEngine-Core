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

#include <string.h>
#include <Error.h>
#include <VIPManager.h>
#include <Globals.h>
#include <Game.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>
#include <Printing.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __DIMM_VALUE_1	0x54
#define __DIMM_VALUE_2	0x50


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Error_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\

/**
 * @class	Error
 * @extends Object
 */
__CLASS_DEFINITION(Error, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Error_constructor(Error this);

bool Game_isConstructed();


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Error_getInstance()
 * @memberof	Error
 * @public
 *
 * @return		Error instance
 */
__SINGLETON(Error);

/**
 * Class constructor
 *
 * @memberof	Error
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) Error_constructor(Error this)
{
	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	Error
 * @public
 *
 * @param this	Function scope
 */
void Error_destructor(Error this)
{
	__SINGLETON_DESTROY;
}

/**
 * Setup the error message and lock program here
 *
 * @memberof		Error
 * @public
 *
 * @param this		Function scope
 * @param message
 * @param detail
 */
int Error_triggerException(Error this __attribute__ ((unused)), char* message, char* detail)
{
	int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	// disable vip interrupts
	VIPManager_disableInterrupts(VIPManager_getInstance());

	// disable timers
	TimerManager_enable(TimerManager_getInstance(), false);

	// disable timer
	TimerManager_setInterrupt(TimerManager_getInstance(), false);

	// turn on the display
	HardwareManager_displayOn(HardwareManager_getInstance());

	// make sure the brightness is ok
	HardwareManager_upBrightness(HardwareManager_getInstance());

	//print error message to screen
	if(0 < y)
	{
		Printing_text(Printing_getInstance(), "                                             ", x, y - 1, NULL);
	}

	Printing_text(Printing_getInstance(), "                   EXCEPTION                    " , x, y++, NULL);
	Printing_text(Printing_getInstance(), "                                                " , x, y++, NULL);
	Printing_text(Printing_getInstance(), " Last process:                                  ", x, y, NULL);
	Printing_text(Printing_getInstance(), Game_isConstructed() ? Game_getLastProcessName(Game_getInstance()) : "constructor", x + 15, y++, NULL);
	Printing_text(Printing_getInstance(), " LP:                                  " , x, y, NULL);
	Printing_hex(Printing_getInstance(), _lp, x + 5, y, 8, NULL);
	Printing_text(Printing_getInstance(), " SP: 		                         " , x, ++y, NULL);
	Printing_hex(Printing_getInstance(), _sp, x + 5, y, 8, NULL);

	if(message)
	{
		Printing_text(Printing_getInstance(), "                                                " , x, ++y + 1, NULL);
		Printing_text(Printing_getInstance(), " Message:                                       " , x, ++y, NULL);

		int stringMaxLenght = (__SCREEN_WIDTH >> 3) - 2;
		int rowsAvailable  = (__SCREEN_HEIGHT >> 3) - y;
		int stringLength = strnlen(message, stringMaxLenght * rowsAvailable) + 1;
		int lines = stringLength / stringMaxLenght + (stringLength % stringMaxLenght ? 1 : 0);
		int line = 0;

		for(; line < lines; line++, message += stringMaxLenght)
		{
			char messageLine[stringLength];
			strncpy(messageLine, message, stringLength);

			// TODO: fix me, termination character not working
			messageLine[stringLength - 1] = (char)0;
			Printing_text(Printing_getInstance(), "                                                " , x, ++y, NULL);
			Printing_text(Printing_getInstance(), messageLine, x + 1, y, NULL);
		}

		if(detail)
		{
			Printing_text(Printing_getInstance(), detail, x + 1, ++y, NULL);
		}

		if(y < (__SCREEN_HEIGHT >> 3) - 1)
		{
			Printing_text(Printing_getInstance(), "                                             ", x, y + 3, NULL);
		}
	}

#ifdef __ALERT_STACK_OVERFLOW
	HardwareManager_printStackStatus(HardwareManager_getInstance(), (__SCREEN_WIDTH >> 3) - 10, 0, true);
#endif

	// error display message
	Printing_render(Printing_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));

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
