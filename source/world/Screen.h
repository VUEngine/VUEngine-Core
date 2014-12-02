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

#ifndef SCREEN_H_
#define SCREEN_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <InGameEntity.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define Screen_METHODS								\
		Object_METHODS								\


// declare the virtual methods which are redefined
#define Screen_SET_VTABLE(ClassName)							\
		Object_SET_VTABLE(ClassName)							\
	

// declare a Screen, which holds the objects in a game world
__CLASS(Screen);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
Screen Screen_getInstance();

// class's destructo
void Screen_destructor(Screen this);

// positione the screen
void Screen_positione(Screen this);

// set the focus entity
// set the focus entity
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity);

// retrieve focus entity
InGameEntity Screen_getFocusInGameEntity(Screen this);

// inform the screen I'm being removed
void Screen_focusEntityDeleted(Screen this, InGameEntity actor);

// translate screen
void Screen_move(Screen this, VBVec3D translation, int cap);

// get screen's position
VBVec3D Screen_getPosition(Screen this);

// set screen's position
void Screen_setPosition(Screen this, VBVec3D position);

// retrieve last displacement
VBVec3D Screen_getLastDisplacement(Screen this);

// set current stage's size
void Screen_setStageSize(Screen this, Size size);

// create a fade delay
void Screen_FXFadeIn(Screen this, int wait);

// create a fade delay
void Screen_FXFadeOut(Screen this, int wait);


#endif

