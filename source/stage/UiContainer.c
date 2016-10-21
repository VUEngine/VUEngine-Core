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

#include <UiContainer.h>
#include <Optics.h>
#include <Game.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define UiContainer_ATTRIBUTES								    		    		        			\
        /* super's attributes */						    		    			    	        	\
        Container_ATTRIBUTES								    		    			        	    \

// define the UiContainer
__CLASS_DEFINITION(UiContainer, Container);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void UiContainer_constructor(UiContainer this, UiContainerDefinition* uiContainerDefinition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(UiContainer, UiContainerDefinition* uiContainerDefinition)
__CLASS_NEW_END(UiContainer, uiContainerDefinition);

// class's constructor
static void UiContainer_constructor(UiContainer this, UiContainerDefinition* uiContainerDefinition)
{
	ASSERT(this, "UiContainer::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Container, NULL);

	// add entities in the definition
	__VIRTUAL_CALL(UiContainer, addEntities, this, uiContainerDefinition->entities);
}

// class's destructor
void UiContainer_destructor(UiContainer this)
{
	ASSERT(this, "UiContainer::destructor: null this");

	// destroy base
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// add entities
void UiContainer_addEntities(UiContainer this, PositionedEntity* entities)
{
	ASSERT(this, "UiContainer::addEntities: null this");
	ASSERT(entities, "UiContainer::addEntities: null entities");

	static int internalId = 0;
	int i = 0;

	for(;entities && entities[i].entityDefinition; i++)
	{
		Entity entity = Entity_loadEntity(&entities[i], internalId++);

		if(entity)
		{
			// must initialize after adding the children
			__VIRTUAL_CALL(Entity, initialize, entity, true);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(Container, initialTransform, entity, &environmentTransform, true);

			__VIRTUAL_CALL(Entity, ready, entity, true);
		}
	}
}

// transform
void UiContainer_transform(UiContainer this, const Transformation* environmentTransform)
{
	ASSERT(this, "UiContainer::transform: null this");

	Screen screen = Screen_getInstance();
	ASSERT(screen, "UiContainer::transform: null screen");

	Screen_prepareForUITransform(screen);

	Container_transform(__SAFE_CAST(Container, this), environmentTransform);

	Screen_doneUITransform(screen);
}

// transform
void UiContainer_initialTransform(UiContainer this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "UiContainer::initialTransform: null this");

	Screen screen = Screen_getInstance();
	ASSERT(screen, "UiContainer::initialTransform: null screen");

	VBVec3D originalScreenPosition  =
	{
		0, 0, 0
	};

	if(screen)
	{
		// must hack the screen position for my children's sprites
		// being properly rendered
		originalScreenPosition = Screen_getPosition(screen);

		VBVec3D tempScreenPosition =
		{
			0, 0, 0
		};

		Screen_setPosition(screen, tempScreenPosition);
	}

	Container_initialTransform(__SAFE_CAST(Container, this), environmentTransform, recursive);

	__VIRTUAL_CALL(Container, updateVisualRepresentation, this);

	if(screen)
	{
		// recover screen
		Screen_setPosition(screen, originalScreenPosition);
	}
}
