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
#include <EntityFactory.h>
#include <Texture.h>
#include <UI.h>
#include <ObjectSpriteContainerManager.h>
#include <ParticleRemover.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Stage_METHODS(ClassName)																		\
		Container_METHODS(ClassName)																	\

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
        /* super's attributes */																		\
        Container_ATTRIBUTES																			\
        /* world's definition pointer */																\
        StageDefinition* stageDefinition;																\
        /* entity factory */																            \
        EntityFactory entityFactory;                                                                    \
        /* particle remover */																            \
        ParticleRemover particleRemover;                                                                \
        /* the stage entities */ 																		\
        VirtualList stageEntities;																		\
        /* the pivot node for streaming */ 																\
        VirtualNode streamingHeadNode;																	\
        /* the stage entities to test for streaming */ 													\
        VirtualList loadedStageEntities;																\
        /* counter to control the streaming phses */													\
        int streamingCycleCounter;																		\
        /* index for method to execute */													            \
        int streamingPhase;                                                                             \
        /* flag to control streaming */ 															    \
        u32 hasRemovedChildren;                                                                      \
        /* the UI */ 																					\
        UI ui;																							\
        /* focus entity: needed for streaming */														\
        InGameEntity focusInGameEntity;																	\
        /* focus entity: previous distance. Used for the streaming */									\
        long previousFocusEntityDistance;																\
        /* next entity's id */																			\
        s16 nextEntityId;																				\

// declare a Stage, which holds the objects in a game world
__CLASS(Stage);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

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
	    u16 minimimSpareMilliSecondsToAllowStreaming;
		u16 loadPadding;
		u16 unloadPadding;
		u16 streamingAmplitude;
		u16 particleRemovalDelayCycles;

	} streaming;

	// rendering
	struct Rendering
	{
		// number of cycles that the texture writing is idle
		int cyclesToWaitForTextureWriting;

		// maximum number of texture's rows to write each time the
		// texture writing is active
		int texturesMaximumRowsToWrite;

		// maximum number of rows to compute
		// on each call to the affine functions
		int maximumAffineRowsToComputePerCall;

        // color config
        ColorConfig colorConfig;

		// palettes' config
		PaletteConfig paletteConfig;

	    // bgmap segments configuration
	    // number of segments reserved for the param tables
		int paramTableSegments;

		// obj segments' sizes (__spt0 to __spt3)
		fix19_13 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		// obj segments z coordinates (__spt0 to __spt3)
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
		// ui's definition
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
void Stage_spawnEntity(Stage this, PositionedEntity* positionedEntity, Container requester, EventListener callback);
Entity Stage_addChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent);
void Stage_removeChild(Stage this, Container child);
void Stage_update(Stage this, u32 elapsedTime);
void Stage_transform(Stage this, const Transformation* environmentTransform);
void Stage_stream(Stage this);
void Stage_streamAll(Stage this);
UI Stage_getUI(Stage this);
void Stage_suspend(Stage this);
void Stage_resume(Stage this);
bool Stage_handlePropagatedMessage(Stage this, int message);
StageDefinition* Stage_getStageDefinition(Stage this);
ParticleRemover Stage_getParticleRemover(Stage this);

#endif
