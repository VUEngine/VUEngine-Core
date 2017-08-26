/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <ScreenMovementManager.h>
#include <Screen.h>
#include <Game.h>
#include <ClockManager.h>
#include <TimerManager.h>
#include <VIPManager.h>

#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ScreenMovementManager
 * @extends Object
 * @ingroup screen
 */
__CLASS_DEFINITION(ScreenMovementManager, Object);
__CLASS_FRIEND_DEFINITION(Screen);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(ScreenMovementManager);

// class's constructor
void __attribute__ ((noinline)) ScreenMovementManager_constructor(ScreenMovementManager this)
{
	ASSERT(this, "ScreenMovementManager::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void ScreenMovementManager_destructor(ScreenMovementManager this)
{
	ASSERT(this, "ScreenMovementManager::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

// center world's screen in function of focus actor's position
void ScreenMovementManager_focus(ScreenMovementManager this __attribute__ ((unused)), u32 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "ScreenMovementManager::update: null this");

	Screen screen = Screen_getInstance();

	// if focusEntity is defined
	if(screen && screen->focusEntity)
	{
		Container focusEntityParent = Container_getParent(__SAFE_CAST(Container, screen->focusEntity));

		if(focusEntityParent)
		{
			// get focusEntity is moving
			if(__VIRTUAL_CALL(SpatialObject, isMoving, screen->focusEntity) || !checkIfFocusEntityIsMoving)
			{
				// save last position
				screen->lastDisplacement = screen->position;

				// get focusEntity's position
				screen->position = *Entity_getPosition(__SAFE_CAST(Entity, screen->focusEntity));

				screen->position.x += screen->focusEntityPositionDisplacement.x - ITOFIX19_13(__HALF_SCREEN_WIDTH);
				screen->position.y += screen->focusEntityPositionDisplacement.y - ITOFIX19_13(__HALF_SCREEN_HEIGHT);
				screen->position.z += screen->focusEntityPositionDisplacement.z - ITOFIX19_13(__HALF_SCREEN_DEPTH);

				if(0 > screen->position.x)
				{
					screen->position.x = 0;
				}
				else if(ITOFIX19_13(screen->stageSize.x) < screen->position.x + ITOFIX19_13(__SCREEN_WIDTH))
				{
					screen->position.x = ITOFIX19_13(screen->stageSize.x - __SCREEN_WIDTH);
				}

				if(0 > screen->position.y)
				{
					screen->position.y = 0;
				}
				else if(ITOFIX19_13(screen->stageSize.y) < screen->position.y + ITOFIX19_13(__SCREEN_HEIGHT))
				{
					screen->position.y = ITOFIX19_13(screen->stageSize.y - __SCREEN_HEIGHT);
				}

				screen->lastDisplacement.x = screen->position.x - screen->lastDisplacement.x;
				screen->lastDisplacement.y = screen->position.y - screen->lastDisplacement.y;
				screen->lastDisplacement.z = screen->position.z - screen->lastDisplacement.z;
			}
			else
			{
				// not moving
				screen->lastDisplacement.x = 0;
				screen->lastDisplacement.y = 0;
				screen->lastDisplacement.z = 0;
			}
		}
	}
}
