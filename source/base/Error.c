/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Error.h>
#include <Game.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>

//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __DIMM_VALUE_1	0b01010100
#define __DIMM_VALUE_2	0b01010000
//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Error_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// define the Error
__CLASS_DEFINITION(Error, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void Error_constructor(Error this);

bool Game_isConstructed();


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(Error);


// class's constructor
static void Error_constructor(Error this)
{
	__CONSTRUCT_BASE();
}

// class's destructor
void Error_destructor(Error this)
{
	__SINGLETON_DESTROY;
}

// setup the error message and lock program here
int Error_triggerException(Error this, char* string)
{
	int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 48 / 2 ? __EXCEPTION_COLUMN : 0;
	int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	// disable timers
	TimerManager_enable(TimerManager_getInstance(), false);

	// disable timer
	TimerManager_setInterrupt(TimerManager_getInstance(), false);

	// turn on the display
    HardwareManager_displayOn(HardwareManager_getInstance());

	// make sure the brigtness is ok
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
	
	Printing_text(Printing_getInstance(), "                                                " , x, ++y + 1, NULL);
	y += 2;
	Printing_text(Printing_getInstance(), "                                                " , x, y++ + 1, NULL);
	Printing_text(Printing_getInstance(), " Message:                                       " , x, y++, NULL);

	int stringMaxLenght = __SCREEN_WIDTH / 8 - 2;
	int rowsAvailable  = __SCREEN_HEIGHT / 8 - y;
	int stringLength = strnlen(string, stringMaxLenght * rowsAvailable);
	int lines = stringLength / stringMaxLenght + (stringLength % stringMaxLenght? 1: 0);
	int line = 0;
	
	for(; line < lines; line++, string += stringMaxLenght)
	{
		char messageLine[stringLength];
		strncpy(messageLine, string, stringLength);
		
		// TODO: fix me, termination character not working
		messageLine[stringLength - 1] = (char)0;
		Printing_text(Printing_getInstance(), messageLine, x + 1, y++ + 2, NULL);
	}
	
	if(y < __SCREEN_HEIGHT / 8 - 1)
	{
		Printing_text(Printing_getInstance(), "                                             ", x, y + 3, NULL);
	}
	
	// error display message
	Printing_render(Printing_getInstance(), SpriteManager_getFreeLayer(SpriteManager_getInstance()));
//	Printing_render(Printing_getInstance(), 31);

	// dimm game
	VIP_REGS[GPLT0] = __DIMM_VALUE_1;
	VIP_REGS[GPLT1] = __DIMM_VALUE_2;
	VIP_REGS[GPLT2] = __DIMM_VALUE_1;
	VIP_REGS[GPLT3] = __GPLT3_VALUE;
	VIP_REGS[JPLT0] = __DIMM_VALUE_1;
	VIP_REGS[JPLT1] = __DIMM_VALUE_1;
	VIP_REGS[JPLT2] = __DIMM_VALUE_1;
	VIP_REGS[JPLT3] = __DIMM_VALUE_1;

	//trap the game here
	while (true);

	return false;
}