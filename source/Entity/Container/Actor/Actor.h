/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ACTOR_H_
#define ACTOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <Container.h>
#include <Vector3D.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Actor;
class ActorFactory;
class Telegram;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An Actor Spec
/// @memberof Actor
typedef struct ActorSpec
{
	/// Class allocator
	AllocatorPointer allocator;

	/// Component specs
	ComponentSpec** componentSpecs;

	/// Children specs
	struct PositionedActor* childrenSpecs;

	/// Extra info
	void* extraInfo;

	// Size
	// If 0, it is computed from the visual components if any
	PixelSize pixelSize;

	// Actor's in-game type
	uint8 inGameType;

	/// Animation to play automatically
	char* initialAnimation;

} ActorSpec;

/// An Actor spec that is stored in ROM
/// @memberof Actor
typedef const ActorSpec ActorROMSpec;

/// Struct that specifies how to create an spatially situated actor
/// @memberof Actor
typedef struct PositionedActor
{
	// Pointer to the actor spec in ROM
	const ActorSpec* actorSpec;

	// Position in the screen coordinates
	ScreenPixelVector onScreenPosition;

	// Rotation in screen coordinates
	ScreenPixelRotation onScreenRotation;

	// Scale in screen coordinates
	ScreenPixelScale onScreenScale;

	// Actor's id
	int16 id;

	// Name
	char* name;

	/// Children
	struct PositionedActor* childrenSpecs;

	/// Extra info
	void* extraInfo;

	/// Force load even if out of the camera's frustum
	bool loadRegardlessOfPosition;

} PositionedActor;

/// A PositionedActor spec that is stored in ROM
/// @memberof Actor
typedef const PositionedActor PositionedActorROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Actor
///
/// Inherits from Container
///
/// Implements a container that can be added to stages
class Actor : Container
{
	/// @protectedsection

	/// Size of the actor in function of its components and its children's, grand children's,
	/// etc. components
	Size size;

	/// Factory to create this actor's children
	ActorFactory actorFactory;

	/// Pointer to the spec that defines how to initialize the actor
	const ActorSpec* actorSpec;

	/// Diplacement between the actor's bounding box's center and its local position used to speed up the
	/// visibility check of the actor withing the camera's frustum
	Vector3D* centerDisplacement;

	/// Name of the currently playing animation. This is here to save on memory on Entities that
	/// don't need to keep track of the playing animation for resuming
	const char* playingAnimationName;

	/// @publicsection

	/// Create a new actor instance and configure it with the provided arguments.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the new actor
	/// and the spatial information about where and how to positione it
	/// @param internalId: ID to keep track internally of the new instance
	/// @return The new actor
	static Actor createActor(const PositionedActor* const positionedActor, int16 internalId);

	/// Create a new actor instance and configure it over time with the provided arguments.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the new actor
	/// and the spatial information about where and how to positione it
	/// @param internalId: ID to keep track internally of the new instance
	/// @return The new, still not configured actor
	static Actor createActorDeferred(const PositionedActor* const positionedActor, int16 internalId);

	/// Compute the spatially located bounding box of an actor created with the provided positioned actor
	/// struct.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the an actor
	/// @param environmentPosition: Vector used as the origin with respect to which computed the bounding
	/// box's position
	/// @return Spatially located bounding box of an actor that would be created with the provided
	/// positioned actor struct
	static RightBox getRightBoxFromSpec
	(
		const PositionedActor* positionedActor, const Vector3D* environmentPosition
	);

	/// Test if the provided right box lies inside the camera's frustum.
	/// @param vector3D: RightBox's translation vector
	/// @param rightBox: RightBox to test
	static inline bool isInsideFrustrum(Vector3D vector3D, RightBox rightBox);

