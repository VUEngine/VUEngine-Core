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

#ifndef STAGE_H_
#define STAGE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>
#include <Printing.h>
#include <EntityFactory.h>
#include <Texture.h>
#include <UiContainer.h>
#include <ObjectSpriteContainerManager.h>
#include <ParticleRemover.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Stage_METHODS(ClassName)																		\
		Container_METHODS(ClassName)																	\

// declare the virtual methods which are redefined
#define Stage_SET_VTABLE(ClassName)																		\
		Container_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Stage, update);														\
		__VIRTUAL_SET(ClassName, Stage, transform);														\
		__VIRTUAL_SET(ClassName, Stage, synchronizeGraphics);											\
		__VIRTUAL_SET(ClassName, Stage, suspend);														\
		__VIRTUAL_SET(ClassName, Stage, resume);														\
		__VIRTUAL_SET(ClassName, Stage, removeChild);													\
		__VIRTUAL_SET(ClassName, Stage, handlePropagatedMessage);										\

#define Stage_ATTRIBUTES																				\
		/* super's attributes */																		\
		Container_ATTRIBUTES																			\
		/* world's definition pointer */																\
		StageDefinition* stageDefinition;																\
		/* entity factory */																			\
		EntityFactory entityFactory;																	\
		/* particle remover */																			\
		ParticleRemover particleRemover;																\
		/* the stage entities */ 																		\
		VirtualList stageEntities;																		\
		/* the pivot node for streaming */ 																\
		VirtualNode streamingHeadNode;																	\
		/* the stage entities to test for streaming */ 													\
		VirtualList loadedStageEntities;																\
		/* counter to control the streaming phses */													\
		int streamingCycleCounter;																		\
		/* index for method to execute */																\
		int streamingPhase;																				\
		/* flag to control streaming */ 																\
		u32 hasRemovedChildren;																			\
		/* the ui container */ 																			\
		UiContainer uiContainer;																		\
		/* focus entity: needed for streaming */														\
		Entity focusEntity;																	\
		/* screen's previous distance. Used for the streaming */										\
		long screenPreviousDistance;																	\
		/* next entity's id */																			\
		s16 nextEntityId;																				\

__CLASS(Stage);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

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

		// maximum number of texture's rows to write each time the texture writing is active
		int texturesMaximumRowsToWrite;

		// maximum number of rows to compute on each call to the affine functions
		int maximumAffineRowsToComputePerCall;

		// color config
		ColorConfig colorConfig;

		// palettes' config
		PaletteConfig paletteConfig;

		// bgmap segments configuration
		// number of segments reserved for the param tables
		int paramTableSegments;

		// obj segments' sizes (__spt0 to __spt3)
		s16 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

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
		// fonts for preloading
		FontDefinition** fontDefinitions;

		// char sets for preloading
		CharSetDefinition** charSetDefinitions;

		// textures for preloading
		TextureDefinition** textureDefinitions;

		// pointer to the background music
		const u16 (*bgm)[];

	} assets;

	struct Entities
	{
		// ui's definition
		UiContainerDefinition uiContainerDefinition;

		// each of the stage's entities
		PositionedEntity* children;

	} entities;

	// post processing effects
	PostProcessingEffect* postProcessingEffects;

} StageDefinition;

typedef const StageDefinition StageROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Stage);

void Stage_destructor(Stage this);
void Stage_setupPalettes(Stage this);
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideScreenPosition);
void Stage_loadPostProcessingEffects(Stage this);
Size Stage_getSize(Stage this);
bool Stage_registerEntityId(Stage this, s16 internalId, EntityDefinition* entityDefinition);
void Stage_spawnEntity(Stage this, PositionedEntity* positionedEntity, Container requester, EventListener callback);
Entity Stage_addChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent);
void Stage_removeChild(Stage this, Container child);
void Stage_update(Stage this, u32 elapsedTime);
void Stage_transform(Stage this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void Stage_synchronizeGraphics(Stage this);
bool Stage_stream(Stage this);
void Stage_streamAll(Stage this);
UiContainer Stage_getUiContainer(Stage this);
void Stage_suspend(Stage this);
void Stage_resume(Stage this);
bool Stage_handlePropagatedMessage(Stage this, int message);
StageDefinition* Stage_getStageDefinition(Stage this);
ParticleRemover Stage_getParticleRemover(Stage this);
void Stage_showStreamingProfiling(Stage this, int x, int y);


#endif
