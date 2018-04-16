/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
		Object_ATTRIBUTES																				\
		/**
		 * @var Optical					optical
		 * @brief						Optical values used in projection values
		 * @memberof					Camera
		 */																								\
		Optical optical;																				\
		/**
		 * @var Vector3D				position
		 * @brief						Camera position
		 * @memberof					Camera
		 */																								\
		Vector3D position;																				\
		/**
		 * @var Vector3D				previousPosition
		 * @brief						Backup of Camera position
		 * @memberof					Camera
		 */																								\
		Vector3D previousPosition;																		\
		/**
		 * @var Vector3D				positionBackup
		 * @brief						Backup of Camera position
		 * @memberof					Camera
		 */																								\
		Vector3D positionBackup;																		\
		/**
		 * @var CameraMovementManager	cameraMovementManager
		 * @brief						Camera position displacement manager
		 * @memberof					Camera
		 */																								\
		CameraMovementManager cameraMovementManager;													\
		/**
		 * @var CameraEffectManager		cameraEffectManager
		 * @brief						Camera effect manager
		 * @memberof					Camera
		 */																								\
		CameraEffectManager cameraEffectManager;														\
		/**
		 * @var Vector3D				focusEntityPositionDisplacement
		 * @brief						Camera position displacement
		 * @memberof					Camera
		 */																								\
		Vector3D focusEntityPositionDisplacement;														\
		/**
		 * @var Entity					focusEntity
		 * @brief						Actor to center the camera around
		 * @memberof					Camera
		 */																								\
		Entity focusEntity;																				\
		/**
		 * @var const					focusEntityPosition
		 * @brief						Position of actor to center the camera around
		 * @memberof					Camera
		 */																								\
		const Vector3D* focusEntityPosition;															\
		/**
		 * @var Vector3D				lastDisplacement
		 * @brief						World's camera's last displacement
		 * @memberof					Camera
		 */																								\
		Vector3D lastDisplacement;																		\
		/**
		 * @var Size					stageSize
		 * @brief						Stage's size in pixels
		 * @memberof					Camera
		 */																								\
		Size stageSize;																					\
		/**
		 * @var CameraFrustum			cameraFrustum
		 * @brief						Camera frustum
		 * @memberof					Camera
		 */																								\
		CameraFrustum cameraFrustum;																	\

__CLASS(Camera);

/**
 * Camera frustum
 *
 * @memberof 	Camera
 */
typedef struct CameraFrustum
{
	/// x0 frustum
	s16 x0;
	/// y0 frustum
	s16 y0;
	/// z0 frustum
	s16 z0;
	/// x1 frustum
	s16 x1;
	/// y1 frustum
	s16 y1;
	/// z1 frustum
	s16 z1;

} CameraFrustum;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

extern const Vector3D* _cameraPosition;
extern const Vector3D* _cameraPreviousPosition;
extern const Vector3D* _cameraDisplacement;
extern const CameraFrustum* _cameraFrustum;
extern const Optical* _optical;

Camera Camera_getInstance();

void Camera_capPosition(Camera this);
void Camera_destructor(Camera this);
void Camera_doneUITransform(Camera this);
void Camera_focus(Camera this, u32 checkIfFocusEntityIsMoving);
void Camera_forceDisplacement(Camera this, int flag);
CameraFrustum Camera_getCameraFrustum(Camera this);
Entity Camera_getFocusEntity(Camera this);
Vector3D Camera_getLastDisplacement(Camera this);
Optical Camera_getOptical(Camera this);
Vector3D Camera_getPosition(Camera this);
Size Camera_getStageSize(Camera this);
void Camera_move(Camera this, Vector3D translation, int cap);
void Camera_onFocusEntityDeleted(Camera this, Entity actor);
void Camera_prepareForUITransform(Camera this);
void Camera_reset(Camera this);
void Camera_resetCameraFrustum(Camera this);
void Camera_setCameraEffectManager(Camera this, CameraEffectManager cameraEffectManager);
void Camera_setCameraFrustum(Camera this, CameraFrustum cameraFrustum);
void Camera_setCameraMovementManager(Camera this, CameraMovementManager cameraMovementManager);
void Camera_setFocusEntityPositionDisplacement(Camera this, Vector3D focusEntityPositionDisplacement);
void Camera_setFocusGameEntity(Camera this, Entity focusEntity);
void Camera_setOptical(Camera this, Optical optical);
void Camera_setPosition(Camera this, Vector3D position);
void Camera_setStageSize(Camera this, Size size);
void Camera_startEffect(Camera this, int effect, ...);
void Camera_stopEffect(Camera this, int effect);
void Camera_unsetFocusEntity(Camera this);
Vector3D Camera_getFocusEntityPosition(Camera this);
Vector3D Camera_getFocusEntityPositionDisplacement(Camera this);
void Camera_print(Camera this, int x, int y);

#endif
