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
 * @fn			Camera::getInstance()
 * @memberof	Camera
 * @public
 * @return		Camera instance
 */


/**
 * Class constructor
 */
void Camera::constructor()
{
	// construct base object
	Base::constructor();

	// initialize world's camera's position
	this->position = Vector3D::zero();

	// set the default camera movement manager
	this->cameraMovementManager = CameraMovementManager::getInstance();

	// set the default camera effect manager
	this->cameraEffectManager = CameraEffectManager::getInstance();

	this->focusEntityPositionDisplacement.x = 0;
	this->focusEntityPositionDisplacement.y = 0;
	this->focusEntityPositionDisplacement.z = 0;

	// clear focus actor pointer
	this->focusEntity = NULL;
	this->focusEntityPosition = NULL;

	this->position = Vector3D::zero();
	this->previousPosition = Vector3D::zero();
	this->positionBackup = Vector3D::zero();
	this->lastDisplacement = Vector3D::zero();

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

	Camera::setOptical(this, Optical::getFromPixelOptical(pixelOptical));

	// set global pointer to improve access to critical values
	_optical = &this->optical;
	_cameraPosition = &this->position;
	_cameraPreviousPosition = &this->previousPosition;
	_cameraDisplacement = &this->lastDisplacement;
	_cameraFrustum = &this->cameraFrustum;
}

/**
 * Class destructor
 */
void Camera::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Set the movement manager
 *
 * @param cameraMovementManager	The CameraMovementManager
 */
void Camera::setCameraMovementManager(CameraMovementManager cameraMovementManager)
{
	if(this->cameraMovementManager != cameraMovementManager)
	{
		if(this->cameraMovementManager)
		{
			delete this->cameraMovementManager;
		}

		this->cameraMovementManager = cameraMovementManager;
	}
}

/**
 * Set the effect manager
 *
 * @param cameraEffectManager	The CameraEffectManager
 */
void Camera::setCameraEffectManager(CameraEffectManager cameraEffectManager)
{
	if(this->cameraEffectManager != cameraEffectManager)
	{
		if(this->cameraEffectManager)
		{
			delete this->cameraEffectManager;
		}

		this->cameraEffectManager = cameraEffectManager;
	}
}

/**
 * Center world's camera in function of focus actor's position
 *
 * @param checkIfFocusEntityIsMoving	The CameraEffectManager
 */
