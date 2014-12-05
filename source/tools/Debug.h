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

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __DEBUG_TOOLS

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <GameState.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define Debug_METHODS								\
		Object_METHODS								\


// declare the virtual methods which are redefined
#define Debug_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)				\


// declare a Debug
__CLASS(Debug);


// for debugging
typedef struct ClassSizeData {
	
	char* name;
	int (*classSizeFunction)(void);
	
}ClassSizeData;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
Debug Debug_getInstance();

// class's destructor
void Debug_destructor(Debug this);

// update
void Debug_update(Debug this);

// show debug screens
void Debug_show(Debug this, GameState gameState);

// hide debug screens
void Debug_hide(Debug this);

// show previous page
void Debug_showPreviousPage(Debug this);

// show next page
void Debug_showNextPage(Debug this);

// show previous sub page
void Debug_showPreviousSubPage(Debug this);

// show next sub page
void Debug_showNextSubPage(Debug this);

// displace view to the left
void Debug_diplaceLeft(Debug this);

// displace view to the right
void Debug_diplaceRight(Debug this);

// displace view up
void Debug_diplaceUp(Debug this);

// displace view down
void Debug_diplaceDown(Debug this);

#endif

#endif /*CLOCK_H_*/
