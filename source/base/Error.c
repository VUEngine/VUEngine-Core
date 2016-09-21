/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __DIMM_VALUE_1	0x54
#define __DIMM_VALUE_2	0x50


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Error_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\

// define the Error
__CLASS_DEFINITION(Error, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Error_constructor(Error this);

bool Game_isConstructed();


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(Error);

// class's constructor
static void __attribute__ ((noinline)) Error_constructor(Error this)
{
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void Error_destructor(Error this)
{
	__SINGLETON_DESTROY;
}

// setup the error message and lock program here
int Error_triggerException(Error this __attribute__ ((unused)), char* message, char* detail)
{
	int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

    // disable VIP interrutps
    VIPManager_disableInterrupt(VIPManager_getInstance());

	// disable timers
	TimerManager_enable(TimerManager_getInstance(), false);

	// disable timer
	TimerManager_setInterrupt(TimerManager_getInstance(), false);

	// turn on the display
    HardwareManager_displayOn(HardwareManager_getInstance());

	// make sure the brightness is ok
    HardwareManager_upBrightness(HardwareManager_getInstance());

	Printing_loadFonts(Printing_getInstance());

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
	Printing_hex(Printing_getInstance(), _lp, x + 5, y, NULL);
	Printing_text(Printing_getInstance(), " SP: 		                         " , x, ++y, NULL);
	Printing_hex(Printing_getInstance(), _sp, x + 5, y, NULL);

	if(message)
	{
		Printing_text(Printing_getInstance(), "                                                " , x, ++y + 1, NULL);
		Printing_text(Printing_getInstance(), " Message:                                       " , x, ++y, NULL);

		int stringMaxLenght = (__SCREEN_WIDTH >> 3) - 2;
		int rowsAvailable  = (__SCREEN_HEIGHT >> 3) - y;
		int stringLength = strnlen(message, stringMaxLenght * rowsAvailable) + 1;
		int lines = stringLength / stringMaxLenght + (stringLength % stringMaxLenght? 1: 0);
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
