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

#ifndef STAGE_H_
#define STAGE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>
#include <Texture.h>
#include <UI.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Stage_METHODS															\
		Container_METHODS														\


// declare the virtual methods which are redefined
#define Stage_SET_VTABLE(ClassName)												\
		Container_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Stage, update);								\

#define Stage_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Container_ATTRIBUTES;														\
																				\
	/* world's definition pointer */											\
	StageDefinition* stageDefinition;											\
																				\
	/* the stage entities */ 													\
	VirtualList stageEntities;													\
																				\
	/* the stage entities to test for streaming */ 								\
	VirtualList stageEntitiesToTest;											\
																				\
	/* the stage entities to test for streaming */ 								\
	VirtualList loadedStageEntities;											\
																				\
	/* the removed entities */ 													\
	VirtualList removedEntities;												\
																				\
	/* the UI */ 																\
	UI ui;																		\
																				\
	/* flag to know if the stage must */										\
	/* flush unused char groups */												\
	int flushCharGroups;														\
																				\
	/* focus entity: needed for streaming */									\
	Entity focusEntity;															\

// declare a Stage, which holds the objects in a game world
__CLASS(Stage);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a game world in ROM memory
typedef struct  StageDefinition
{
	// world's size over each axis in pixels
	Size size;

	// initial screen's position inside the game world
	VBVec3D screenPosition;

	// each of the stage's entities
	TextureDefinition** textures;

	// UI's definition
	UIDefinition uiDefinition;

	// each of the stage's entities
	PositionedEntity* entities;

	// pointer to the background music
	const u16 (*bgm)[];

}StageDefinition;

typedef const StageDefinition StageROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

// class's allocator
__CLASS_NEW_DECLARE(Stage);

// class's destructo
void Stage_destructor(Stage this);

// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities);

// retrieve size
Size Stage_getSize(Stage this);

// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D *position, void *extraInfo, int permanent);

// add entity to the stage
void Stage_removeEntity(Stage this, Entity entity, int permanent);

// execute stage's logic
void Stage_update(Stage this);

// stream entities according to screen's position
void Stage_stream(Stage this);

// stream entities according to screen's position
void Stage_streamAll(Stage this);

// if set to true, the char set memory is flushed when
// a char defintion is no longer used
// only useful to false when preloading textures
// otherwise it doesn't have any effect and flushing
// is the default  behvior
void Stage_setFlushCharGroups(Stage this, int flushCharGroups);

// retrieve ui
UI Stage_getUI(Stage this);

#endif

