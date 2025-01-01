/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <Entity.h>
#include <VIPManager.h>

#include "CameraMovementManager.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Camera;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	CameraMovementManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::destructor()
{
	// destroy base
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::reset()
{
	this->focusEntity = NULL;
	this->focusEntityPosition = NULL;
	this->focusEntityPositionDisplacement = Vector3D::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::setFocusEntity(Entity focusEntity)
{
	this->focusEntity = focusEntity;
	this->focusEntityPosition = NULL;
	this->focusEntityRotation = NULL;

	if(!isDeleted(this->focusEntity))
	{
		Entity::addEventListener(this->focusEntity, ListenerObject::safeCast(this), (EventListener)CameraMovementManager::onFocusEntityDeleted,  kEventContainerDeleted);
		this->focusEntityPosition = Entity::getPosition(this->focusEntity);
		this->focusEntityRotation = Entity::getRotation(this->focusEntity);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Entity CameraMovementManager::getFocusEntity()
{
	return this->focusEntity;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::setFocusEntityPositionDisplacement(const Vector3D* focusEntityPositionDisplacement)
{
	if(NULL == focusEntityPositionDisplacement)
	{
		this->focusEntityPositionDisplacement = Vector3D::zero();
		return;
	}

	this->focusEntityPositionDisplacement = *focusEntityPositionDisplacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* CameraMovementManager::getFocusEntityPositionDisplacement()
{
	return &this->focusEntityPositionDisplacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D CameraMovementManager::focus(Camera camera)
{
	if(isDeleted(camera))
	{
		return Vector3D::zero();
	}

	if(isDeleted(this->focusEntity))
	{
		return Camera::getPosition(camera);
	}

	NormalizedDirection normalizedDirection = Entity::getNormalizedDirection(this->focusEntity);

	// calculate the target position
	Vector3D cameraNewPosition =
	{
		this->focusEntityPosition->x + normalizedDirection.x * this->focusEntityPositionDisplacement.x - __HALF_SCREEN_WIDTH_METERS,
		this->focusEntityPosition->y + normalizedDirection.y * this->focusEntityPositionDisplacement.y - __HALF_SCREEN_HEIGHT_METERS,
		this->focusEntityPosition->z + normalizedDirection.z * this->focusEntityPositionDisplacement.z - __HALF_SCREEN_DEPTH_METERS,
	};

	return cameraNewPosition;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CameraMovementManager::onFocusEntityDeleted(ListenerObject eventFirer)
{
	if(ListenerObject::safeCast(this->focusEntity) == eventFirer)
	{
		CameraMovementManager::setFocusEntity(this, NULL);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

