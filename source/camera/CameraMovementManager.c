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

#include <CameraMovementManager.h>

#include <Camera.h>
#include <Entity.h>
#include <VIPManager.h>

#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Camera;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CameraMovementManager::getInstance()
 * @memberof	CameraMovementManager
 * @public
 * @return		CameraMovementManager instance
 */


/**
 * Class constructor
 */
void CameraMovementManager::constructor()
{
	// construct base object
	Base::constructor();

	CameraMovementManager::reset(this);
}

/**
 * Class destructor
 */
void CameraMovementManager::destructor()
{
	// destroy base
	Base::destructor();
}

void CameraMovementManager::reset()
{
	this->focusEntity = NULL;
	this->focusEntityPosition = NULL;
	this->focusEntityPositionDisplacement = Vector3D::zero();
	this->lastCameraDisplacement = Vector3D::zero();
}

Entity CameraMovementManager::getFocusEntity()
{
	return this->focusEntity;
}

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

		// focus now
		Camera::focus(Camera::getInstance(), false);
	}
}

void CameraMovementManager::onFocusEntityDeleted(ListenerObject eventFirer)
{
	if(ListenerObject::safeCast(this->focusEntity) == eventFirer)
	{
		CameraMovementManager::setFocusEntity(this, NULL);
	}
}

void CameraMovementManager::setFocusEntityPositionDisplacement(const Vector3D* focusEntityPositionDisplacement)
{
	if(NULL == focusEntityPositionDisplacement)
	{
		this->focusEntityPositionDisplacement = Vector3D::zero();
		return;
	}

	this->focusEntityPositionDisplacement = *focusEntityPositionDisplacement;
}

/**
 * Center world's camera in function of focus actor's position
 *
 * @param checkIfFocusEntityIsMoving	Flag whether to check if the focus Entity is moving
 */
void CameraMovementManager::focus(Camera camera, bool checkIfFocusEntityIsMoving __attribute__ ((unused)))
{
	if(isDeleted(camera))
	{
		return;
	}

	// if focusEntity is defined
	Entity focusEntity = Camera::getFocusEntity(camera);

	if(isDeleted(focusEntity))// || !Entity::isTransformed(focusEntity))
	{
		return;
	}

	NormalizedDirection normalizedDirection = Entity::getNormalizedDirection(focusEntity);

	if(NULL == this->focusEntityPosition)
	{
		this->focusEntityPosition = Entity::getPosition(focusEntity);
	}

	// calculate the target position
	Vector3D cameraNewPosition =
	{
		this->focusEntityPosition->x + normalizedDirection.x * this->focusEntityPositionDisplacement.x - __HALF_SCREEN_WIDTH_METERS,
		this->focusEntityPosition->y + normalizedDirection.y * this->focusEntityPositionDisplacement.y - __HALF_SCREEN_HEIGHT_METERS,
		this->focusEntityPosition->z + normalizedDirection.z * this->focusEntityPositionDisplacement.z - __HALF_SCREEN_DEPTH_METERS,
	};

#ifndef __RELEASE
	Vector3D currentCameraPosition = Camera::getPosition(camera);
	Camera::setPosition(camera, cameraNewPosition, true);
	this->lastCameraDisplacement = Vector3D::sub(Camera::getPosition(camera), currentCameraPosition);
#else
	Vector3D currentCameraPosition = *_cameraPosition;
	Camera::setPosition(camera, cameraNewPosition, true);
	this->lastCameraDisplacement = Vector3D::sub(*_cameraPosition, currentCameraPosition);
#endif
}

/**
 * Retrieve the camera's last position displacement
 *
 * @return		Last position displacement vector
 */
Vector3D CameraMovementManager::getLastCameraDisplacement()
{
	return this->lastCameraDisplacement;
}
