/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

// defines a game world in ROM memory
typedef struct StageDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	// general level's attributes
	struct Level
	{
		// level's size in pixels
		PixelSize pixelSize;

		// camera's initial position inside the game world
		PixelVector cameraInitialPosition;

		// camera's frustum
		CameraFrustum cameraFrustum;

	} level;

	// streaming
	struct Streaming
	{
		u16 loadPadding;
		u16 unloadPadding;
		u16 streamingAmplitude;
		u16 particleRemovalDelayCycles;
		bool deferred;

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

		// OBJECT segments' sizes (__spt0 to __spt3)
		s16 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		// OBJECT segments z coordinates (__spt0 to __spt3)
		s16 objectSpritesContainersZPosition[__TOTAL_OBJECT_SEGMENTS];

		// engine's optical values structure
		PixelOptical pixelOptical;

	} rendering;

	struct Physics
	{
		// physical world's gravity
		Acceleration gravity;

		// physical world's friction coefficient
		fix10_6 frictionCoefficient;

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

/**
 * Stage Entity Description
 *
 * @memberof Stage
 */
typedef struct StageEntityDescription
{
	PixelRightBox pixelRightBox;
	PositionedEntity* positionedEntity;
	u32 distance;
	s16 internalId;

} StageEntityDescription;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class Stage : Container
{
	// world's definition pointer
	StageDefinition* stageDefinition;
	// entity factory
	EntityFactory entityFactory;
	// particle remover
	ParticleRemover particleRemover;
	// the stage entities
	VirtualList stageEntities;
	// the pivot node for streaming
	VirtualNode streamingHeadNode;
	// the stage entities to test for streaming
	VirtualList loadedStageEntities;
	// counter to control the streaming phases
	int streamingCycleCounter;
	// index for method to execute
	int streamingPhase;
	// flag to control streaming
	u32 hasRemovedChildren;
	// the ui container
	UiContainer uiContainer;
	// focus entity: needed for streaming
	Entity focusEntity;
	// camera's previous distance. Used for the streaming
	u32 cameraPreviousDistance;
	// next entity's id
	s16 nextEntityId;

	/// @publicsection
	void constructor(StageDefinition* stageDefinition);
	void setupPalettes();
	void load(VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition);
	void loadPostProcessingEffects();
	Size getSize();
	CameraFrustum getCameraFrustum();
	bool registerEntityId(s16 internalId, EntityDefinition* entityDefinition);
	void spawnEntity(PositionedEntity* positionedEntity, Container requester, EventListener callback);
	Entity addChildEntity(const PositionedEntity* const positionedEntity, bool permanent);
	UiContainer getUiContainer();
	StageDefinition* getStageDefinition();
	ParticleRemover getParticleRemover();
	void showStreamingProfiling(int x, int y);
	bool unloadOutOfRangeEntities(int defer);
    bool loadInRangeEntities(int defer);
	virtual bool stream();
	virtual void streamAll();
	override void update(u32 elapsedTime);
	override void transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void synchronizeGraphics();
	override void suspend();
	override void resume();
	override void removeChild(Container child, bool deleteChild);
	override bool handlePropagatedMessage(int message);
}


#endif
