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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Camera.h>
#include <Optics.h>
#include <Game.h>
#include <CameraMovementManager.h>
#include <CameraEffectManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Camera
 * @extends Object
 * @ingroup camera
 */
__CLASS_DEFINITION(Camera, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Camera_constructor(Camera this);
static Vector3D Camera_getCappedPosition(Camera this, Vector3D position);


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

const Optical* _optical = NULL;
const Vector3D* _cameraPosition = NULL;
const Vector3D* _cameraPreviousPosition = NULL;
const Vector3D* _cameraDisplacement = NULL;
const CameraFrustum* _cameraFrustum = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Camera_getInstance()
 * @memberof	Camera
 * @public
 *
 * @return		Camera instance
 */
__SINGLETON(Camera);

/**
 * Class constructor
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) Camera_constructor(Camera this)
{
	ASSERT(this, "Camera::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// initialize world's camera's position
	this->position = (Vector3D){0, 0, 0};

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

	this->position = (Vector3D){0, 0, 0};
	this->previousPosition = (Vector3D){0, 0, 0};
	this->positionBackup = (Vector3D){0, 0, 0};
	this->lastDisplacement = (Vector3D){0, 0, 0};

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;

	PixelOptical pixelOptical =
    {
    	__MAXIMUM_X_VIEW_DISTANCE,				// maximum distance from the screen to the infinite
    	__MAXIMUM_Y_VIEW_DISTANCE,				// maximum distance from the screen to the infinite
    	__DISTANCE_EYE_SCREEN,
    	__BASE_FACTOR,							// distance from left to right eye (depth perception)
    	__HORIZONTAL_VIEW_POINT_CENTER,			// horizontal View point center
    	__VERTICAL_VIEW_POINT_CENTER,			// vertical View point center
    	__SCALING_MODIFIER_FACTOR,				// scaling factor for sprite resizing
    };

	Camera_setOptical(this, Optical_getFromPixelOptical(pixelOptical));

	// set global pointer to improve access to critical values
	_optical = &this->optical;
	_cameraPosition = &this->position;
	_cameraPreviousPosition = &this->previousPosition;
	_cameraDisplacement = &this->lastDisplacement;
	_cameraFrustum = &this->cameraFrustum;
}

/**
 * Class destructor
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_destructor(Camera this)
{
	ASSERT(this, "Camera::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Set the movement manager
 *
 * @memberof					Camera
 * @public
 *
 * @param this					Function scope
 * @param cameraMovementManager	The CameraMovementManager
 */
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

/**
 * Set the effect manager
 *
 * @memberof					Camera
 * @public
 *
 * @param this					Function scope
 * @param cameraEffectManager	The CameraEffectManager
 */
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

/**
 * Center world's camera in function of focus actor's position
 *
 * @memberof							Camera
 * @public
 *
 * @param this							Function scope
 * @param checkIfFocusEntityIsMoving	The CameraEffectManager
 */
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

	 CameraMovementManager_focus(this->cameraMovementManager, checkIfFocusEntityIsMoving);

#ifdef __PRINT_CAMERA_STATUS
	Camera_print(this, 1, 1);
#endif
}

/**
 * Set the focus entity
 *
 * @memberof			Camera
 * @public
 *
 * @param this			Function scope
 * @param focusEntity	The CameraEffectManager
 */
void Camera_setFocusGameEntity(Camera this, Entity focusEntity)
{
	ASSERT(this, "Camera::setFocusEntity: null this");

	this->focusEntity = focusEntity;
	this->focusEntityPosition = NULL;

	if(focusEntity)
	{
		this->focusEntityPosition =  SpatialObject_getPosition(this->focusEntity);

		// focus now
		Camera_focus(this, false);
	}
}

/**
 * Unset the focus entity
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_unsetFocusEntity(Camera this)
{
	ASSERT(this, "Camera::unsetFocusEntity: null this");

	this->focusEntity = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

/**
 * Retrieve focus entity
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Focus Entity
 */
Entity Camera_getFocusEntity(Camera this)
{
	ASSERT(this, "Camera::getFocusEntity: null this");

	return this->focusEntity;
}

