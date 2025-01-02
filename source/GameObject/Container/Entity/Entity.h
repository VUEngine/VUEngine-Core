/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <Container.h>
#include <Vector3D.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;
class EntityFactory;
class Telegram;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An Entity Spec
/// @memberof Entity
typedef struct EntitySpec
{
	/// Class allocator
	AllocatorPointer allocator;

	/// Component specs
	ComponentSpec** componentSpecs;

	/// Children specs
	struct PositionedEntity* childrenSpecs;

	/// Extra info
	void* extraInfo;

	// Size
	// If 0, it is computed from the visual components if any
	PixelSize pixelSize;

	// Entity's in-game type
	uint8 inGameType;

} EntitySpec;

/// An Entity spec that is stored in ROM
/// @memberof Entity
typedef const EntitySpec EntityROMSpec;

/// Struct that specifies how to create an spatially situated entity
/// @memberof Entity
typedef struct PositionedEntity
{
	// Pointer to the entity spec in ROM
	EntitySpec* entitySpec;

	// Position in the screen coordinates
	ScreenPixelVector onScreenPosition;

	// Rotation in screen coordinates
	ScreenPixelRotation onScreenRotation;

	// Scale in screen coordinates
	ScreenPixelScale onScreenScale;

	// Entity's id
	int16 id;

	// Name
	char* name;

	/// Children
	struct PositionedEntity* childrenSpecs;

	/// Extra info
	void* extraInfo;

	/// Force load even if out of the camera's frustum
	bool loadRegardlessOfPosition;

} PositionedEntity;

/// A PositionedEntity spec that is stored in ROM
/// @memberof Entity
typedef const PositionedEntity PositionedEntityROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Entity
///
/// Inherits from Container
///
/// Implements a container that can be added to stages
class Entity : Container
{
	/// @protectedsection

	/// Size of the entity in function of its components and its children's, grand children's,
	/// etc. components
	Size size;

	/// Factory to create this entity's children
	EntityFactory entityFactory;

	/// Pointer to the spec that defines how to initialize the entity
	EntitySpec* entitySpec;

	/// Diplacement between the entity's bounding box's center and its local position used to speed up the
	/// visibility check of the entity withing the camera's frustum
	Vector3D* centerDisplacement;

	/// @publicsection

	/// Create a new entity instance and configure it with the provided arguments.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the new entity
	/// and the spatial information about where and how to positione it
	/// @param internalId: ID to keep track internally of the new instance
	/// @return The new entity
	static Entity createEntity(const PositionedEntity* const positionedEntity, int16 internalId);

	/// Create a new entity instance and configure it over time with the provided arguments.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the new entity
	/// and the spatial information about where and how to positione it
	/// @param internalId: ID to keep track internally of the new instance
	/// @return The new, still not configured entity
	static Entity createEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId);

	/// Compute the spatially located bounding box of an entity created with the provided positioned entity
	/// struct.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the an entity
	/// @param environmentPosition: Vector used as the origin with respect to which computed the bounding
	/// box's position
	/// @return Spatially located bounding box of an entity that would be created with the provided
	/// positioned entity struct
	static RightBox getRightBoxFromSpec(
		const PositionedEntity* positionedEntity, const Vector3D* environmentPosition
	);

	/// Test if the provided right box lies inside the camera's frustum.
	/// @param vector3D: RightBox's translation vector
	/// @param rightBox: RightBox to test
	static inline bool isInsideFrustrum(Vector3D vector3D, RightBox rightBox);

	/// Class' constructor
	/// @param entitySpec: Specification that determines how to configure the entity
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(EntitySpec* entitySpec, int16 internalId, const char* const name);

	/// Add the components that must attach to this entity.
	/// Create the components that must attach to this container. 	
	/// @param componentSpecs: Specifications to be used to configure the new components
	override void createComponents(ComponentSpec** componentSpecs);

	/// Destroy the components that attach to this entity.
	override void destroyComponents();

	/// Configure the entity's size.
	override void calculateSize();

	/// Retrieve the object's radius.
	/// @return Radius
	override fixed_t getRadius();

	/// Check if the object is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the object; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();

	/// Prepare to suspend this instance's logic.
	override void suspend();

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Default command handler.
	/// @param command: Propagated command
	/// @param valud: A command related value
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override void handleCommand(int32 command, va_list args);

	/// Retrieve the entity's spec.
	/// @return Specification that determines how the entity was configured
	EntitySpec* getSpec();

	/// Retrieve the entity's entity factory
	/// @return Entity's entity facotyr
	EntityFactory getEntityFactory();

	/// Set the normalized direction towards where the entity faces.
	/// @param normalizedDirection: New facing direction with is components normalized
	void setNormalizedDirection(NormalizedDirection normalizedDirection);

	/// Retrieve the normalized direction towards where the entity faces.
	/// @return Entity's facing direction with is components normalized
	NormalizedDirection getNormalizedDirection();

	/// Spawn a new child and configure it with the provided positioned entity struct.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the new child
	Entity spawnChildEntity(const PositionedEntity* const positionedEntity);

	/// Spawn children and configure them with the provided entity specs.
	/// @param childrenSpecs: Array of entity specs to use to initialize the new children
	void addChildEntities(const PositionedEntity* childrenSpecs);

	/// Spawn children and configure them over time with the provided entity specs.
	/// @param childrenSpecs: Array of entity specs to use to initialize the new children
	void addChildEntitiesDeferred(const PositionedEntity* childrenSpecs);

	/// Retrieve the entity's width.
	/// @return Entity's width
	fixed_t getWidth();

	/// Retrieve the entity's height.
	/// @return Entity's height
	fixed_t getHeight();

	/// Retrieve the entity's depth.
	/// @return Entity's depth
	fixed_t getDepth();

	/// Check if the entity is withing the camera's frustum.
	/// @param padding: Padding to be added to camera's frustum
	/// @param recursive: If true, the check is performed on the children, grand children, etc.
	/// @return True if the entity is within the camera's frustum
	bool isInCameraRange(int16 padding, bool recursive);

	/// Set the entity's spec.
	/// @param entitySpec: Specification that determines how to configure the entity
	virtual void setSpec(void* entitySpec);

	/// Set any extra info provided by the PositionedEntity struct used to instantiate this entity.
	/// @param extraInfo: Pointer to the extra information that the entity might need
	virtual void setExtraInfo(void* extraInfo);

	/// Check if the entity must be streamed in after being streamed out or destroyed.
	/// @return True if the streaming must spawn this entity back when deleted
	virtual bool alwaysStreamIn();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline bool
Entity::isInsideFrustrum(Vector3D vector3D, RightBox rightBox)
{
	extern const CameraFrustum* _cameraFrustum;
	vector3D = Vector3D::rotate(Vector3D::getRelativeToCamera(vector3D), *_cameraInvertedRotation);

	if(vector3D.x + rightBox.x0 > __PIXELS_TO_METERS(_cameraFrustum->x1) ||
	   vector3D.x + rightBox.x1 < __PIXELS_TO_METERS(_cameraFrustum->x0))
	{
		return false;
	}

	// check y visibility
	if(vector3D.y + rightBox.y0 > __PIXELS_TO_METERS(_cameraFrustum->y1) ||
	   vector3D.y + rightBox.y1 < __PIXELS_TO_METERS(_cameraFrustum->y0))
	{
		return false;
	}

	// check z visibility
	if(vector3D.z + rightBox.z0 > __PIXELS_TO_METERS(_cameraFrustum->z1) ||
	   vector3D.z + rightBox.z1 < __PIXELS_TO_METERS(_cameraFrustum->z0))
	{
		return false;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


#endif
