/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef SCREEN_MOVEMENT_MANAGER_H_
#define SCREEN_MOVEMENT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>

//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

enum ScreenFX
{
	kFadeIn = 0,
	kFadeOut,
	kScreenLastFX
};

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ScreenMovementManager_METHODS											\
    	Object_METHODS															\
		__VIRTUAL_DEC(positione);												\
		__VIRTUAL_DEC(startEffect);												\
		__VIRTUAL_DEC(stopEffect);												\

// declare the virtual methods which are redefined
#define ScreenMovementManager_SET_VTABLE(ClassName)								\
    	Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, ScreenMovementManager, positione);				\
		__VIRTUAL_SET(ClassName, ScreenMovementManager, startEffect);			\
		__VIRTUAL_SET(ClassName, ScreenMovementManager, stopEffect);			\

#define ScreenMovementManager_ATTRIBUTES										\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// declare a ScreenMovementManager
__CLASS(ScreenMovementManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ScreenMovementManager ScreenMovementManager_getInstance();

void ScreenMovementManager_constructor(ScreenMovementManager this);
void ScreenMovementManager_destructor(ScreenMovementManager this);
void ScreenMovementManager_positione(ScreenMovementManager this, u8 checkIfFocusEntityIsMoving);
void ScreenMovementManager_startEffect(ScreenMovementManager this, int effect, int duration);
void ScreenMovementManager_stopEffect(ScreenMovementManager this, int effect);

#endif