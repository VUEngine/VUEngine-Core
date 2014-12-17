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

#include <Error.h>
#include <Game.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Error_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// define the Error
__CLASS_DEFINITION(Error);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void Error_constructor(Error this);

int Game_isConstructed();


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(Error);


// class's constructor
static void Error_constructor(Error this)
{
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void Error_destructor(Error this)
{
	__SINGLETON_DESTROY(Object);
}

// setup the error message and lock program here
int Error_triggerException(Error this, char* string)
{
	int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 48 / 2 ? __EXCEPTION_COLUMN : 0;
	int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	//VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);

	//clear screen
    //HardwareManager_clearScreen(HardwareManager_getInstance());

	// disable timer
	TimerManager_setInterrupt(TimerManager_getInstance(), false);

	// turn on the display
    HardwareManager_displayOn(HardwareManager_getInstance());

	// make sure the brigtness is ok
    HardwareManager_upBrightness(HardwareManager_getInstance());

    TimerManager_enable(TimerManager_getInstance(), true);

	Printing_loadFont();

	//print error message to screen
	if (0 < y)
	{
		Printing_text("                                             ", x, y - 1);
	}

	Printing_text("Game::lastProcess:", x, y);
	Printing_text(Game_isConstructed() ? Game_getLastProcessName(Game_getInstance()) : "constructor", x + 19, y);
	Printing_text("Exception:" , x, y + 1);
	Printing_text(string, x, y + 2);

	if (y < 26)
	{
		Printing_text("                                             ", x, y + 3);
	}

	// error display message
	Printing_render(SpriteManager_getFreeLayer(SpriteManager_getInstance()));

	//trap the game here
	while (true);

	return false;
}