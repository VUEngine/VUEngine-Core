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

#ifndef STAGE_H_
#define STAGE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <InGameEntity.h>
#include <Texture.h>
#include <UI.h>
#include <ObjectSpriteContainerManager.h>
#include <VPUManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Stage_METHODS																					\
		Container_METHODS																				\

// declare the virtual methods which are redefined
#define Stage_SET_VTABLE(ClassName)																		\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Stage, update);														\
		__VIRTUAL_SET(ClassName, Stage, transform);														\
		__VIRTUAL_SET(ClassName, Stage, suspend);														\
		__VIRTUAL_SET(ClassName, Stage, resume);														\
		__VIRTUAL_SET(ClassName, Stage, removeChild);													\
		__VIRTUAL_SET(ClassName, Stage, handlePropagatedMessage);										\

#define Stage_ATTRIBUTES																				\
																										\
	/* super's attributes */																			\
	Container_ATTRIBUTES;																				\
																										\
	/* world's definition pointer */																	\
	StageDefinition* stageDefinition;																	\
																										\
	/* the stage entities */ 																			\
	VirtualList stageEntities;																			\
																										\
	/* the stage entities to test for streaming */ 														\
	VirtualList stageEntitiesToTest;																	\
																										\
	/* the stage entities to test for streaming */ 														\
	VirtualList loadedStageEntities;																	\
																										\
	/* the removed entities */ 																			\
	VirtualList removedEntities;																		\
																										\
	/* streaming's preloaded entities */ 																\
	VirtualList entitiesToLoad;																			\
																										\
	/* streaming's uninitialized entities */ 															\
	VirtualList entitiesToInitialize;																	\
																										\
	/* streaming's non yet transformed entities */ 														\
	VirtualList entitiesToTransform;																	\
																										\
	/* the UI */ 																						\
	UI ui;																								\
																										\
	/* focus entity: needed for streaming */															\
	InGameEntity focusInGameEntity;																		\
																										\
	/* next entity's id */																				\
	s16 nextEntityId;																					\

// declare a Stage, which holds the objects in a game world
__CLASS(Stage);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------


// defines a game world in ROM memory
typedef struct StageTextureEntryDefinition
{
	TextureDefinition* textureDefinition;

	// is a managed texture
	bool isManaged;
	
} StageTextureEntryDefinition;

typedef const StageTextureEntryDefinition StageTextureEntryROMDef;

// defines a game world in ROM memory
typedef struct StageDefinition
{
	// general level's attributes
	struct Level
	{
		// level's size
		Size size;
		
		// screen's initial position inside the game world
		VBVec3D screenInitialPosition;
		
	} level;

	// streaming
	struct Streaming 
	{
		u8 delayPerCycle;
		u8 loadPadding;
		u8 unloadPadding;
		u8 streamingAmplitude;
		
	} streaming;
	
	// rendering
	struct Rendering
	{
		// number of cycles that the texture writing is idle
		u8 cyclesToWaitForTextureWriting;
		
		// maximum number of texture's rows to write each time the 
		// texture writing is active
		u8 texturesMaximumRowsToWrite;
		
		// palettes' config
		PaletteConfig paletteConfig;
		
	    // BGMAP segments configuration
	    // number of segments reserved for dynamically allocated textures when preloading
		u8 spareBgmapSegments;

		// OBJs segments's sizes(SPT0 to SPT3)
		fix19_13 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		// OBJs segments z coordinates (SPT0 to SPT3)
		fix19_13 objectSpritesContainersZPosition[__TOTAL_OBJECT_SEGMENTS];
		
		// engine's optical values structure
		Optical optical;

	} rendering;

	struct Physics 
	{
		// physical world's gravity
		Acceleration gravity;
		
		// physical world's friction
		fix19_13 friction;
		
	} physics;
	

	struct Assets
	{
		// char sets for preloading
		CharSetDefinition** charSets;

		// textures for preloading
		StageTextureEntryDefinition* stageTextureEntryDefinitions;

		// pointer to the background music
		const u16 (*bgm)[];

	} assets;

	struct Entities
	{
		// UI's definition
		UIDefinition uiDefinition;
	
		// each of the stage's entities
		PositionedEntity* children;
		
	} entities;

} StageDefinition;

typedef const StageDefinition StageROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Stage);

void Stage_destructor(Stage this);
void Stage_setupPalettes(Stage this);
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList entityNamesToIgnore, bool overrideScreenPosition);
Size Stage_getSize(Stage this);
bool Stage_registerEntityId(Stage this, s16 id, EntityDefinition* entityDefinition);
Entity Stage_addPositionedEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent);
Entity Stage_addEntity(Stage this, const EntityDefinition* const entityDefinition, const char* const name, const VBVec3D* const position, void* const extraInfo, bool permanent);
void Stage_removeChild(Stage this, Container child);
void Stage_update(Stage this);
void Stage_transform(Stage this, const Transformation* environmentTransform);
void Stage_stream(Stage this);
void Stage_streamAll(Stage this);
UI Stage_getUI(Stage this);
void Stage_suspend(Stage this);
void Stage_resume(Stage this);
bool Stage_handlePropagatedMessage(Stage this, int message);


#endif