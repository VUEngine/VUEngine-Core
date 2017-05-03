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

#ifndef SCREEN_H_
#define SCREEN_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>
#include <ScreenMovementManager.h>
#include <ScreenEffectManager.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
		const VBVec3D* focusInGameEntityPosition;														\
		/* world's screen's last displacement */														\
		VBVec3D lastDisplacement;																		\
		/* stage's size in pixels */																	\
		Size stageSize;																					\
		/* camera frustum */																			\
		CameraFrustum cameraFrustum;																	\

// declare a Screen
__CLASS(Screen);

typedef struct CameraFrustum
{
	u16 x0;
	u16 y0;
	u16 x1;
	u16 y1;

}CameraFrustum;

//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

extern const VBVec3D* _screenPosition;
extern const VBVec3D* _screenDisplacement;
extern const CameraFrustum* _cameraFrustum;
extern const Optical* _optical;


Screen Screen_getInstance();

void Screen_destructor(Screen this);
void Screen_setScreenMovementManager(Screen this, ScreenMovementManager screenMovementManager);
void Screen_setScreenEffectManager(Screen this, ScreenEffectManager screenEffectManager);
void Screen_focus(Screen this, u32 checkIfFocusEntityIsMoving);
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
void Screen_resetCameraFrustum(Screen this);
void Screen_setCameraFrustum(Screen this, CameraFrustum cameraFrustum);
CameraFrustum Screen_getCameraFrustum(Screen this);

#endif
