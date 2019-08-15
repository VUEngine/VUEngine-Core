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
#include <UIContainer.h>
#include <ParticleRemover.h>
#include <VIPManager.h>
#include <Camera.h>
#include <SpriteManager.h>
#include <TimerManager.h>
#include <SoundManager.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

// defines a game world in ROM memory
typedef struct StageSpec
{
	/// class allocator
	AllocatorPointer allocator;

	// Timer config
	struct Timer
	{
		u16 resolution;
		u16 timePerInterrupt;
		u16 timePerInterruptUnits;

	} timer;

	// Sound config
	struct SoundConfig
	{
		u16 pcmTargetPlaybackFrameRate;
		bool deferMIDIPlayback;
		
	} sound;

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

		// object segments sizes (__spt0 to __spt3)
		s16 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		// object segments z coordinates (__spt0 to __spt3)
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
		FontSpec** fontSpecs;

		// char sets for preloading
		CharSetSpec** charSetSpecs;

		// textures for preloading
		TextureSpec** textureSpecs;

		// pointer to the background sounds
		Sound** sounds;

	} assets;

	struct Entities
	{
		// ui's spec
		UIContainerSpec uiContainerSpec;

		// each of the stage's entities
		PositionedEntity* children;

	} entities;

	// post processing effects
	PostProcessingEffect* postProcessingEffects;

} StageSpec;

typedef const StageSpec StageROMSpec;

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
	// world's spec pointer
	StageSpec* stageSpec;
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
	// The sounds
	VirtualList soundWrappers;
	// counter to control the streaming phases
	int streamingCycleCounter;
	// index for method to execute
	int streamingPhase;
	// flag to control streaming
	u32 hasRemovedChildren;
	// the ui container
	UIContainer uiContainer;
	// focus entity: needed for streaming
	Entity focusEntity;
	// camera's previous distance. Used for the streaming
	u32 cameraPreviousDistance;
	// next entity's id
	s16 nextEntityId;

	/// @publicsection
	void constructor(StageSpec* stageSpec);
	void setupPalettes();
	void loadPostProcessingEffects();
	void setupTimer();
	Size getSize();
	CameraFrustum getCameraFrustum();
	bool registerEntityId(s16 internalId, EntitySpec* entitySpec);
	void spawnEntity(PositionedEntity* positionedEntity, Container requester, EventListener callback);
	Entity addChildEntity(const PositionedEntity* const positionedEntity, bool permanent);
	UIContainer getUIContainer();
	StageSpec* getStageSpec();
	ParticleRemover getParticleRemover();
	void showStreamingProfiling(int x, int y);
	bool unloadOutOfRangeEntities(int defer);
    bool loadInRangeEntities(int defer);
	Entity findChildByInternalId(s16 internalId);
	bool updateEntityFactory();
	VirtualList getSoundWrappers();

	virtual void load(VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition);
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
