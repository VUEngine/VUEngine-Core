/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

typedef struct Streaming
{
	uint16 loadPadding;
	uint16 unloadPadding;
	uint16 streamingAmplitude;
	uint16 particleRemovalDelayCycles;
	bool deferred;

} Streaming;

// defines a game world in ROM memory
typedef struct StageSpec
{
	/// class allocator
	AllocatorPointer allocator;

	// Timer config
	struct Timer
	{
		uint16 resolution;
		uint16 timePerInterrupt;
		uint16 timePerInterruptUnits;

	} timer;

	// Sound config
	struct SoundConfig
	{
		uint16 pcmTargetPlaybackFrameRate;
		uint16 MIDIPlaybackCounterPerInterrupt;

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
	Streaming streaming;

	// rendering
	struct Rendering
	{
		// maximum number of texture's rows to write each time the texture writing is active
		int32 texturesMaximumRowsToWrite;

		// maximum number of rows to compute on each call to the affine functions
		int32 maximumAffineRowsToComputePerCall;

		// color config
		ColorConfig colorConfig;

		// palettes' config
		PaletteConfig paletteConfig;

		// bgmap segments configuration
		// number of segments reserved for the param tables
		int32 paramTableSegments;

		// object segments sizes (__spt0 to __spt3)
		int16 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		// object segments z coordinates (__spt0 to __spt3)
		int16 objectSpritesContainersZPosition[__TOTAL_OBJECT_SEGMENTS];

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
	uint32 distance;
	int16 internalId;

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
	// The sounds
	VirtualList soundWrappers;
	// List of listeners for entity loading
	VirtualList entityLoadingListeners;
	// Streaming settings
	Streaming streaming;
	// counter to control the streaming phases
	int32 streamingCycleCounter;
	// index for method to execute
	int32 streamingPhase;
	// flag to control streaming
	uint32 hasRemovedChildren;
	// the ui container
	UIContainer uiContainer;
	// focus entity: needed for streaming
	Entity focusEntity;
	// camera's previous distance. Used for the streaming
	uint32 cameraPreviousDistance;
	// next entity's id
	int16 nextEntityId;
	// flag to prevent loading entities that are within the screen's space
	bool forceNoPopIn;

	/// @publicsection
	void constructor(StageSpec* stageSpec);
	void setupPalettes();
	void loadPostProcessingEffects();
	void setupTimer();
	Size getSize();
	PixelSize getPixelSize();
	CameraFrustum getCameraFrustum();
	void addEntityLoadingListener(Object context, EventListener callback);
	bool registerEntityId(int16 internalId, EntitySpec* entitySpec);
	void registerEntities(VirtualList positionedEntitiesToIgnore);
	void spawnEntity(PositionedEntity* positionedEntity, Container requester, EventListener callback);
	Entity addChildEntity(const PositionedEntity* const positionedEntity, bool permanent);
	Entity addChildEntityWithId(const PositionedEntity* const positionedEntity, bool permanent, int16 internalId);
	UIContainer getUIContainer();
	StageSpec* getStageSpec();
	ParticleRemover getParticleRemover();
	void showStreamingProfiling(int32 x, int32 y);
	bool unloadOutOfRangeEntities(int32 defer);
    bool loadInRangeEntities(int32 defer);
	Entity findChildByInternalId(int16 internalId);
	bool updateEntityFactory();
	EntityFactory getEntityFactory();
	VirtualList getSoundWrappers();
	bool streamAll();
	void streamAllOut();
	void forceNoPopIn(bool forceNoPopIn);

	virtual void load(VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition);
	virtual bool stream();
	override void update(uint32 elapsedTime);
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	override void synchronizeGraphics();
	override void suspend();
	override void resume();
	override void removeChild(Container child, bool deleteChild);
	override bool handlePropagatedMessage(int32 message);
}


#endif
