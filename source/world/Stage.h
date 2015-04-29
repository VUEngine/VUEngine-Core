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
#include <ObjectSpriteContainerManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


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
		__VIRTUAL_SET(ClassName, Stage, suspend);								\
		__VIRTUAL_SET(ClassName, Stage, resume);								\

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
	/* streaming's preloaded entities */ 										\
	VirtualList entitiesToLoad;													\
																				\
	/* streaming's uninitialized entities */ 									\
	VirtualList entitiesToInitialize;											\
																				\
	/* the UI */ 																\
	UI ui;																		\
																				\
	/* focus entity: needed for streaming */									\
	Entity focusEntity;															\
																				\
	/* next entity's id */														\
	s16 nextEntityId;															\


// declare a Stage, which holds the objects in a game world
__CLASS(Stage);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a game world in ROM memory
typedef struct StageDefinition
{
	// world's size over each axis in pixels
	Size size;
	
	// physical world's gravity
	Acceleration gravity;
	
	// physical world's friction
	fix19_13 friction;

	// OBJs segments z coordinates (SPT0 to SPT3)
	fix19_13 objectSpritesContainersZPosition[__TOTAL_OBJECT_SEGMENTS];

	// initial screen's position inside the game world
	VBVec3D screenPosition;
	
	// engine's optical values structure
	Optical optical;

	// each of the stage's entities
	TextureDefinition** textures;

	// UI's definition
	UIDefinition uiDefinition;

	// each of the stage's entities
	PositionedEntity* entities;

	// pointer to the background music
	const u16 (*bgm)[];

	// stages's identifier
	void* identifier;

	// stages's name
	void* name;

} StageDefinition;

typedef const StageDefinition StageROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Stage);

void Stage_destructor(Stage this);
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList entityNamesToIgnore);
Size Stage_getSize(Stage this);
Entity Stage_addPositionedEntity(Stage this, PositionedEntity* positionedEntity, bool permanent);
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D *position, void *extraInfo, bool permanent);
void Stage_removeEntity(Stage this, Entity entity, bool permanent);
void Stage_update(Stage this);
void Stage_stream(Stage this);
void Stage_streamAll(Stage this);
UI Stage_getUI(Stage this);
void Stage_suspend(Stage this);
void Stage_resume(Stage this);


#endif