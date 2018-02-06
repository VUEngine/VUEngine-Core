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

/**
 * @class	CameraMovementManager
 * @extends Object
 * @ingroup camera
 */
__CLASS_DEFINITION(CameraMovementManager, Object);
__CLASS_FRIEND_DEFINITION(Camera);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CameraMovementManager_getInstance()
 * @memberof	CameraMovementManager
 * @public
 *
 * @return		CameraMovementManager instance
 */
__SINGLETON(CameraMovementManager);

/**
 * Class constructor
 *
 * @memberof	CameraMovementManager
 * @public
 *
 * @param this	Function scope
 */
void __attribute__ ((noinline)) CameraMovementManager_constructor(CameraMovementManager this)
{
	ASSERT(this, "CameraMovementManager::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	CameraMovementManager
 * @public
 *
 * @param this	Function scope
 */
void CameraMovementManager_destructor(CameraMovementManager this)
{
	ASSERT(this, "CameraMovementManager::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Center world's camera in function of focus actor's position
 *
 * @memberof							CameraMovementManager
 * @public
 *
 * @param this							Function scope
 * @param checkIfFocusEntityIsMoving	Flag whether to check if the focus Entity is moving
 */
void CameraMovementManager_focus(CameraMovementManager this __attribute__ ((unused)), u32 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "CameraMovementManager::update: null this");

	Camera camera = Camera_getInstance();

	// if focusEntity is defined
	if(camera && camera->focusEntity)
	{
		Container focusEntityParent = Container_getParent(__SAFE_CAST(Container, camera->focusEntity));

		if(focusEntityParent)
		{
			// get focusEntity is moving
			if(__VIRTUAL_CALL(SpatialObject, isMoving, camera->focusEntity) || !checkIfFocusEntityIsMoving)
			{
				// save last position
				camera->lastDisplacement = camera->position;

				// get focusEntity's position
				camera->position = *Entity_getPosition(__SAFE_CAST(Entity, camera->focusEntity));

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
