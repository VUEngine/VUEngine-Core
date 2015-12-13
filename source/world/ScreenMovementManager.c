/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ScreenMovementManager.h>
#include <Screen.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


// define the ScreenMovementManager
__CLASS_DEFINITION(ScreenMovementManager, Object);

__CLASS_FRIEND_DEFINITION(Screen);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void ScreenMovementManager_FXFadeIn(ScreenMovementManager this, int duration);
void ScreenMovementManager_FXFadeOut(ScreenMovementManager this, int duration);


//---------------------------------------------------------------------------------------------------------
// 												GLOBALS
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(ScreenMovementManager);

// class's constructor
void ScreenMovementManager_constructor(ScreenMovementManager this)
{
	ASSERT(this, "ScreenMovementManager::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();
}

// class's destructor
void ScreenMovementManager_destructor(ScreenMovementManager this)
{
	ASSERT(this, "ScreenMovementManager::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

// center world's screen in function of focus actor's position
void ScreenMovementManager_position(ScreenMovementManager this, u8 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "ScreenMovementManager::update: null this");

	Screen screen = Screen_getInstance();
	
	// if focusInGameEntity is defined
	if(screen && screen->focusInGameEntity)
	{
		Container focusInGameEntityParent = Container_getParent(__GET_CAST(Container, screen->focusInGameEntity));
		
		if(focusInGameEntityParent)
		{
			// transform focus entity
			Transformation environmentTransform = Container_getEnvironmentTransform(focusInGameEntityParent);

			// apply transformations
			__VIRTUAL_CALL(void, Container, transform, screen->focusInGameEntity, &environmentTransform);
	
			// get focusInGameEntity is moving
			if(__VIRTUAL_CALL(bool, InGameEntity, isMoving, screen->focusInGameEntity) || !checkIfFocusEntityIsMoving)
			{
				// save last position
				screen->lastDisplacement = screen->position;
	
				// get focusInGameEntity's position
				screen->position = *Entity_getPosition(__GET_CAST(Entity, screen->focusInGameEntity));
				
				screen->position.x += screen->focusEntityPositionDisplacement.x - ITOFIX19_13(__SCREEN_WIDTH >> 1);
				screen->position.y += screen->focusEntityPositionDisplacement.y - ITOFIX19_13(__SCREEN_HEIGHT >> 1);
				screen->position.z += screen->focusEntityPositionDisplacement.z - ITOFIX19_13(__SCREEN_DEPTH >> 1);
	
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

void ScreenMovementManager_startEffect(ScreenMovementManager this, int effect, int duration)
{
	ASSERT(this, "ScreenMovementManager::startEffect: null this");
	
	switch(effect)
	{
		case kFadeIn:
			
			ScreenMovementManager_FXFadeIn(this, duration);
			break;

		case kFadeOut:
			
			ScreenMovementManager_FXFadeOut(this, duration);
			break;
	}
}

void ScreenMovementManager_stopEffect(ScreenMovementManager this, int effect)
{
	ASSERT(this, "ScreenMovementManager::stopEffect: null this");
}

// fade in the screen
void ScreenMovementManager_FXFadeIn(ScreenMovementManager this, int duration)
{
	ASSERT(this, "ScreenMovementManager::FXFadeIn: null this");

#ifdef __DEBUG
	return;
#endif

	int i = 0;
	// create the delay
	for(; i <= 32; i += 2)
	{
		if(duration)
		{
			// create time delay
			Clock_delay(Game_getClock(Game_getInstance()), duration);
		}

		// increase brightness
		SET_BRIGHT(i, i << 1, i);
	}
}

// fade out the screen
void ScreenMovementManager_FXFadeOut(ScreenMovementManager this, int duration)
{
	ASSERT(this, "ScreenMovementManager::FXFadeOut: null this");

#ifdef __DEBUG
	return;
#endif
	int i = 32;

	// create the delay
	for(; i >= 0; i-=2)
	{
		if(duration)
		{
			// create time delay
			Clock_delay(Game_getClock(Game_getInstance()), duration);
		}
		// decrease brightness
		SET_BRIGHT(i, i << 1, i);
	}
}
