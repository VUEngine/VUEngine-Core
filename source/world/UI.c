/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <UI.h>
#include <Optics.h>
#include <Game.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define UI_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Container_ATTRIBUTES;														\

// define the UI
__CLASS_DEFINITION(UI, Container);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

//class's constructor
static void UI_constructor(UI this, UIDefinition* uiDefinition);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(UI, UIDefinition* uiDefinition)
__CLASS_NEW_END(UI, uiDefinition);

// class's constructor
static void UI_constructor(UI this, UIDefinition* uiDefinition)
{
	ASSERT(this, "UI::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(-1);

	// add entities in the definition
	__VIRTUAL_CALL(void, UI, addEntities, this, uiDefinition->entities);
}

// class's destructor
void UI_destructor(UI this)
{
	ASSERT(this, "UI::destructor: null this");

	// destroy base
	__DESTROY_BASE;
}

// add entities
void UI_addEntities(UI this, PositionedEntity* entities)
{
	ASSERT(this, "UI::addEntities: null this");
	ASSERT(entities, "UI::addEntities: null entities");

	static int ID = 0;
	int i = 0;
	
	// static to avoid call to memcpy
	Transformation environmentTransform = Container_getEnvironmentTransform(__GET_CAST(Container, this));

	for (;entities && entities[i].entityDefinition; i++)
	{
		Entity entity = Entity_loadFromDefinition(&entities[i], ID++);

		__VIRTUAL_CALL(void, Entity, initialize, entity);

		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, entity, environmentTransform);

		Container_addChild(__GET_CAST(Container, this), __GET_CAST(Container, entity));
	}
}

// transform
void UI_transform(UI this, const Transformation* environmentTransform)
{
	ASSERT(this, "UI::transform: null this");

	Screen screen = Screen_getInstance();
	ASSERT(screen, "UI::transform: null screen");

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

	Container_transform(__GET_CAST(Container, this), environmentTransform);
	
	if(screen)
	{
		// recover screen
		Screen_setPosition(screen, originalScreenPosition);
	}
}

// transform
void UI_initialTransform(UI this, Transformation* environmentTransform)
{
	ASSERT(this, "UI::initialTransform: null this");

	Screen screen = Screen_getInstance();
	ASSERT(screen, "UI::initialTransform: null screen");

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

	Container_initialTransform(__GET_CAST(Container, this), environmentTransform);
	
	if(screen)
	{
		// recover screen
		Screen_setPosition(screen, originalScreenPosition);
	}
}