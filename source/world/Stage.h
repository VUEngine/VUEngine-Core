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

#ifndef STAGE_H_
#define STAGE_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <config.h>
#include <Container.h>
#include <Entity.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */
	
// declare the virtual methods
#define Stage_METHODS								\
		Container_METHODS							\


// declare the virtual methods which are redefined
#define Stage_SET_VTABLE(ClassName)							\
		Container_SET_VTABLE(ClassName)						\


#define Stage_ATTRIBUTES						\
												\
	/* super's attributes */					\
	Container_ATTRIBUTES;						\
												\
	/* world's definition pointer */			\
	StageDefinition* stageDefinition;			\
												\
	/* this allows to determine which */ 		\
	/* entities to load from a */ 				\
	/* world definition */						\
	VirtualList entityState;					\
												\
	/* flag to know if the stage must */		\
	/* track entities's load states */			\
	int saveEntityStates:1;


// declare a Stage, which holds the objects in a game world
__CLASS(Stage);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// a actor asociated with a position
typedef const struct PositionedEntity{
	
	// pointer to the entity definition in ROM
	EntityDefinition* entity;
	
	// position in the world
	VBVec3DReal position;

	// extra info
	void* extraInfo;
	
}PositionedEntity;

/* ---------------------------------------------------------------------------------------------------------*/
// defines a game world in ROM memory
typedef struct  StageDefinition{
	
	// world's size over each axis in pixels
	Size size;
	
	// initial screen's position inside the game world
	VBVec3D screenPosition;
	
	// pointer to the background music
	const u16 (*bgm)[];

	// each of the stage's entities
	PositionedEntity entities[__ENTITIESPERWORLD];

}StageDefinition;

typedef const StageDefinition StageROMDef;


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Stage);

// class's destructo
void Stage_destructor(Stage this);

// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition);

// load objects on demand (if they aren't loaded and are visible)
void Stage_loadEntities(Stage this);

// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D *position, int inGameIndex, void *extraInfo);

// unload non visible entities
void Stage_unloadEntities(Stage this);

// set save entity's states flag
int Stage_saveEntityStates(Stage this);

#endif

