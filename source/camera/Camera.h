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

#ifndef CAMERA_H_
#define CAMERA_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <Entity.h>
#include <CameraMovementManager.h>
#include <CameraEffectManager.h>


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
#define Camera_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Camera_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\

#define Camera_ATTRIBUTES																				\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* optic values used in projection values */													\
		Optical optical;																				\
		/* camera position */																			\
		Vector3D position;																				\
		Vector3D positionBackup;																			\
		/* camera position displacement manager */														\
		CameraMovementManager cameraMovementManager;													\
		/* camera effect manager */																		\
		CameraEffectManager cameraEffectManager;														\
		/* camera position displacement */																\
		Vector3D focusEntityPositionDisplacement;														\
		/* actor to center the camera around */															\
		Entity focusEntity;																				\
		const Vector3D* focusEntityPosition;																\
		/* world's camera's last displacement */														\
		Vector3D lastDisplacement;																		\
		/* stage's size in pixels */																	\
		Size stageSize;																					\
		/* camera frustum */																			\
		CameraFrustum cameraFrustum;																	\

// declare a Camera
__CLASS(Camera);

typedef struct CameraFrustum
{
	s16 x0;
	s16 y0;
	s16 x1;
	s16 y1;
	s16 z0;
	s16 z1;

}CameraFrustum;

//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

extern const Vector3D* _cameraPosition;
extern const Vector3D* _cameraDisplacement;
extern const CameraFrustum* _cameraFrustum;
extern const Optical* _optical;


Camera Camera_getInstance();

void Camera_destructor(Camera this);
void Camera_setCameraMovementManager(Camera this, CameraMovementManager cameraMovementManager);
void Camera_setCameraEffectManager(Camera this, CameraEffectManager cameraEffectManager);
void Camera_focus(Camera this, u32 checkIfFocusEntityIsMoving);
Optical Camera_getOptical(Camera this);
void Camera_setOptical(Camera this, Optical optical);
void Camera_setFocusGameEntity(Camera this, Entity focusEntity);
void Camera_unsetFocusEntity(Camera this);
Entity Camera_getFocusEntity(Camera this);
void Camera_onFocusEntityDeleted(Camera this, Entity actor);
void Camera_capPosition(Camera this);
void Camera_move(Camera this, Vector3D translation, int cap);
Vector3D Camera_getPosition(Camera this);
void Camera_setPosition(Camera this, Vector3D position);
void Camera_prepareForUITransform(Camera this);
void Camera_doneUITransform(Camera this);
void Camera_setFocusEntityPositionDisplacement(Camera this, Vector3D focusEntityPositionDisplacement);
Vector3D Camera_getLastDisplacement(Camera this);
Size Camera_getStageSize(Camera this);
void Camera_setStageSize(Camera this, Size size);
void Camera_forceDisplacement(Camera this, int flag);
void Camera_startEffect(Camera this, int effect, ...);
void Camera_stopEffect(Camera this, int effect);
void Camera_resetCameraFrustum(Camera this);
void Camera_setCameraFrustum(Camera this, CameraFrustum cameraFrustum);
CameraFrustum Camera_getCameraFrustum(Camera this);

#endif
