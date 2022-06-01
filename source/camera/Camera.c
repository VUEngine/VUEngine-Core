/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
const Rotation* _cameraRotation = NULL;
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
	this->focusEntityRotation = NULL;

	this->position = Vector3D::zero();
	this->positionBackup = Vector3D::zero();

	this->rotation = (Rotation){0, 0, 0};

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
	_cameraFrustum = &this->cameraFrustum;
	_cameraRotation = &this->rotation;
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
void Camera::focus(uint32 checkIfFocusEntityIsMoving)
{
	this->transformationFlags = false;

	ASSERT(this->cameraMovementManager, "Camera::focus: null cameraMovementManager");

	CameraMovementManager::focus(this->cameraMovementManager, checkIfFocusEntityIsMoving);

#ifdef __SHOW_CAMERA_STATUS
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
	this->focusEntityRotation = NULL;

	if(focusEntity)
	{
		this->focusEntityPosition = SpatialObject::getPosition(this->focusEntity);
		this->focusEntityRotation = SpatialObject::getRotation(this->focusEntity);

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

static uint8 Camera::computeTranslationFlags(Vector3D translation)
{
	if(translation.z)
	{
		return __INVALIDATE_PROJECTION | __INVALIDATE_SCALE;
	}
	else if(translation.x || translation.y)
	{
		return __INVALIDATE_PROJECTION;
	}

	return false;
}

/**
 * Translate camera
 *
 * @param translation
 * @param cap
 */
void Camera::translate(Vector3D translation, int32 cap)
{
	this->transformationFlags |= Camera::computeTranslationFlags(translation);

	this->position = Vector3D::sum(this->position, translation);

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
 * Get camera's rotation
 *
 * @return		Camera rotation
 */
Rotation Camera::getRotation()
{
	return this->rotation;
}

/**
 * Set camera's position
 *
 * @param position	Camera position
 */
void Camera::setPosition(Vector3D position, bool cap)
{
	this->transformationFlags |= Camera::computeTranslationFlags(Vector3D::sub(position, this->position));

	this->position = position;

	if(cap)
	{
		Camera::capPosition(this);
	}
}

static uint8 Camera::computeRotationFlags(Rotation rotation)
{
	if(rotation.x || rotation.y || rotation.z)
	{
		return __INVALIDATE_ROTATION;
	}

	return false;
}

/**
 * Set camera's rotation
 *
 * @param rotation	Camera rotation
 */
void Camera::setRotation(Rotation rotation)
{
	this->transformationFlags |= Camera::computeRotationFlags(Rotation::sub(rotation, this->rotation));

	this->rotation = rotation;
}

/**
 * Set camera's position
 *
 * @param position	Camera position
 */
void Camera::rotate(Rotation rotation)
{
	this->transformationFlags |= Camera::computeRotationFlags(rotation);

	this->rotation = Rotation::sum(this->rotation, rotation);
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

	this->transformationFlags |= __INVALIDATE_PROJECTION | __INVALIDATE_ROTATION | __INVALIDATE_SCALE;
}

/**
 * Set camera's position displacement
 *
 * @param focusEntityPositionDisplacement
 */
void Camera::setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement)
{
	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;
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
 * Start an effect
 *
 * @param effect	Effect reference ID
 * @param args		Various effect parameters
 */
void Camera::startEffect(int32 effect, ...)
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
void Camera::stopEffect(int32 effect)
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
	return NULL != this->focusEntityPosition ? *this->focusEntityPosition : Vector3D::zero();
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
 * Retrieve focus entity rotation
 *
 * @return		Focus entity rotation
 */
Rotation Camera::getFocusEntityRotation()
{
	return NULL != this->focusEntityRotation ? *this->focusEntityRotation : Rotation::zero();
}

/**
 * Retrieve the status of the camera's transformation
 *
 * @return		Transformation's status flag
 */
uint8 Camera::getTransformationFlags()
{
	return this->transformationFlags;
}

/**
 * Print status
 *
 * @param x				Column
 * @param y				Row
 * @param inPixels		Whether to printing the output in pixels or not
 */
void Camera::print(int32 x, int32 y, bool inPixels)
{
	uint8 controlsXPos = 38;
	uint8 controlsYPos = 2;
	Printing::text(Printing::getInstance(), "Mode    \x16", controlsXPos, controlsYPos++, NULL);
	controlsYPos++;
	Printing::text(Printing::getInstance(), "Move\x1E\x1A\x1B\x1C\x1D", controlsXPos, controlsYPos++, NULL);
	Printing::text(Printing::getInstance(), "      \x1F\x1A\x1B", controlsXPos, controlsYPos++, NULL);

	Printing::text(Printing::getInstance(), "MOVE CAMERA", x, y++, NULL);
	Printing::text(Printing::getInstance(), "              X    Y    Z    ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Stage's size:                   ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.x) : __FIX10_6_TO_I(this->stageSize.x), x + 14, y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.y) : __FIX10_6_TO_I(this->stageSize.y), x + 19, y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->stageSize.z) : __FIX10_6_TO_I(this->stageSize.z), x + 24, y, NULL);
	Printing::text(Printing::getInstance(), "Position:                       ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.x) : __FIX10_6_TO_I(this->position.x), x + 14, y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.y) : __FIX10_6_TO_I(this->position.y), x + 19, y, NULL);
	Printing::int32(Printing::getInstance(), inPixels ? __METERS_TO_PIXELS(this->position.z) : __FIX10_6_TO_I(this->position.z), x + 24, y, NULL);
}