void Camera::focus(u32 checkIfFocusEntityIsMoving)
{
	ASSERT(this->cameraMovementManager, "Camera::focus: null cameraMovementManager");

#ifdef __DEBUG_TOOLS
	if(!Game::isInSpecialMode(Game::getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if(!Game::isInSpecialMode(Game::getInstance()))
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game::isInSpecialMode(Game::getInstance()))
#endif

	CameraMovementManager::focus(this->cameraMovementManager, checkIfFocusEntityIsMoving);

#ifdef __PRINT_CAMERA_STATUS
	Camera::print(this, 1, 1);
#endif
}

/**
 * Set the focus entity
 *
 * @param focusEntity	The CameraEffectManager
 */
void Camera::setFocusGameEntity(Entity focusEntity)
{
	this->focusEntity = focusEntity;
	this->focusEntityPosition = NULL;

	if(focusEntity)
	{
		this->focusEntityPosition =  SpatialObject::getPosition(this->focusEntity);

		// focus now
		Camera::focus(this, false);
	}
}

/**
 * Unset the focus entity
 */
void Camera::unsetFocusEntity()
{
	this->focusEntity = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

/**
 * Retrieve focus entity
 *
 * @return		Focus Entity
 */
Entity Camera::getFocusEntity()
{
	return this->focusEntity;
}

/**
 * An actor has been deleted
 *
 * @param actor	Entity that has been deleted
 */
void Camera::onFocusEntityDeleted(Entity actor)
{
	if(this->focusEntity == actor)
	{
		Camera::unsetFocusEntity(this);
	}
}

/**
 * Translate camera
 *
 * @param translation
 * @param cap
 */
void Camera::move(Vector3D translation, int cap)
{
	this->lastDisplacement = translation;

	this->position.x += translation.x;
	this->position.y += translation.y;
	this->position.z += translation.z;

	if(cap)
	{
		Camera::capPosition(this);
	}
}

/**
 * Cap position
 *
 * @return position	Capped Vector3d
 */
Vector3D Camera::getCappedPosition(Vector3D position)
{
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
 */
void Camera::capPosition()
{
	this->position = Camera::getCappedPosition(this, this->position);
}

/**
 * Get camera's position
 *
 * @return		Camera position
 */
Vector3D Camera::getPosition()
{
	return this->position;
}

/**
 * Set camera's position
 *
 * @param position	Camera position
 */
void Camera::setPosition(Vector3D position)
{
	position = Camera::getCappedPosition(this, position);

	this->lastDisplacement.x = position.x - this->position.x;
	this->lastDisplacement.y = position.y - this->position.y;
	this->lastDisplacement.z = position.z - this->position.z;

	this->previousPosition = this->position;
	this->position = position;
}

/**
 * Set camera's position for UI transformation
 */
void Camera::prepareForUI()
{
	this->positionBackup = this->position;

	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
}

/**
 * Set camera's position after UI transformation
 */
void Camera::doneUITransform()
{
	this->position = this->positionBackup;
}

/**
 * Retrieve optical config structure
 *
 * @return 		Optical config structure
 */
Optical Camera::getOptical()
{
	return this->optical;
}

/**
 * Set optical config structure
 *
 * @param optical
 */
void Camera::setOptical(Optical optical)
{
	this->optical = optical;
}

/**
 * Set camera's position displacement
 *
 * @param focusEntityPositionDisplacement
 */
void Camera::setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement)
{
	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;

	// focus now
	Camera::focus(this, false);

	// make sure that any other entity knows about the change
	Camera::forceDisplacement(this, true);
}

/**
 * Retrieve last displacement
 *
 * @return		Last displacement vector
 */
Vector3D Camera::getLastDisplacement()
{
	return this->lastDisplacement;
}

/**
 * Get current stage's size
 *
 * @return		Stage size
 */
Size Camera::getStageSize()
{
	return this->stageSize;
}

/**
 * Set current stage's size
 *
 * @param size	Stage size
 */
void Camera::setStageSize(Size size)
{
	this->stageSize = size;
}

/**
 * Force values as if camera is moving
 *
 * @param flag
 */
void Camera::forceDisplacement(int flag)
{
	this->lastDisplacement.x = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.y = flag ? __1I_FIX10_6 : 0;
	this->lastDisplacement.z = flag ? __1I_FIX10_6 : 0;
}

/**
 * Start an effect
 *
 * @param effect	Effect reference ID
 * @param args		Various effect parameters
 */
void Camera::startEffect(int effect, ...)
{
	va_list args;
	va_start(args, effect);
	CameraEffectManager::startEffect(this->cameraEffectManager, effect, args);
	va_end(args);
}

/**
 * Stop an effect
 *
 * @param effect	Effect reference ID
 */
void Camera::stopEffect(int effect)
{
	CameraEffectManager::stopEffect(this->cameraEffectManager, effect);
}

/**
 * Reset the camera
 */
void Camera::reset()
{
	Camera::setFocusGameEntity(this, NULL);

	this->position = Vector3D::zero();
	this->previousPosition = this->position;
	this->lastDisplacement = Vector3D::zero();

	Camera::resetCameraFrustum(this);
}

/**
 * Reset the camera frustum
 */
void Camera::resetCameraFrustum()
{
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
 * @param cameraFrustum	Camera frustum
 */
void Camera::setCameraFrustum(CameraFrustum cameraFrustum)
{
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
 * @return		Camera frustum
 */
CameraFrustum Camera::getCameraFrustum()
{
	return this->cameraFrustum;
}

/**
 * Retrieve focus entity position
 *
 * @return		Focus entity position vector
 */
Vector3D Camera::getFocusEntityPosition()
{
	return this->focusEntityPosition ? *this->focusEntityPosition : Vector3D::zero();
}

/**
 * Retrieve focus entity position displacement
 *
 * @return		Focus entity position displacement vector
 */
Vector3D Camera::getFocusEntityPositionDisplacement()
{
	return this->focusEntityPositionDisplacement;
}

/**
 * Print status
 *
 * @param x				Column
 * @param y				Row
 * @param inPixels		Whether to printing the output in pixels or not
 */
void Camera::print(int x, int y, bool inPixels)
{
	Printing::text(Printing::getInstance(), "MOVE CAMERA", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Mode    \x16", 38, 1, NULL);
	Printing::text(Printing::getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", 38, 3, NULL);
	Printing::text(Printing::getInstance(), "      \x1F\x1A\x1B", 38, 4, NULL);
	Printing::text(Printing::getInstance(), "              X    Y    Z    ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Stage's size:                   ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.x) : __FIX10_6_TO_I(this->stageSize.x), x + 14, y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.y) : __FIX10_6_TO_I(this->stageSize.y), x + 19, y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.z) : __FIX10_6_TO_I(this->stageSize.z), x + 24, y, NULL);
	Printing::text(Printing::getInstance(), "Position:                       ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.x) : __FIX10_6_TO_I(this->position.x), x + 14, y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.y) : __FIX10_6_TO_I(this->position.y), x + 19, y, NULL);
	Printing::int(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.z) : __FIX10_6_TO_I(this->position.z), x + 24, y, NULL);
}
