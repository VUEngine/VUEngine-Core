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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Behavior.h>
#include <Body.h>
#include <Camera.h>
#include <ColliderManager.h>
#include <Container.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Vector3D.h>
#include <Wireframe.h>
#include <WireframeManager.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Behavior;
class Entity;
class EntityFactory;
class Telegram;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// An Entity Spec
/// @memberof Entity
typedef struct EntitySpec
{
	/// Class allocator
	AllocatorPointer allocator;

	/// Component specs
	ComponentSpec** componentSpecs;

	/// Children
	struct PositionedEntity* childrenSpecs;

	/// Extra info
	void* extraInfo;

	/// If 0, width and height will be inferred from the first sprite's texture's pixelSize
	PixelSize pixelSize;

	/// Entity's in-game type
	uint8 inGameType;

} EntitySpec;

/// An Entity spec that is stored in ROM
/// @memberof Entity
typedef const EntitySpec EntityROMSpec;


/// Struct that specifies how to create an spatially situated entity
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


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Entity
///
/// Inherits from Container
///
/// Implements a container that can have sprites, wireframes and colliders attached to it.
class Entity : Container
{
	/// @protectedsection

	/// Signals if collisions against this entity's colliders are allowed
	bool collisionsEnabled;

	/// Signals if collisions are against other entity's colliders are allowed
	bool checkingCollisions;
	
	/// Entity's internal id, set by the engine
	int16 internalId;
	
	/// Size of the entity in function of its components and its children's, grand children's,
	/// etc. components
	Size size;

	/// Factory to create this entity's children
	EntityFactory entityFactory;

	/// Pointer to the spec that defines how to initialize the entity 
	EntitySpec* entitySpec;

	/// Diplacement between the entity's bounding box's center and its local position
	/// used to speed up the visibility check of the entity withing the camera's frustum
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

	/// Compute the spatially located bounding box of an entity created with the provided positioned entity struct.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the an entity
	/// @param environmentPosition: Vector used as the origin with respect to which computed the bounding box's position
	/// @return Spatially located bounding box of an entity that would be created with the provided positioned entity struct
	static RightBox getRightBoxFromSpec(const PositionedEntity* positionedEntity, const Vector3D* environmentPosition);

	/// Test if the provided right box lies inside the camera's frustum.
	/// @param vector3D: RightBox's translation vector
	/// @param rightBox: RightBox to test
	static inline bool isInsideFrustrum(Vector3D vector3D, RightBox rightBox);

	/// Class' constructor
	/// @param entitySpec: Specification that determines how to configure the entity
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(EntitySpec* entitySpec, int16 internalId, const char* const name);

	/// Retrieve the object's radius.
	/// @return Radius
	override fixed_t getRadius();

	/// Retrieve the object's bounciness factor.
	/// @return Object's bounciness factor
	override fixed_t getBounciness();

	/// Retrieve the object's friction coefficient.
	/// @return Object's friction coefficient
	override fixed_t getFrictionCoefficient();

	/// Check if the object is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the object; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Retrieve the enum that determines the type of game object.
	/// @return The enum that determines the type of game object
	override uint32 getInGameType();

	/// Add the components that must attach to this entity. 
	override void createComponents();

	/// Destroy the components that attach to this entity. 	
	override void destroyComponents();

	/// Make this instance visible.
	override void show();

	/// Make this instance invisible.
	override void hide();

	/// Prepare to suspend this instance's logic.
	override void suspend();

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Set this instance's transparency effects.
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	override void setTransparency(uint8 transparency);

	/// Retrieve the entity's spec.
	/// @return Specification that determines how the entity was configured
	EntitySpec* getSpec();

	/// Retrieve the entity's internal id used by the engine to keep track of it.
	/// @return Entity's internal id
	int16 getInternalId();

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
	
	/// Retrieve a child of this entity whose internal ID equals the provided one.
	/// @param id: Internal ID to look for
	/// @return Child entity whose ID matches the provided one
	Entity getChildById(int16 id);

