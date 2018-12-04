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
void CameraMovementManager::focus(u32 checkIfFocusEntityIsMoving)
{
	Camera camera = Camera::getInstance();

	// if focusEntity is defined
	if(camera && camera->focusEntity)
	{
		Container focusEntityParent = Container::getParent(camera->focusEntity);

		if(focusEntityParent)
		{
			// get focusEntity is moving
			if( SpatialObject::isMoving(camera->focusEntity) || !checkIfFocusEntityIsMoving)
			{
				// save last position
				camera->lastDisplacement = camera->position;

				// get focusEntity's position
				camera->position = *Entity::getPosition(camera->focusEntity);

				camera->position.x += camera->focusEntityPositionDisplacement.x - __I_TO_FIX10_6(__HALF_SCREEN_WIDTH);
				camera->position.y += camera->focusEntityPositionDisplacement.y - __I_TO_FIX10_6(__HALF_SCREEN_HEIGHT);
				camera->position.z += camera->focusEntityPositionDisplacement.z - __I_TO_FIX10_6(__HALF_SCREEN_DEPTH);

				if(0 > camera->position.x)
				{
					camera->position.x = 0;
				}
				else if(__I_TO_FIX10_6(camera->stageSize.x) < camera->position.x + __I_TO_FIX10_6(__SCREEN_WIDTH))
				{
					camera->position.x = __I_TO_FIX10_6(camera->stageSize.x - __SCREEN_WIDTH);
				}

				if(0 > camera->position.y)
				{
					camera->position.y = 0;
				}
				else if(__I_TO_FIX10_6(camera->stageSize.y) < camera->position.y + __I_TO_FIX10_6(__SCREEN_HEIGHT))
				{
					camera->position.y = __I_TO_FIX10_6(camera->stageSize.y - __SCREEN_HEIGHT);
				}

				camera->lastDisplacement.x = camera->position.x - camera->lastDisplacement.x;
				camera->lastDisplacement.y = camera->position.y - camera->lastDisplacement.y;
				camera->lastDisplacement.z = camera->position.z - camera->lastDisplacement.z;
			}
			else
			{
				// not moving
				camera->lastDisplacement.x = 0;
				camera->lastDisplacement.y = 0;
				camera->lastDisplacement.z = 0;
			}
		}
	}
}

/**
 * Tells me that the focus entity for the camera has been set
 *
 * @param focusEntity	Focus entity
 */
void CameraMovementManager::focusGameEntitySet(Entity focusEntity)
{
}
