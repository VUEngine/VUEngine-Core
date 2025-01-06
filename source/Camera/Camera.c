/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with camera source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
#include <DebugConfig.h>
#include <DirectDraw.h>
#include <Actor.h>
#include <Optics.h>
#include <Printing.h>
#include <VUEngine.h>

#include "Camera.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Optical* _optical __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Vector3D* _cameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Rotation* _cameraRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const Rotation* _cameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
const CameraFrustum* _cameraFrustum __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::reset()
{
	Camera camera = Camera::getInstance();

	camera->transformation.position = Vector3D::zero();
	camera->transformation.rotation = Rotation::zero();
	camera->displacement = Vector3D::zero();
	camera->invertedRotation = Rotation::zero();
	camera->lastDisplacement = Vector3D::zero();

	camera->transformationFlags = __VALID_TRANSFORMATION;

	Camera::resetCameraFrustum(camera);

	if(!isDeleted(camera->cameraMovementManager))
	{
		CameraMovementManager::reset(camera->cameraMovementManager);
	}

	if(!isDeleted(camera->cameraEffectManager))
	{
		CameraEffectManager::reset(camera->cameraEffectManager);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setup(PixelOptical pixelOptical, CameraFrustum cameraFrustum)
{
	Camera camera = Camera::getInstance();
	
	camera->cameraFrustum = Camera::computeClampledFrustum(cameraFrustum);
	camera->optical = Optical::getFromPixelOptical(pixelOptical, camera->cameraFrustum);
	camera->transformationFlags |= __INVALIDATE_TRANSFORMATION;

	DirectDraw::setFrustum(camera->cameraFrustum);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setCameraMovementManager(CameraMovementManager cameraMovementManager)
{
	Camera camera = Camera::getInstance();
	
	if(camera->cameraMovementManager != cameraMovementManager)
	{
		if(!isDeleted(camera->cameraMovementManager))
		{
			delete camera->cameraMovementManager;
		}

		camera->cameraMovementManager = cameraMovementManager;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CameraMovementManager Camera::getCameraMovementManager()
{
	Camera camera = Camera::getInstance();
	
	return camera->cameraMovementManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setCameraEffectManager(CameraEffectManager cameraEffectManager)
{
	Camera camera = Camera::getInstance();
	
	if(camera->cameraEffectManager != cameraEffectManager)
	{
		if(!isDeleted(camera->cameraEffectManager))
		{
			delete camera->cameraEffectManager;
		}

		camera->cameraEffectManager = cameraEffectManager;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CameraEffectManager Camera::getCameraEffectManager()
{
	Camera camera = Camera::getInstance();
	
	return camera->cameraEffectManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Size Camera::getStageSize()
{
	Camera camera = Camera::getInstance();
	
	return camera->stageSize;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setStageSize(Size size)
{
	Camera camera = Camera::getInstance();
	
	camera->stageSize = size;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setFocusActor(Actor focusActor)
{
	Camera camera = Camera::getInstance();
	
	if(!isDeleted(camera->cameraMovementManager))
	{
		CameraMovementManager::setFocusActor(camera->cameraMovementManager, focusActor);

		Camera::focus(camera);
	}

	camera->lastDisplacement = Vector3D::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Actor Camera::getFocusActor()
{
	Camera camera = Camera::getInstance();
	
	if(!isDeleted(camera->cameraMovementManager))
	{
		return CameraMovementManager::getFocusActor(camera->cameraMovementManager);
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::unsetFocusActor()
{
	Camera camera = Camera::getInstance();
	
	if(!isDeleted(camera->cameraMovementManager))
	{
		CameraMovementManager::setFocusActor(camera->cameraMovementManager, NULL);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setFocusActorPositionDisplacement(Vector3D focusActorPositionDisplacement)
{
	Camera camera = Camera::getInstance();
	
	if(!isDeleted(camera->cameraMovementManager))
	{
		CameraMovementManager::setFocusActorPositionDisplacement(camera->cameraMovementManager, &focusActorPositionDisplacement);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Vector3D Camera::getFocusActorPositionDisplacement()
{
	Camera camera = Camera::getInstance();
	
	if(!isDeleted(camera->cameraMovementManager))
	{
		return *CameraMovementManager::getFocusActorPositionDisplacement(camera->cameraMovementManager);
	}

	return Vector3D::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setDisplacement(Vector3D displacement)
{
	Camera camera = Camera::getInstance();
	
	camera->displacement = displacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Vector3D Camera::geDisplacement()
{
	Camera camera = Camera::getInstance();
	
	return camera->displacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setOptical(Optical optical)
{
	Camera camera = Camera::getInstance();
	
	camera->optical = optical;

	camera->transformationFlags |= __INVALIDATE_TRANSFORMATION;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Optical Camera::getOptical()
{
	Camera camera = Camera::getInstance();
	
	return camera->optical;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setTransformation(Transformation transformation, bool cap)
{	
	Camera::setPosition(transformation.position, cap);
	Camera::setRotation(transformation.rotation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Transformation Camera::getTransformation()
{
	Camera camera = Camera::getInstance();
	
	return camera->transformation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setPosition(Vector3D position, bool cap)
{
	Camera camera = Camera::getInstance();
	
	Vector3D currentPosition = camera->transformation.position;
	camera->transformation.position = position;

	if(cap)
	{
		Camera::capPosition(camera);
	}

	Camera::updateTranslationFlags(Vector3D::sub(camera->transformation.position, currentPosition));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::translate(Vector3D displacement, int32 cap)
{
	Camera camera = Camera::getInstance();
	
	Vector3D currentPosition = camera->transformation.position;
	camera->transformation.position = Vector3D::sum(camera->transformation.position, displacement);

	if(cap)
	{
		Camera::capPosition(camera);
	}

	Camera::updateTranslationFlags(Vector3D::sub(camera->transformation.position, currentPosition));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Vector3D Camera::getPosition()
{
	Camera camera = Camera::getInstance();
	
	return camera->transformation.position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::setRotation(Rotation rotation)
{
	Camera camera = Camera::getInstance();
	
	Camera::updateRotationFlags(Rotation::sub(rotation, camera->transformation.rotation));

	camera->transformation.rotation = Rotation::clamp(rotation.x, rotation.y, rotation.z);
	camera->invertedRotation = Rotation::invert(camera->transformation.rotation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::rotate(Rotation rotation)
{
	Camera camera = Camera::getInstance();
	
	Camera::updateRotationFlags(rotation);

	camera->transformation.rotation = Rotation::sum(camera->transformation.rotation, rotation);
	camera->invertedRotation = Rotation::invert(camera->transformation.rotation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Rotation Camera::getRotation()
{
	Camera camera = Camera::getInstance();
	
	return camera->transformation.rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CameraFrustum Camera::getCameraFrustum()
{
	Camera camera = Camera::getInstance();
	
	return camera->cameraFrustum;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Vector3D Camera::getLastDisplacement()
{
	Camera camera = Camera::getInstance();
	
	return camera->lastDisplacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint8 Camera::getTransformationFlags()
{
	Camera camera = Camera::getInstance();
	
	return camera->transformationFlags;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::focus()
{
	Camera camera = Camera::getInstance();
	
	ASSERT(camera->cameraMovementManager, "Camera::focus: null cameraMovementManager");

	if(NULL == CameraMovementManager::getFocusActor(camera->cameraMovementManager))
	{
		return;
	}

	// This was added in commit that reads: 3982a037c69de5ac95626e5d6067edbf744cb68a
	// "Camera transformation flags have to be taken down after one frame to account for the async rendering."
	static bool takeTransformationFlagsDown = false;

	if(takeTransformationFlagsDown)
	{
		camera->transformationFlags = __VALID_TRANSFORMATION;
		takeTransformationFlagsDown = false;
	}

	if(camera->transformationFlags)
	{
		takeTransformationFlagsDown = true;
	}

	camera->lastDisplacement = camera->transformation.position;

	Camera::setPosition(CameraMovementManager::focus(camera->cameraMovementManager, camera), true);

	camera->transformation.position = Vector3D::sum(camera->transformation.position, camera->displacement);
	camera->lastDisplacement = Vector3D::sub(camera->transformation.position, camera->lastDisplacement);

#ifdef __SHOW_CAMERA_STATUS
	Camera::print(1, 1, true);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::startEffect(int32 effect, ...)
{
	Camera camera = Camera::getInstance();
	
#ifdef __TOOLS
	if(VUEngine::isInToolStateTransition())
	{
		return;
	}
#endif

	va_list args;
	va_start(args, effect);
	CameraEffectManager::startEffect(camera->cameraEffectManager, effect, args);
	va_end(args);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::stopEffect(int32 effect)
{
	Camera camera = Camera::getInstance();
	
#ifdef __TOOLS
	if(VUEngine::isInToolStateTransition())
	{
		return;
	}
#endif

	CameraEffectManager::stopEffect(camera->cameraEffectManager, effect);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void Camera::print(int32 x, int32 y, bool inPixels)
{	
	
	
	Printing::text("CAMERA ", x, y++, NULL);
	Printing::text("Position: ", x, ++y, NULL);

	if(inPixels)
	{
		PixelVector::print(PixelVector::getFromVector3D(Camera::getPosition(), 0), x, ++y);
	}
	else
	{
		Vector3D::print(Camera::getPosition(), x, ++y);
	}

	y += 3;
	Printing::text("Rotation: ", x, ++y, NULL);
	Rotation::print(Camera::getRotation(), x, ++y);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::resetCameraFrustum()
{
	Camera camera = Camera::getInstance();
	
	camera->cameraFrustum.x0 = 0;
	camera->cameraFrustum.y0 = 0;
	camera->cameraFrustum.z0 = 0;
	camera->cameraFrustum.x1 = __SCREEN_WIDTH;
	camera->cameraFrustum.y1 = __SCREEN_HEIGHT;
	camera->cameraFrustum.z1 = __SCREEN_DEPTH;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::capPosition()
{
	Camera camera = Camera::getInstance();
	
	camera->transformation.position = Camera::computCappedPosition(camera->transformation.position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Vector3D Camera::computCappedPosition(Vector3D position)
{
	Camera camera = Camera::getInstance();
	
	if(position.x < 0)
	{
		position.x = 0;
	}

	if(position.x + __SCREEN_WIDTH_METERS > camera->stageSize.x)
	{
		position.x = camera->stageSize.x - __SCREEN_WIDTH_METERS;
	}

	if(position.y < 0)
	{
		position.y = 0;
	}

	if(position.y + __SCREEN_HEIGHT_METERS > camera->stageSize.y)
	{
		position.y = camera->stageSize.y - __SCREEN_HEIGHT_METERS;
	}

	if(position.z < 0)
	{
		position.z = 0;
	}

	if(position.z > camera->stageSize.z)
	{
		position.z = camera->stageSize.z;
	}

	return position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CameraFrustum Camera::computeClampledFrustum(CameraFrustum cameraFrustum)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::updateTranslationFlags(Vector3D translation)
{
	Camera camera = Camera::getInstance();
	
	if(0 != translation.z)
	{
		camera->transformationFlags |= __INVALIDATE_PROJECTION | __INVALIDATE_SCALE;
	}
	else if(0 != translation.x || 0 != translation.y)
	{
		camera->transformationFlags |= __INVALIDATE_PROJECTION;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Camera::updateRotationFlags(Rotation rotation)
{
	Camera camera = Camera::getInstance();
	
	if(rotation.x || rotation.y || rotation.z)
	{
		camera->transformationFlags |= __INVALIDATE_ROTATION;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Camera::constructor()
{	
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->transformation.position = Vector3D::zero();
	this->cameraMovementManager = CameraMovementManager::getInstance();
	this->cameraEffectManager = CameraEffectManager::getInstance();

	this->transformation.position = Vector3D::zero();
	this->displacement = Vector3D::zero();
	this->transformation.rotation = Rotation::zero();
	this->invertedRotation = Rotation::invert(this->transformation.rotation);
	this->lastDisplacement = Vector3D::zero();

	this->cameraFrustum.x0 = 0;
	this->cameraFrustum.y0 = 0;
	this->cameraFrustum.z0 = 0;
	this->cameraFrustum.x1 = __SCREEN_WIDTH;
	this->cameraFrustum.y1 = __SCREEN_HEIGHT;
	this->cameraFrustum.z1 = __SCREEN_DEPTH;

	this->transformationFlags = __VALID_TRANSFORMATION;

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

	Camera::setup(pixelOptical, this->cameraFrustum);

	// Set global pointer to improve access to critical values
	_optical = &this->optical;
	_cameraPosition = &this->transformation.position;
	_cameraFrustum = &this->cameraFrustum;
	_cameraRotation = &this->transformation.rotation;
	_cameraInvertedRotation = &this->invertedRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Camera::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
