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
void CameraMovementManager::focus(u32 checkIfFocusEntityIsMoving __attribute__ ((unused)))
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
		focusEntityPosition.x + direction.x * focusEntityPositionDisplacement.x - __PIXELS_TO_METERS(__HALF_SCREEN_WIDTH),
		focusEntityPosition.y + direction.y * focusEntityPositionDisplacement.y - __PIXELS_TO_METERS(__HALF_SCREEN_HEIGHT),
		focusEntityPosition.z + direction.z * focusEntityPositionDisplacement.z - __PIXELS_TO_METERS(__HALF_SCREEN_DEPTH),
	};

	Camera::setPosition(camera, cameraNewPosition);
}
