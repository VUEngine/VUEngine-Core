/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <UI.h>
#include <Optics.h>
#include <Game.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define UI_ATTRIBUTES							\
												\
	/* super's attributes */					\
	Container_ATTRIBUTES;						\



// define the UI
__CLASS_DEFINITION(UI);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void UI_constructor(UI this, UIDefinition* uiDefinition);
	
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S ATTRIBUTES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(UI, __PARAMETERS(UIDefinition* uiDefinition))
__CLASS_NEW_END(UI, __ARGUMENTS(uiDefinition));


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void UI_constructor(UI this, UIDefinition* uiDefinition){

	ASSERT(this, "UI::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Container, __ARGUMENTS(-1));
	
	// add entities in the definition
	__VIRTUAL_CALL(void, UI, addEntities, this, __ARGUMENTS(uiDefinition->entities));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void UI_destructor(UI this){

	ASSERT(this, "UI::destructor: null this");

	// destroy base
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entities
void UI_addEntities(UI this, PositionedEntity* entities){
	
	ASSERT(this, "UI::addEntities: null this");
	ASSERT(entities, "UI::addEntities: null entities");

	static int ID = 0;
	int i = 0;
	for(;entities[i].entityDefinition; i++){
		
		Entity entity = Entity_load(entities[i].entityDefinition, ID++, entities[i].extraInfo);

		Container_addChild((Container)this, (Container)entity);

		VBVec3D position = {
				ITOFIX19_13(entities[i].position.x),
				ITOFIX19_13(entities[i].position.y),
				ITOFIX19_13(entities[i].position.z)
		};

		// set spatial position
		__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(position));
		
	}
}