/**
 * An actor has been deleted
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 * @param actor	Entity that has been deleted
 */
void Camera_onFocusEntityDeleted(Camera this, Entity actor)
{
	ASSERT(this, "Camera::focusEntityDeleted: null this");

	if(this->focusEntity == actor)
	{
		Camera_unsetFocusEntity(this);
	}
}

/**
 * Translate camera
 *
 * @memberof			Camera
 * @public
 *
 * @param this			Function scope
 * @param translation
 * @param cap
 */
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

/**
 * Cap position
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return position	Capped Vector3d
 */
static Vector3D Camera_getCappedPosition(Camera this, Vector3D position)
{
	ASSERT(this, "Camera::getCappedPosition: null this");

	if(position.x < 0)
	{
		position.x = 0;
	}

	if(position.x + __SCREEN_WIDTH_METERS > this->stageSize.x)
	{
		position.x = this->stageSize.x - __SCREEN_WIDTH_METERS;
	}

	if(position.y < 0)
	{
		position.y = 0;
	}

	if(position.y + __SCREEN_HEIGHT_METERS > this->stageSize.y)
	{
		position.y = this->stageSize.y - __SCREEN_HEIGHT_METERS;
	}

	if(position.z < 0)
	{
		position.z = 0;
	}

	if(position.z > this->stageSize.z)
	{
		position.z = this->stageSize.z;
	}

	return position;
}

/**
 * Translate camera
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_capPosition(Camera this)
{
	ASSERT(this, "Camera::capPosition: null this");

	this->position = Camera_getCappedPosition(this, this->position);
}

/**
 * Get camera's position
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Camera position
 */
Vector3D Camera_getPosition(Camera this)
{
	ASSERT(this, "Camera::getPosition: null this");

	return this->position;
}

/**
 * Set camera's position
 *
 * @memberof		Camera
 * @public
 *
 * @param this		Function scope
 * @param position	Camera position
 */
void Camera_setPosition(Camera this, Vector3D position)
{
	ASSERT(this, "Camera::setPosition: null this");

	position = Camera_getCappedPosition(this, position);

	this->lastDisplacement.x = position.x - this->position.x;
	this->lastDisplacement.y = position.y - this->position.y;
	this->lastDisplacement.z = position.z - this->position.z;

	this->previousPosition = this->position;
	this->position = position;
}

/**
 * Set camera's position for UI transformation
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_prepareForUITransform(Camera this)
{
	ASSERT(this, "Camera::prepareForUITransform: null this");

	this->positionBackup = this->position;

	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
}

/**
 * Set camera's position after UI transformation
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_doneUITransform(Camera this)
{
	ASSERT(this, "Camera::doneUITransform: null this");

	this->position = this->positionBackup;
}

/**
 * Retrieve optical config structure
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Optical config structure
 */
Optical Camera_getOptical(Camera this)
{
	ASSERT(this, "Camera::getOptical: null this");

	return this->optical;
}

/**
 * Set optical config structure
 *
 * @memberof		Camera
 * @public
 *
 * @param this		Function scope
 * @param optical
 */
void Camera_setOptical(Camera this, Optical optical)
{
	ASSERT(this, "Camera::setOptical: null this");

	this->optical = optical;
}

/**
 * Set camera's position displacement
 *
 * @memberof								Camera
 * @public
 *
 * @param this								Function scope
 * @param focusEntityPositionDisplacement
 */
void Camera_setFocusEntityPositionDisplacement(Camera this, Vector3D focusEntityPositionDisplacement)
{
	ASSERT(this, "Camera::setPosition: null this");

	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;

	// focus now
	Camera_focus(this, false);

	// make sure that any other entity knows about the change
	Camera_forceDisplacement(this, true);
}

/**
 * Retrieve last displacement
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Last displacement vector
 */
Vector3D Camera_getLastDisplacement(Camera this)
{
	ASSERT(this, "Camera::getLastDisplacement: null this");

	return this->lastDisplacement;
}

/**
 * Get current stage's size
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Stage size
 */
