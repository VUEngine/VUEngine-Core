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
#include <VIPManager.h>
#include <Entity.h>
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

	this->lastCameraDisplacement = Vector3D::zero();
}

/**
 * Class destructor
 */
void CameraMovementManager::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Center world's camera in function of focus actor's position
 *
 * @param checkIfFocusEntityIsMoving	Flag whether to check if the focus Entity is moving
 */
void CameraMovementManager::focus(Camera camera, uint32 checkIfFocusEntityIsMoving __attribute__ ((unused)))
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

	Vector3D focusEntityPosition = Camera::getFocusEntityPosition(camera);
	Vector3D focusEntityPositionDisplacement = Camera::getFocusEntityPositionDisplacement(camera);

	// calculate the target position
	Vector3D cameraNewPosition =
	{
		focusEntityPosition.x + normalizedDirection.x * focusEntityPositionDisplacement.x - __HALF_SCREEN_WIDTH_METERS,
		focusEntityPosition.y + normalizedDirection.y * focusEntityPositionDisplacement.y - __HALF_SCREEN_HEIGHT_METERS,
		focusEntityPosition.z + normalizedDirection.z * focusEntityPositionDisplacement.z - __HALF_SCREEN_DEPTH_METERS,
	};

	Vector3D currentCameraPosition = Camera::getPosition(camera);

	Camera::setPosition(camera, cameraNewPosition, true);

	this->lastCameraDisplacement = Vector3D::sub(Camera::getPosition(camera), currentCameraPosition);
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
