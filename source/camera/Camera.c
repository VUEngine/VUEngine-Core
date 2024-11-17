/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
#include <DebugConfig.h>
#include <DirectDraw.h>
#include <Entity.h>
#include <Optics.h>
#include <Printing.h>

#include "Camera.h"


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

const Optical* _optical __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Vector3D* _cameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Rotation* _cameraRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Rotation* _cameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const CameraFrustum* _cameraFrustum __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Camera::reset()
{
	this->position = Vector3D::zero();
	this->displacement = Vector3D::zero();
	this->rotation = Rotation::zero();
	this->invertedRotation = Rotation::zero();
	this->lastDisplacement = Vector3D::zero();

	this->transformationFlags = false;

	Camera::resetCameraFrustum(this);

	if(!isDeleted(this->cameraMovementManager))
	{
		CameraMovementManager::reset(this->cameraMovementManager);
	}

	if(!isDeleted(this->cameraEffectManager))
	{
		CameraEffectManager::reset(this->cameraEffectManager);
	}
}
//---------------------------------------------------------------------------------------------------------
void Camera::setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum)
{
	this->cameraFrustum = Camera::computeClampledFrustum(this, cameraFrustum);
	this->optical = Optical::getFromPixelOptical(pixelOptical, this->cameraFrustum);
	this->transformationFlags |= __INVALIDATE_TRANSFORMATION;

	DirectDraw::setFrustum(DirectDraw::getInstance(), this->cameraFrustum);
}
//---------------------------------------------------------------------------------------------------------
void Camera::setCameraMovementManager(CameraMovementManager cameraMovementManager)
{
	if(this->cameraMovementManager != cameraMovementManager)
	{
		if(!isDeleted(this->cameraMovementManager))
		{
			delete this->cameraMovementManager;
		}

		this->cameraMovementManager = cameraMovementManager;
	}
}
//---------------------------------------------------------------------------------------------------------
CameraMovementManager Camera::getCameraMovementManager()
{
	return this->cameraMovementManager;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setCameraEffectManager(CameraEffectManager cameraEffectManager)
{
	if(this->cameraEffectManager != cameraEffectManager)
	{
		if(!isDeleted(this->cameraEffectManager))
		{
			delete this->cameraEffectManager;
		}

		this->cameraEffectManager = cameraEffectManager;
	}
}
//---------------------------------------------------------------------------------------------------------
CameraEffectManager Camera::getCameraEffectManager()
{
	return this->cameraEffectManager;
}
//---------------------------------------------------------------------------------------------------------
Size Camera::getStageSize()
{
	return this->stageSize;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setStageSize(Size size)
{
	this->stageSize = size;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setFocusEntity(Entity focusEntity)
{
	if(!isDeleted(this->cameraMovementManager))
	{
		CameraMovementManager::setFocusEntity(this->cameraMovementManager, focusEntity);

		Camera::focus(this);
	}

	this->lastDisplacement = Vector3D::zero();
}
//---------------------------------------------------------------------------------------------------------
Entity Camera::getFocusEntity()
{
	if(!isDeleted(this->cameraMovementManager))
	{
		return CameraMovementManager::getFocusEntity(this->cameraMovementManager);
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void Camera::unsetFocusEntity()
{
	if(!isDeleted(this->cameraMovementManager))
	{
		CameraMovementManager::setFocusEntity(this->cameraMovementManager, NULL);
	}
}
//---------------------------------------------------------------------------------------------------------
void Camera::setFocusEntityPositionDisplacement(Vector3D focusEntityPositionDisplacement)
{
	if(!isDeleted(this->cameraMovementManager))
	{
		CameraMovementManager::setFocusEntityPositionDisplacement(this->cameraMovementManager, &focusEntityPositionDisplacement);
	}
}
//---------------------------------------------------------------------------------------------------------
Vector3D Camera::getFocusEntityPositionDisplacement()
{
	if(!isDeleted(this->cameraMovementManager))
	{
		return *CameraMovementManager::getFocusEntityPositionDisplacement(this->cameraMovementManager);
	}

	return Vector3D::zero();
}
//---------------------------------------------------------------------------------------------------------
void Camera::setDisplacement(Vector3D displacement)
{
	this->displacement = displacement;
}
//---------------------------------------------------------------------------------------------------------
Vector3D Camera::geDisplacement()
{
	return this->displacement;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setOptical(Optical optical)
{
	this->optical = optical;

	this->transformationFlags |= __INVALIDATE_TRANSFORMATION;
}
//---------------------------------------------------------------------------------------------------------
Optical Camera::getOptical()
{
	return this->optical;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setPosition(Vector3D position, bool cap)
{
	Vector3D currentPosition = this->position;
	this->position = position;

	if(cap)
	{
		Camera::capPosition(this);
	}

	this->transformationFlags |= Camera::computeTranslationFlags(Vector3D::sub(this->position, currentPosition));
}
//---------------------------------------------------------------------------------------------------------
void Camera::translate(Vector3D displacement, int32 cap)
{
	Vector3D currentPosition = this->position;
	this->position = Vector3D::sum(this->position, displacement);

	if(cap)
	{
		Camera::capPosition(this);
	}

	this->transformationFlags |= Camera::computeTranslationFlags(Vector3D::sub(this->position, currentPosition));
}
//---------------------------------------------------------------------------------------------------------
Vector3D Camera::getPosition()
{
	return this->position;
}
//---------------------------------------------------------------------------------------------------------
void Camera::setRotation(Rotation rotation)
{
	this->transformationFlags |= Camera::computeRotationFlags(Rotation::sub(rotation, this->rotation));

	this->rotation = Rotation::clamp(rotation.x, rotation.y, rotation.z);
	this->invertedRotation = Rotation::invert(this->rotation);
}
//---------------------------------------------------------------------------------------------------------
void Camera::rotate(Rotation rotation)
{
	this->transformationFlags |= Camera::computeRotationFlags(rotation);

	this->rotation = Rotation::sum(this->rotation, rotation);
	this->invertedRotation = Rotation::invert(this->rotation);
}
//---------------------------------------------------------------------------------------------------------
Rotation Camera::getRotation()
{
	return this->rotation;
}
//---------------------------------------------------------------------------------------------------------
CameraFrustum Camera::getCameraFrustum()
{
	return this->cameraFrustum;
}
//---------------------------------------------------------------------------------------------------------
Vector3D Camera::getLastDisplacement()
{
	return this->lastDisplacement;
}
//---------------------------------------------------------------------------------------------------------
uint8 Camera::getTransformationFlags()
{
	return this->transformationFlags;
}
//---------------------------------------------------------------------------------------------------------
void Camera::focus()
{
	static bool takeTransformationFlagsDown = false;

	if(takeTransformationFlagsDown)
	{
		this->transformationFlags = false;
		takeTransformationFlagsDown = false;
	}

	if(this->transformationFlags)
	{
		takeTransformationFlagsDown = true;
	}

	ASSERT(this->cameraMovementManager, "Camera::focus: null cameraMovementManager");

	if(NULL == CameraMovementManager::getFocusEntity(this->cameraMovementManager))
	{
		return;
	}

	this->lastDisplacement = this->position;

	Camera::setPosition(this, CameraMovementManager::focus(this->cameraMovementManager, this), true);

	this->position = Vector3D::sum(this->position, this->displacement);
	this->lastDisplacement = Vector3D::sub(this->position, this->lastDisplacement);

#ifdef __SHOW_CAMERA_STATUS
	Camera::print(this, 1, 1, true);
#endif
}
//---------------------------------------------------------------------------------------------------------
void Camera::startEffect(int32 effect, ...)
{
	if(VUEngine::isInToolStateTransition(VUEngine::getInstance()))
	{
		return;
	}

	if(VUEngine::isInToolStateTransition(VUEngine::getInstance()))
	{
		return;
	}

	va_list args;
	va_start(args, effect);
	CameraEffectManager::startEffect(this->cameraEffectManager, effect, args);
	va_end(args);
}
//---------------------------------------------------------------------------------------------------------
void Camera::stopEffect(int32 effect)
{
	if(VUEngine::isInToolStateTransition(VUEngine::getInstance()))
	{
		return;
	}

	if(VUEngine::isInToolStateTransition(VUEngine::getInstance()))
	{
		return;
	}

	CameraEffectManager::stopEffect(this->cameraEffectManager, effect);
}
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void Camera::print(int32 x, int32 y, bool inPixels)
{
	Printing printing = Printing::getInstance();
	
	Printing::text(printing, "CAMERA ", x, y++, NULL);
	Printing::text(printing, "Position: ", x, ++y, NULL);

	if(inPixels)
	{
		PixelVector::print(PixelVector::getFromVector3D(Camera::getPosition(Camera::getInstance()), 0), x, ++y);
	}
	else
	{
		Vector3D::print(Camera::getPosition(Camera::getInstance()), x, ++y);
	}

	y += 3;
	Printing::text(printing, "Rotation: ", x, ++y, NULL);
	Rotation::print(Camera::getRotation(Camera::getInstance()), x, ++y);
}
#endif
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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

	this->position = Vector3D::zero();
	this->displacement = Vector3D::zero();
	this->rotation = Rotation::zero();
	this->invertedRotation = Rotation::invert(this->rotation);
	this->lastDisplacement = Vector3D::zero();

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.z0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	this->cameraFrustum.z1 = __SCREEN_DEPTH;

	this->transformationFlags = false;

	PixelOptical pixelOptical =
    {
    	__MAXIMUM_X_VIEW_DISTANCE,				// maximum distance from the screen to the infinite
    	__MAXIMUM_Y_VIEW_DISTANCE,				// maximum distance from the screen to the infinite
		__CAMERA_NEAR_PLANE,					// distance from player's eyes to the virtual screen
    	__BASE_FACTOR,							// distance from left to right eye (depth perception)
    	__HORIZONTAL_VIEW_POINT_CENTER,			// horizontal View point center
    	__VERTICAL_VIEW_POINT_CENTER,			// vertical View point center
    	__SCALING_MODIFIER_FACTOR,				// scaling factor for sprite resizing
    };

	Camera::setup(this, pixelOptical, this->cameraFrustum);

	// set global pointer to improve access to critical values
	_optical = &this->optical;
	_cameraPosition = &this->position;
	_cameraFrustum = &this->cameraFrustum;
	_cameraRotation = &this->rotation;
	_cameraInvertedRotation = &this->invertedRotation;
}
//---------------------------------------------------------------------------------------------------------
void Camera::destructor()
{
	// destroy base
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Camera::resetCameraFrustum()
{
	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.z0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	this->cameraFrustum.z1 = __SCREEN_DEPTH;
}
//---------------------------------------------------------------------------------------------------------
void Camera::capPosition()
{
	this->position = Camera::computCappedPosition(this, this->position);
}
//---------------------------------------------------------------------------------------------------------
Vector3D Camera::computCappedPosition(Vector3D position)
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
//---------------------------------------------------------------------------------------------------------
CameraFrustum Camera::computeClampledFrustum(CameraFrustum cameraFrustum)
{
	if(cameraFrustum.x1 > __SCREEN_WIDTH)
	{
		cameraFrustum.x1 = __SCREEN_WIDTH;
	}

	if(cameraFrustum.y1 > __SCREEN_HEIGHT)
	{
		cameraFrustum.y1 = __SCREEN_HEIGHT;
	}

	// 9: 2's power equal to the math type fixed_t
	if(cameraFrustum.z1 > (1 << (9 + __PIXELS_PER_METER_2_POWER)))
	{
		cameraFrustum.z1 = 1;
	}

	if(cameraFrustum.x0 >= cameraFrustum.x1)
	{
		cameraFrustum.x0 = cameraFrustum.x1 - 1;
	}

	if(cameraFrustum.y0 >= cameraFrustum.y1)
	{
		cameraFrustum.y0 = cameraFrustum.y1 - 1;
	}

	if(cameraFrustum.z0 >= cameraFrustum.z1)
	{
		cameraFrustum.z0 = cameraFrustum.z1 - 1;
	}

	return cameraFrustum;
}
//---------------------------------------------------------------------------------------------------------
static uint8 Camera::computeTranslationFlags(Vector3D translation)
{
	if(0 != translation.z)
	{
		return __INVALIDATE_PROJECTION | __INVALIDATE_SCALE;
	}
	else if(0 != translation.x || 0 != translation.y)
	{
		return __INVALIDATE_PROJECTION;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
static uint8 Camera::computeRotationFlags(Rotation rotation)
{
	if(rotation.x || rotation.y || rotation.z)
	{
		return __INVALIDATE_ROTATION;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
