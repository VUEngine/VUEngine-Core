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
#include <Game.h>
#include <ClockManager.h>
#include <TimerManager.h>
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
void CameraMovementManager::focus(uint32 checkIfFocusEntityIsMoving __attribute__ ((unused)))
{
	Camera camera = Camera::getInstance();

	// if focusEntity is defined
	Entity focusEntity = Camera::getFocusEntity(camera);

	if(isDeleted(focusEntity) || !Entity::isTransformed(focusEntity))
	{
		return;
	}

	Direction direction = Entity::getDirection(focusEntity);

	Vector3D focusEntityPosition = Camera::getFocusEntityPosition(camera);
	Vector3D focusEntityPositionDisplacement = Camera::getFocusEntityPositionDisplacement(camera);

	// calculate the target position
	Vector3D cameraNewPosition =
	{
		focusEntityPosition.x + direction.x * focusEntityPositionDisplacement.x - __HALF_SCREEN_WIDTH_METERS,
		focusEntityPosition.y + direction.y * focusEntityPositionDisplacement.y - __HALF_SCREEN_HEIGHT_METERS,
		focusEntityPosition.z + direction.z * focusEntityPositionDisplacement.z - __HALF_SCREEN_DEPTH_METERS,
	};

	Camera::setPosition(camera, cameraNewPosition, true);
	Camera::setRotation(camera, Camera::getFocusEntityRotation(camera));
}