Size Camera_getStageSize(Camera this)
{
	ASSERT(this, "Camera::getStageSize: null this");

	return this->stageSize;
}

/**
 * Set current stage's size
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 * @param size	Stage size
 */
void Camera_setStageSize(Camera this, Size size)
{
	ASSERT(this, "Camera::setStageSize: null this");

	this->stageSize = size;
}

/**
 * Force values as if camera is moving
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 * @param flag
 */
void Camera_forceDisplacement(Camera this, int flag)
{
	ASSERT(this, "Camera::forceDisplacement: null this");

	this->lastDisplacement.x = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.y = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.z = flag ? __1I_FIX10_6 : 0;
}

/**
 * Start an effect
 *
 * @memberof		Camera
 * @public
 *
 * @param this		Function scope
 * @param effect	Effect reference ID
 * @param args		Various effect parameters
 */
void Camera_startEffect(Camera this, int effect, ...)
{
	ASSERT(this, "Camera::startEffect: null this");

	va_list args;
	va_start(args, effect);
	 CameraEffectManager_startEffect(this->cameraEffectManager, effect, args);
	va_end(args);
}

/**
 * Stop an effect
 *
 * @memberof		Camera
 * @public
 *
 * @param this		Function scope
 * @param effect	Effect reference ID
 */
void Camera_stopEffect(Camera this, int effect)
{
	ASSERT(this, "Camera::stopEffect: null this");

	 CameraEffectManager_stopEffect(this->cameraEffectManager, effect);
}

/**
 * Reset the camera
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_reset(Camera this)
{
	ASSERT(this, "Camera::reset: null this");

	Camera_setFocusGameEntity(this, NULL);

	this->position = (Vector3D){0, 0, 0};
	this->previousPosition = this->position;
	this->lastDisplacement = (Vector3D){0, 0, 0};

	Camera_resetCameraFrustum(this);
}

/**
 * Reset the camera frustum
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 */
void Camera_resetCameraFrustum(Camera this)
{
	ASSERT(this, "Camera::resetCameraFrustum: null this");

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.z0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	this->cameraFrustum.z1 = __SCREEN_DEPTH;
}

/**
 * Set the camera frustum
 *
 * @memberof			Camera
 * @public
 *
 * @param this			Function scope
 * @param cameraFrustum	Camera frustum
 */
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

/**
 * Get the camera frustum
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Camera frustum
 */
CameraFrustum Camera_getCameraFrustum(Camera this)
{
	ASSERT(this, "Camera::getCameraFrustum: null this");

	return this->cameraFrustum;
}

/**
 * Retrieve focus entity position
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Focus entity position vector
 */
Vector3D Camera_getFocusEntityPosition(Camera this)
{
	ASSERT(this, "Camera::getLastDisplacement: null this");

	return this->focusEntityPosition ? *this->focusEntityPosition : (Vector3D){0, 0, 0};
}

/**
 * Retrieve focus entity position displacement
 *
 * @memberof	Camera
 * @public
 *
 * @param this	Function scope
 *
 * @return		Focus entity position displacement vector
 */
Vector3D Camera_getFocusEntityPositionDisplacement(Camera this)
{
	ASSERT(this, "Camera::getFocusEntityPositionDisplacement: null this");

	return this->focusEntityPositionDisplacement;
}


/**
 * Print status
 *
 * @memberof			Camera
 * @public
 *
 * @param this			Function scope
 * @param x				Column
 * @param y				Row
 */
void Camera_print(Camera this, int x, int y)
{
	ASSERT(this, "Camera::print: null this");

	Printing_text(Printing_getInstance(), "MOVE SCREEN", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing_text(Printing_getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", 38, 2, NULL);
	Printing_text(Printing_getInstance(), "      \x1F\x1A\x1B", 38, 3, NULL);
	Printing_text(Printing_getInstance(), "Stage's size:               ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->stageSize.x), x + 10, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->stageSize.y), x + 15, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->stageSize.z), x + 20, y, NULL);
	Printing_text(Printing_getInstance(), "Position:               ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->position.x), x + 10, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->position.y), x + 15, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->position.z), x + 20, y, NULL);
}