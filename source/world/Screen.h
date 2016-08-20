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

#ifndef SCREEN_H_
#define SCREEN_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>
#include <ScreenMovementManager.h>
#include <ScreenEffectManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Screen_METHODS(ClassName)																		\
        Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Screen_SET_VTABLE(ClassName)																	\
        Object_SET_VTABLE(ClassName)																	\

#define Screen_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* optic values used in projection values */													\
        Optical optical;																				\
        /* screen position */																			\
        VBVec3D position;																				\
        VBVec3D positionBackup;																			\
        /* screen position displacement manager */														\
        ScreenMovementManager screenMovementManager;													\
	    /* screen effect manager */																		\
    	ScreenEffectManager screenEffectManager;														\
        /* screen position displacement */																\
        VBVec3D focusEntityPositionDisplacement;														\
        /* actor to center the screen around */															\
        InGameEntity focusInGameEntity;																	\
        /* world's screen's last displacement */														\
        VBVec3D lastDisplacement;																		\
        /* stage's size in pixels */																	\
        Size stageSize;																					\

// declare a Screen
__CLASS(Screen);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Screen Screen_getInstance();

void Screen_destructor(Screen this);
void Screen_setScreenMovementManager(Screen this, ScreenMovementManager screenMovementManager);
void Screen_setScreenEffectManager(Screen this, ScreenEffectManager screenEffectManager);
void Screen_focus(Screen this, u8 checkIfFocusEntityIsMoving);
Optical Screen_getOptical(Screen this);
void Screen_setOptical(Screen this, Optical optical);
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity);
void Screen_unsetFocusInGameEntity(Screen this);
InGameEntity Screen_getFocusInGameEntity(Screen this);
void Screen_onFocusEntityDeleted(Screen this, InGameEntity actor);
void Screen_capPosition(Screen this);
void Screen_move(Screen this, VBVec3D translation, int cap);
VBVec3D Screen_getPosition(Screen this);
void Screen_setPosition(Screen this, VBVec3D position);
void Screen_prepareForUITransform(Screen this);
void Screen_doneUITransform(Screen this);
void Screen_setFocusEntityPositionDisplacement(Screen this, VBVec3D focusEntityPositionDisplacement);
VBVec3D Screen_getLastDisplacement(Screen this);
Size Screen_getStageSize(Screen this);
void Screen_setStageSize(Screen this, Size size);
void Screen_forceDisplacement(Screen this, int flag);
void Screen_startEffect(Screen this, int effect, ...);
void Screen_stopEffect(Screen this, int effect);


#endif
