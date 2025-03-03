/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STAGE_H_
#define STAGE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Container.h>
#include <CharSet.h>
#include <Actor.h>
#include <ActorFactory.h>
#include <Printer.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <VIPManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;
class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An Stage Spec
/// @memberof Stage
typedef struct StageSpec
{
	AllocatorPointer allocator;

	/// Timer config
	struct Timer
	{
		/// Timer's resolution (__TIMER_100US or __TIMER_20US)
		uint16 resolution;

		/// Target elapsed time between timer interrupts
		uint16 targetTimePerInterrupt;

		/// Timer interrupt's target time units
		uint16 targetTimePerInterrupttUnits;

	} timer;

	/// Sound config
	struct SoundConfig
	{
		/// Target refresh rate for PCM playback
		uint16 pcmTargetPlaybackRefreshRate;

	} sound;

	/// General stage's attributes
	struct Level
	{
		/// Stage's size in pixels
		PixelSize pixelSize;

		/// Camera's initial position inside the stage
		PixelVector cameraInitialPosition;

		/// camera's frustum
		CameraFrustum cameraFrustum;

	} level;

	/// Streaming
	struct Streaming
	{
		/// Padding to be added to camera's frustum when checking if a actor spec
		/// describes an actor that is within the camera's range
		uint16 loadPadding;

		/// Padding to be added to camera's frustum when checking if a actor is 
		/// out of the camera's range
		uint16 unloadPadding;

		/// Amount of actor descriptions to check for streaming in entitis
		uint16 streamingAmplitude;
		
		/// If true, actor instantiation is done over time
		bool deferred;

	} streaming;

	/// Rendering
	struct Rendering
	{
		/// Maximum number of texture's rows to write each time the texture writing is active
		int32 texturesMaximumRowsToWrite;

		/// Maximum number of rows to compute on each call to the affine functions
		int32 maximumAffineRowsToComputePerCall;

		/// Color configuration
		ColorConfig colorConfig;

		/// Palettes' configuration
		PaletteConfig paletteConfig;

		/// Number of BGMAP segments reserved for the param tables
		int32 paramTableSegments;

		/// Object segments' sizes (__spt0 to __spt3)
		int16 objectSpritesContainersSize[__TOTAL_OBJECT_SEGMENTS];

		/// Object segments' z coordinates (__spt0 to __spt3)
		int16 objectSpritesContainersZPosition[__TOTAL_OBJECT_SEGMENTS];

		/// Struct defining the optical settings for the stage
		PixelOptical pixelOptical;

	} rendering;

	/// Physics
	struct Physics
	{
		/// Physical world's gravity
		Vector3D gravity;

		/// Physical world's friction coefficient
		fixed_t frictionCoefficient;

	} physics;

	/// Assets
	struct Assets
	{
		/// Fonts to preload
		FontSpec** fontSpecs;

		// CharSets to preload
		CharSetSpec** charSetSpecs;

		// Textures to preload
		TextureSpec** textureSpecs;

		/// Sounds to load
		SoundSpec** sounds;

	} assets;

	/// Actors
	struct Actors
	{
		/// UI configuration
		struct 
		{
			// UI's children actors
			PositionedActor* childrenSpecs;

			/// UI's class
			AllocatorPointer allocator;

		} UI;

		// Stage's children actors
		PositionedActor* children;

	} actors;

	/// Post processing effects
	PostProcessingEffect* postProcessingEffects;

} StageSpec;

/// A Stage spec that is stored in ROM
/// @memberof Stage
typedef const StageSpec StageROMSpec;

/// A struct that holds precomputed information about the configuration of
/// then entites that will populate the stage
/// @memberof Stage
typedef struct StageActorDescription
{
	/// Bounding box of the actor to 
	RightBox rightBox;
	
	/// Struct that defines which actor spec to use to configure the new actor
	PositionedActor* positionedActor;

	/// Pointer to the extra information that the actor might need
	void* extraInfo;

	/// ID to keep track internally of the actor
	int16 internalId;

	/// If false, the bounding box's volume is zero
	bool validRightBox;

} StageActorDescription;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Stage
///
/// Inherits from Container
///
/// Implements a container that represents a game level.
class Stage : Container
{
	/// @protectedsection

	/// Pointer to the spec that defines how to initialize the stage 
	StageSpec* stageSpec;

	/// Factory to create this actor's children
	ActorFactory actorFactory;

	/// List of structs that holds precomputed information about the configuration of
	/// then entites that will populate the stage
	VirtualList stageActorDescriptions;

	/// Pivot node for streaming
	VirtualNode streamingHeadNode;

	// List of sounds loaded from the stage spec
	VirtualList sounds;

	/// List of listeners for actor loading
	VirtualList actorLoadingListeners;

	/// Index for streaming method to execute in the current game cycle
	int32 streamingPhase;

	/// Amount of actor descriptions to check for streaming in entitis
	uint16 streamingAmplitude;

	/// Must keep track of the camera's focus actor in order to restore
	/// it when resuming the stage's owner state
	Actor focusActor;

	/// Next ID to use for new actors
	int16 nextActorId;

	/// Flag to determine the direction of the stream in
	bool reverseStreaming;

	/// Cache of the camera's transformation for resuming the game
	Transformation cameraTransformation;

	/// @publicsection

	/// Class' constructor
	/// @param stageSpec: Specification that determines how to configure the stage
	void constructor(StageSpec* stageSpec);
	
	/// Prepare to suspend this instance's logic.
	override void suspend();

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Retrieve the stage's spec.
	/// @return Specification that determines how the stage was configured
	const StageSpec* getSpec();

	/// Configure the timer.
	void configureTimer();

	/// Configure the color palettes.
	void configurePalettes();

	/// Retrieve the palette configuration for the stage.
	/// @return Palette configuration struct
	PaletteConfig getPaletteConfig();
	
	/// Register the stage's spec actors in the streaming list
	void registerActors(VirtualList positionedActorsToIgnore);

	/// Register an event listener for the event when a new actor is instantiated.
	/// @param listener: Object that will be notified of event
	void addActorLoadingListener(ListenerObject listener);

	/// Spawn a new child and configure it with the provided positioned actor struct.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the new child
	/// @param permanent: If true, the actor is not subject to the streaming
	Actor spawnChildActor(const PositionedActor* const positionedActor, bool permanent);

	/// Destroy a stage's child.
	/// @param child: Actor to destroy
	void destroyChildActor(Actor child);

	/// Stream in or/and out all pending actors.
	void streamAll();

	/// Retrieve the sounds that are playing in the stage.
	/// @return List of playing sounds
	VirtualList getSounds();
	
	/// Fade in or out the registered sounds
	/// @param playbackType: Specifies how the playback should start
	void fadeSounds(uint32 playbackType);

	/// Print the stage's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// Stream in or out actors within or outside the camera's range.
	virtual bool stream();

	/// Configure the stage with the actors defined in its spec.
	/// @param positionedActorsToIgnore: List of positioned actor structs to register for streaming
	virtual void configure(VirtualList positionedActorsToIgnore);

	/// @privatesection

	/// These are not meant to be called externally. They are declared here
	/// because of the preprocessor's limitations for forward declarations
	/// in source files. Don't call these.
	bool unloadOutOfRangeActors(int32 defer);
	bool loadInRangeActors(int32 defer);
}

#endif
