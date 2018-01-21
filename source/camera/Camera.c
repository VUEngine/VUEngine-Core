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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Camera.h>
#include <Optics.h>
#include <Game.h>
#include <CameraMovementManager.h>
#include <CameraEffectManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Camera
 * @extends Object
 * @ingroup screen
 */
__CLASS_DEFINITION(Camera, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Camera_constructor(Camera this);


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

const Optical* _optical = NULL;
const Vector3D* _cameraPosition = NULL;
const Vector3D* _cameraDisplacement = NULL;
const CameraFrustum* _cameraFrustum = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(Camera);

// class's constructor
static void __attribute__ ((noinline)) Camera_constructor(Camera this)
{
	ASSERT(this, "Camera::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// initialize world's camera's position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	this->positionBackup = this->position;

	// set the default camera movement manager
	this->cameraMovementManager = CameraMovementManager_getInstance();

	// set the default camera effect manager
	this->cameraEffectManager = CameraEffectManager_getInstance();

	this->focusEntityPositionDisplacement.x = 0;
	this->focusEntityPositionDisplacement.y = 0;
	this->focusEntityPositionDisplacement.z = 0;

	// clear focus actor pointer
	this->focusEntity = NULL;
	this->focusEntityPosition = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;

	// accounts for the physical (real) space between the eyes and
	// the VB's screens, whose virtual representation is the Camera instance
	this->optical.distanceEyeScreen = __PIXELS_TO_METERS(__DISTANCE_EYE_SCREEN);

	// maximum distance from the _SC to the infinite
	this->optical.maximumXViewDistancePower = __PIXELS_TO_METERS(__MAXIMUM_X_VIEW_DISTANCE_POWER);

	// maximum distance from the _SC to the infinite
	this->optical.maximumYViewDistancePower = __PIXELS_TO_METERS(__MAXIMUM_Y_VIEW_DISTANCE_POWER);

	// distance from left to right eye (depth sensation)
	this->optical.baseDistance = __PIXELS_TO_METERS(__BASE_FACTOR);

	// horizontal view point center
	this->optical.horizontalViewPointCenter = __PIXELS_TO_METERS(__HORIZONTAL_VIEW_POINT_CENTER);

	// vertical view point center
	this->optical.verticalViewPointCenter = __PIXELS_TO_METERS(__VERTICAL_VIEW_POINT_CENTER);

	// set global pointer to improve access to critical values
	_optical = &this->optical;
	_cameraPosition = &this->position;
	_cameraDisplacement = &this->lastDisplacement;
	_cameraFrustum = &this->cameraFrustum;
}

// class's destructor
void Camera_destructor(Camera this)
{
	ASSERT(this, "Camera::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

// set the movement manager
void Camera_setCameraMovementManager(Camera this, CameraMovementManager cameraMovementManager)
{
	ASSERT(this, "Camera::setCameraMovementManager: null this");

	if(this->cameraMovementManager != cameraMovementManager)
	{
		if(this->cameraMovementManager)
		{
			__DELETE(this->cameraMovementManager);
		}

		this->cameraMovementManager = cameraMovementManager;
	}
}

// set the effect manager
void Camera_setCameraEffectManager(Camera this, CameraEffectManager cameraEffectManager)
{
	ASSERT(this, "Camera::setCameraEffectManager: null this");

	if(this->cameraEffectManager != cameraEffectManager)
	{
		if(this->cameraEffectManager)
		{
			__DELETE(this->cameraEffectManager);
		}

		this->cameraEffectManager = cameraEffectManager;
	}
}

// center world's camera in function of focus actor's position
void Camera_focus(Camera this, u32 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "Camera::focus: null this");
	ASSERT(this->cameraMovementManager, "Camera::focus: null cameraMovementManager");

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif

	__VIRTUAL_CALL(CameraMovementManager, focus, this->cameraMovementManager, checkIfFocusEntityIsMoving);
}

// set the focus entity
void Camera_setFocusGameEntity(Camera this, Entity focusEntity)
{
	ASSERT(this, "Camera::setFocusEntity: null this");

	this->focusEntity = focusEntity;
	this->focusEntityPosition = NULL;

	if(focusEntity)
	{
		this->focusEntityPosition = __VIRTUAL_CALL(SpatialObject, getPosition, this->focusEntity);

		// focus now
		Camera_focus(this, false);
	}
}

// unset the focus entity
void Camera_unsetFocusEntity(Camera this)
{
	ASSERT(this, "Camera::unsetFocusEntity: null this");

	this->focusEntity = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// retrieve focus entity
Entity Camera_getFocusEntity(Camera this)
{
	ASSERT(this, "Camera::getFocusEntity: null this");

	return this->focusEntity;
}

// an actor has been deleted
void Camera_onFocusEntityDeleted(Camera this, Entity actor)
{
	ASSERT(this, "Camera::focusEntityDeleted: null this");

	if(this->focusEntity == actor)
	{
		Camera_unsetFocusEntity(this);
	}
}


// translate camera
void Camera_move(Camera this, Vector3D translation, int cap)
{
	ASSERT(this, "Camera::move: null this");

	this->lastDisplacement = translation;

	this->position.x += translation.x;
	this->position.y += translation.y;
	this->position.z += translation.z;

	if(cap)
	{
		Camera_capPosition(this);
	}
}

// translate camera
void Camera_capPosition(Camera this)
{
	ASSERT(this, "Camera::capPosition: null this");

	if(this->position.x < 0)
	{
		this->position.x = 0;
	}

	if(this->position.x + __SCREEN_WIDTH_METERS > this->stageSize.x)
	{
		this->position.x = this->stageSize.x - __SCREEN_WIDTH_METERS;
	}

	if(this->position.y < 0)
	{
		this->position.y = 0;
	}

	if(this->position.y + __SCREEN_HEIGHT_METERS > this->stageSize.y)
	{
		this->position.y = this->stageSize.y - __SCREEN_HEIGHT_METERS;
	}

	if(this->position.z < 0)
	{
		this->position.z = 0;
	}

	if(this->position.z > this->stageSize.z)
	{
		this->position.z = this->stageSize.z;
	}
}

// get camera's position
Vector3D Camera_getPosition(Camera this)
{
	ASSERT(this, "Camera::getPosition: null this");

	return this->position;
}

// set camera's position
void Camera_setPosition(Camera this, Vector3D position)
{
	ASSERT(this, "Camera::setPosition: null this");

	this->position = position;

	this->lastDisplacement.x = __1I_FIX10_6;
	this->lastDisplacement.y = __1I_FIX10_6;
	this->lastDisplacement.z = __1I_FIX10_6;

	Camera_capPosition(this);
}

// set camera's position for UI transformation
void Camera_prepareForUITransform(Camera this)
{
	ASSERT(this, "Camera::prepareForUITransform: null this");

	this->positionBackup = this->position;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// set camera's position after UI transformation
void Camera_doneUITransform(Camera this)
{
	ASSERT(this, "Camera::doneUITransform: null this");

	this->position = this->positionBackup;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// retrieve optical config structure
Optical Camera_getOptical(Camera this)
{
	ASSERT(this, "Camera::getOptical: null this");

	return this->optical;
}

// set optical config structure
void Camera_setOptical(Camera this, Optical optical)
{
	ASSERT(this, "Camera::setOptical: null this");

	this->optical = optical;
}

// set camera's position displacement
void Camera_setFocusEntityPositionDisplacement(Camera this, Vector3D focusEntityPositionDisplacement)
{
	ASSERT(this, "Camera::setPosition: null this");

	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;

	// focus now
	Camera_focus(this, false);

	// make sure that any other entity knows about the change
	Camera_forceDisplacement(this, true);
}

// retrieve last displacement
Vector3D Camera_getLastDisplacement(Camera this)
{
	ASSERT(this, "Camera::getLastDisplacement: null this");

	return this->lastDisplacement;
}

// get current stage's size
Size Camera_getStageSize(Camera this)
{
	ASSERT(this, "Camera::getStageSize: null this");

	return this->stageSize;
}

// set current stage's size
void Camera_setStageSize(Camera this, Size size)
{
	ASSERT(this, "Camera::setStageSize: null this");

	this->stageSize = size;
}

// force values as if camera is moving
void Camera_forceDisplacement(Camera this, int flag)
{
	ASSERT(this, "Camera::forceDisplacement: null this");

	this->lastDisplacement.x = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.y = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.z = flag ? __1I_FIX10_6 : 0;
}

void Camera_startEffect(Camera this, int effect, ...)
{
	ASSERT(this, "Camera::startEffect: null this");

	va_list args;
	va_start(args, effect);
	__VIRTUAL_CALL(CameraEffectManager, startEffect, this->cameraEffectManager, effect, args);
	va_end(args);
}

void Camera_stopEffect(Camera this, int effect)
{
	ASSERT(this, "Camera::stopEffect: null this");

	__VIRTUAL_CALL(CameraEffectManager, stopEffect, this->cameraEffectManager, effect);
}

void Camera_resetCameraFrustum(Camera this)
{
	ASSERT(this, "Camera::stopEffect: null this");

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.z0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	this->cameraFrustum.z1 = __SCREEN_DEPTH;
}

void Camera_setCameraFrustum(Camera this, CameraFrustum cameraFrustum)
{
	ASSERT(this, "Camera::setCameraFrustum: null this");

	this->cameraFrustum = cameraFrustum;

	if(this->cameraFrustum.x1 > __SCREEN_WIDTH)
	{
		this->cameraFrustum.x1 = __SCREEN_WIDTH;
	}

	if(this->cameraFrustum.y1 > __SCREEN_HEIGHT)
	{
		this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	}

	// 9: 2's power equal to the math type fix10_6
	if(this->cameraFrustum.z1 > (1 << (9 + __PIXELS_PER_METER_2_POWER)))
	{
		this->cameraFrustum.z1 = 1;
	}

	if(this->cameraFrustum.x0 > this->cameraFrustum.x1)
	{
		this->cameraFrustum.x0 = this->cameraFrustum.x1 - 1;
	}

	if(this->cameraFrustum.y0 > this->cameraFrustum.y1)
	{
		this->cameraFrustum.y0 = this->cameraFrustum.y1 - 1;
	}

	if(this->cameraFrustum.z0 > this->cameraFrustum.z1)
	{
		this->cameraFrustum.z0 = this->cameraFrustum.z1 - 1;
	}
}

CameraFrustum Camera_getCameraFrustum(Camera this)
{
	ASSERT(this, "Camera::getCameraFrustum: null this");

	return this->cameraFrustum;
}
