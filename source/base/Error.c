/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Error.h>
#include <Game.h>
#include <SpriteManager.h>
#include <HardwareManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Error_ATTRIBUTES				\
										\
	/* super's attributes */			\
	Object_ATTRIBUTES;					\


// define the Error
__CLASS_DEFINITION(Error);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void Error_constructor(Error this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a singleton
__SINGLETON(Error);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Error_constructor(Error this){
	
	__CONSTRUCT_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Error_destructor(Error this){

	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup the error message and lock program here
int Error_triggerException(Error this, char* string){
	
	//clear screen
    //HardwareManager_clearScreen(HardwareManager_getInstance());

	// turn on the display
    HardwareManager_displayOn(HardwareManager_getInstance());
	
	// make sure the brigtness is ok
    HardwareManager_upBrightness(HardwareManager_getInstance());
	
	Printing_writeAscii(TextureManager_getFreeBgmap(TextureManager_getInstance()));

	//print error message to screen
#define EXCEPTION_LINE	27
	//print error message to screen
	Printing_text("                                             ", 0, EXCEPTION_LINE);
	Printing_text("Exception:" , 1, EXCEPTION_LINE);
	Printing_text(string, 12, EXCEPTION_LINE);
	
	// error display message
	Printing_render(SpriteManager_getFreeLayer(SpriteManager_getInstance()), TextureManager_getFreeBgmap(TextureManager_getInstance()));

	//trap the game here	
	while(true);
	
	return false;
}

