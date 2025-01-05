/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <Actor.h>
#include <VIPManager.h>

#include "CameraMovementManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Camera;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	CameraMovementManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::reset()
{
	this->focusActor = NULL;
	this->focusActorPosition = NULL;
	this->focusActorPositionDisplacement = Vector3D::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::setFocusActor(Actor focusActor)
{
	this->focusActor = focusActor;
	this->focusActorPosition = NULL;
	this->focusActorRotation = NULL;

	if(!isDeleted(this->focusActor))
	{
		Actor::addEventListener
		(
			this->focusActor, ListenerObject::safeCast(this), (EventListener)CameraMovementManager::onFocusActorDeleted, 
			kEventContainerDeleted
		);
		
		this->focusActorPosition = Actor::getPosition(this->focusActor);
		this->focusActorRotation = Actor::getRotation(this->focusActor);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor CameraMovementManager::getFocusActor()
{
	return this->focusActor;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraMovementManager::setFocusActorPositionDisplacement(const Vector3D* focusActorPositionDisplacement)
{
	if(NULL == focusActorPositionDisplacement)
	{
		this->focusActorPositionDisplacement = Vector3D::zero();
		return;
	}

	this->focusActorPositionDisplacement = *focusActorPositionDisplacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* CameraMovementManager::getFocusActorPositionDisplacement()
{
	return &this->focusActorPositionDisplacement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D CameraMovementManager::focus(Camera camera)
{
	if(isDeleted(camera))
	{
		return Vector3D::zero();
	}

	if(isDeleted(this->focusActor))
	{
		return Camera::getPosition(camera);
	}

	NormalizedDirection normalizedDirection = Actor::getNormalizedDirection(this->focusActor);

	// Calculate the target position
	Vector3D cameraNewPosition =
	{
		this->focusActorPosition->x + normalizedDirection.x * this->focusActorPositionDisplacement.x - __HALF_SCREEN_WIDTH_METERS,
		this->focusActorPosition->y + normalizedDirection.y * this->focusActorPositionDisplacement.y - __HALF_SCREEN_HEIGHT_METERS,
		this->focusActorPosition->z + normalizedDirection.z * this->focusActorPositionDisplacement.z - __HALF_SCREEN_DEPTH_METERS,
	};

	return cameraNewPosition;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CameraMovementManager::onFocusActorDeleted(ListenerObject eventFirer)
{
	if(ListenerObject::safeCast(this->focusActor) == eventFirer)
	{
		CameraMovementManager::setFocusActor(this, NULL);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
