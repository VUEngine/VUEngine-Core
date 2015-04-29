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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>
#include <ScreenMovementManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0

// delay between screen movements during shaking effect (in ms)
#define SHAKE_DELAY		100

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Screen_METHODS															\
    Object_METHODS																\


// declare the virtual methods which are redefined
#define Screen_SET_VTABLE(ClassName)											\
    Object_SET_VTABLE(ClassName)												\
    __VIRTUAL_SET(ClassName, Screen, handleMessage);							\

#define Screen_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* optic values used in projection values */								\
	Optical optical;															\
																				\
	/* screen position */														\
	VBVec3D position;															\
																				\
	/* screen position displacement manager */									\
	ScreenMovementManager screenMovementManager;								\
																				\
	/* screen position displacement */											\
	VBVec3D focusEntityPositionDisplacement;									\
																				\
	/* actor to center the screen around */										\
	InGameEntity focusInGameEntity;												\
																				\
	/* temporary variable to hold the focus entity during shaking fx */			\
	InGameEntity tempFocusInGameEntity;											\
																				\
	/* last offset set by shake function */										\
	VBVec3D lastShakeOffset;													\
																				\
	/* time left in current shaking fx (in ms) */								\
	u16 shakeTimeLeft;															\
																				\
	/* world's screen's last displacement */									\
	VBVec3D lastDisplacement;													\
																				\
	/* stage's size in pixels */												\
	Size stageSize;																\

// declare a Screen
__CLASS(Screen);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Screen Screen_getInstance();

void Screen_destructor(Screen this);
void Screen_setScreenMovementManager(Screen this, ScreenMovementManager screenMovementManager);
void Screen_positione(Screen this, u8 checkIfFocusEntityIsMoving);
bool Screen_handleMessage(Screen this, Telegram telegram);
Optical Screen_getOptical(Screen this);
void Screen_setOptical(Screen this, Optical optical);
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity);
void Screen_unsetFocusInGameEntity(Screen this);
InGameEntity Screen_getFocusInGameEntity(Screen this);
void Screen_onFocusEntityDeleted(Screen this, InGameEntity actor);
void Screen_move(Screen this, VBVec3D translation, int cap);
VBVec3D Screen_getPosition(Screen this);
void Screen_setPosition(Screen this, VBVec3D position);
void Screen_setFocusEntityPositionDisplacement(Screen this, VBVec3D focusEntityPositionDisplacement);
VBVec3D Screen_getLastDisplacement(Screen this);
Size Screen_getStageSize(Screen this);
void Screen_setStageSize(Screen this, Size size);
void Screen_forceDisplacement(Screen this, int flag);
void Screen_FXFadeIn(Screen this, int wait);
void Screen_FXFadeOut(Screen this, int wait);
void Screen_FXShakeStart(Screen this, u16 duration);
void Screen_FXShakeStop(Screen this);


#endif