	/// Attach a new component to the entity and configure it with the provided spec.
	/// @param componentSpec: Specification to be used to configure the new component
	/// @return Added component
	Component addComponent(ComponentSpec* componentSpec);

	/// Remove a component from the entity.
	/// @param component: Component to remove
	void removeComponent(Component component);

	/// Attach a new components to the entity and configure it with the provided specs.
	/// @param componentSpecs: Specifications to be used to configure the new components
	void addComponents(ComponentSpec** componentSpecs);

	/// Remove the components of a give type from the entity.
	/// @param componentType: Type of components to remove
	void removeComponents(uint32 componentType);

	/// Retrieve a list with the components of the provided type.
	/// @param componentType: Type of components to add
	/// @return Linked list of components of the type provided
	VirtualList getComponents(uint32 componentType);

	/// Retrieve the linked list of components that are instances of the provided class.
	/// @param classPointer: Pointer to the class to use as search criteria. Usage: typeofclass(ClassName)
	/// @param components: Linked list to be filled with the behaviors that meed the search criteria 
	/// (it is externally allocated and must be externally deleted)
	/// @param componentType: Type of components to retrieve
	/// @return True if one or more behaviors met the search criteria; false otherwise
	bool getComponentsOfClass(ClassPointer classPointer, VirtualList components, uint32 componentType);
	
	/// Enable collision detection on the entity's colliders.
	void enableCollisions();

	/// Disable collision detection on the entity's colliders.
	void disableCollisions();

	/// Enable or disable collision detection against other entities' colliders.
	/// @param activate: If true, this entity's colliders check collision against other entities'
	void checkCollisions(bool activate);

	/// Enable or disable the register of detected collisions.
	/// @param activate: If false, this entity's colliders won't keep track of collisions, hence they
	/// won't notify of it of persisting (::collisionPersists) collisions or when end (::collisionEnds)
	void registerCollisions(bool activate);

	/// Set the layers in which this entity's colliders must live.
	/// @param layers: Flags that determine the layers for the entity's colliders
	void setCollidersLayers(uint32 layers);

	/// Retrieve the layers in which this entity's colliders live.
	/// @return Flags that determine the layers where the entity's colliders live
	uint32 getCollidersLayers();

	/// Set the layers that the entity's colliders must ignore when detecting collision.
	/// @param layersToIgnore: Flags that determine the layers with colliders to ignore when detecting collisions
	void setCollidersLayersToIgnore(uint32 layersToIgnore);

	/// Retrieve the layers that the entity's colliders ignore when detecting collision.
	/// @return The layers that the entity's colliders ignore when detecting collision
	uint32 getCollidersLayersToIgnore();

	/// Check if the entity has attached colliders.
	/// @return True if the entity hast at least on collider arrached; false otherwise
	bool hasColliders();

	/// Make the entity's colliders visible.
	void showColliders();

	/// Make the entity's colliders invisible.
	void hideColliders();
	
	/// Configure the entity's size.
	void calculateSize();

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

//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static inline bool Entity::isInsideFrustrum(Vector3D vector3D, RightBox rightBox)
{
	extern const CameraFrustum* _cameraFrustum;
	vector3D = Vector3D::rotate(Vector3D::getRelativeToCamera(vector3D), *_cameraInvertedRotation);
	
	if(vector3D.x + rightBox.x0 > __PIXELS_TO_METERS(_cameraFrustum->x1) || vector3D.x + rightBox.x1 < __PIXELS_TO_METERS(_cameraFrustum->x0))
	{
		return false;
	}

	// check y visibility
	if(vector3D.y + rightBox.y0 > __PIXELS_TO_METERS(_cameraFrustum->y1) || vector3D.y + rightBox.y1 < __PIXELS_TO_METERS(_cameraFrustum->y0))
	{
		return false;
	}

	// check z visibility
	if(vector3D.z + rightBox.z0 > __PIXELS_TO_METERS(_cameraFrustum->z1) || vector3D.z + rightBox.z1 < __PIXELS_TO_METERS(_cameraFrustum->z0))
	{
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------


#endif