	/// Class' constructor
	/// @param actorSpec: Specification that determines how to configure the actor
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name);

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Add the components that must attach to this actor.
	/// Create the components that must attach to this container. 	
	/// @param componentSpecs: Specifications to be used to configure the new components
	override void createComponents(ComponentSpec** componentSpecs);

	/// Destroy the components that attach to this actor.
	override void destroyComponents();

	/// Configure the actor's size.
	override void calculateSize();

	/// Retrieve the object's radius.
	/// @return Radius
	override fixed_t getRadius();

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();

	/// Make the animated actor ready to start operating once it has been completely intialized.
	/// @param recursive: If true, the ready call is propagated to its children, grand children, etc.
	override void ready(bool recursive);

	/// Prepare to suspend this instance's logic.
	override void suspend();

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Default command handler.
	/// @param command: Propagated command
	/// @param valud: A command related value
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override void handleCommand(int32 command, va_list args);

	/// Default string handler for propagateString
	/// @param string: Propagated string
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override bool handlePropagatedString(const char* string);

	/// Retrieve the actor's spec.
	/// @return Specification that determines how the actor was configured
	const ActorSpec* getSpec();

	/// Retrieve the actor's actor factory
	/// @return Actor's actor facotyr
	ActorFactory getActorFactory();

	/// Spawn a new child and configure it with the provided positioned actor struct.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the new child
	Actor spawnChildActor(const PositionedActor* const positionedActor);

	/// Spawn children and configure them with the provided actor specs.
	/// @param childrenSpecs: Array of actor specs to use to initialize the new children
	void addChildActors(const PositionedActor* childrenSpecs);

	/// Spawn children and configure them over time with the provided actor specs.
	/// @param childrenSpecs: Array of actor specs to use to initialize the new children
	void addChildActorsDeferred(const PositionedActor* childrenSpecs);

	/// Retrieve the actor's width.
	/// @return Actor's width
	fixed_t getWidth();

	/// Retrieve the actor's height.
	/// @return Actor's height
	fixed_t getHeight();

	/// Retrieve the actor's depth.
	/// @return Actor's depth
	fixed_t getDepth();

	/// Check if the actor is withing the camera's frustum.
	/// @param padding: Padding to be added to camera's frustum
	/// @param recursive: If true, the check is performed on the children, grand children, etc.
	/// @return True if the actor is within the camera's frustum
	bool isInCameraRange(int16 padding, bool recursive);

	/// Play the animation with the provided name.
	/// @param animationName: Name of the animation to play
	void playAnimation(const char* animationName);

	/// Pause or unpause the currently playing animation if any.
	/// @param pause: Flag that signals if the animation must be paused or unpaused
	void pauseAnimation(bool pause);

	/// Stop any playing animation if any.
	void stopAnimation();

	/// Check if an animation is playing.
	/// @return True if an animation is playing; false otherwise
	bool isPlaying();

	/// Check if the animation whose name is provided is playing.
	/// @param animationName: Name of the animation to check
	/// @return True if an animation is playing; false otherwise
	bool isPlayingAnimation(char* animationName);

	/// Retrieve the animation function's name currently playing if any
	/// @return Animation function's name currently playing if any
	const char* getPlayingAnimationName();

	/// Skip the currently playing animation to the next frame.
	void nextFrame();

	/// Rewind the currently playing animation to the previous frame.
	void previousFrame();

	/// Skip the currently playing animation to the provided frame.
	/// @param frame: The frame of the playing animation to skip to
	/// @return True if the actual frame was changed; false otherwise
	void setActualFrame(int16 frame);

	/// Retrieve the actual frame of the playing animation if any.
	/// @return Actual frame of the playing animation if any
	int16 getActualFrame();

	/// Retrieve the number of frames in the currently playing animation if any.
	/// @return The numer of frames if an animation is playing; o otherwise
	int32 getNumberOfFrames();

	/// Set the actor's spec.
	/// @param actorSpec: Specification that determines how to configure the actor
	virtual void setSpec(void* actorSpec);

	/// Set any extra info provided by the PositionedActor struct used to instantiate this actor.
	/// @param extraInfo: Pointer to the extra information that the actor might need
	virtual void setExtraInfo(void* extraInfo);

	/// Check if the actor must be streamed in after being streamed out or destroyed.
	/// @return True if the streaming must spawn this actor back when deleted
	virtual bool alwaysStreamIn();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline bool
Actor::isInsideFrustrum(Vector3D vector3D, RightBox rightBox)
{
	extern const CameraFrustum* _cameraFrustum;

	vector3D = Vector3D::rotate(Vector3D::getRelativeToCamera(vector3D), *_cameraInvertedRotation);

#ifndef __LEGACY_COORDINATE_PROJECTION
	vector3D = 
		Vector3D::sum
		(
			vector3D, 
			(Vector3D)
			{
				__PIXELS_TO_METERS(_cameraFrustum->x1 - _cameraFrustum->x0) >> 1,
				__PIXELS_TO_METERS(_cameraFrustum->y1 - _cameraFrustum->y0) >> 1,
				__PIXELS_TO_METERS(_cameraFrustum->z1 - _cameraFrustum->z0) >> 1,
			}
		);
#endif

	if
	(
		vector3D.x + rightBox.x0 > __PIXELS_TO_METERS(_cameraFrustum->x1) 
		||
		vector3D.x + rightBox.x1 < __PIXELS_TO_METERS(_cameraFrustum->x0)
	)
	{
		return false;
	}

	// Check y visibility
	if
	(
		vector3D.y + rightBox.y0 > __PIXELS_TO_METERS(_cameraFrustum->y1) 
		||
		vector3D.y + rightBox.y1 < __PIXELS_TO_METERS(_cameraFrustum->y0)
	)
	{
		return false;
	}

	// Check z visibility
	if
	(
		vector3D.z + rightBox.z0 > __PIXELS_TO_METERS(_cameraFrustum->z1)
		||
		vector3D.z + rightBox.z1 < __PIXELS_TO_METERS(_cameraFrustum->z0))
	{
		return false;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
