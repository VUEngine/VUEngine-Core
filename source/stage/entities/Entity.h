/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ENTITY_H_
#define ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Behavior.h>
#include <Body.h>
#include <CollisionManager.h>
#include <Container.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Wireframe.h>
#include <WireframeManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Behavior;
class Entity;
class EntityFactory;
class Telegram;


// defines an entity in ROM memory
typedef struct EntitySpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// children
	struct PositionedEntity* childrenSpecs;

	/// behaviors
	BehaviorSpec** behaviorSpecs;

	/// extra info
	void* extraInfo;

	/// sprites
	SpriteSpec** spriteSpecs;

	/// use z displacement in projection
	bool useZDisplacementInProjection;

	/// wireframees
	WireframeSpec** wireframeSpecs;

	/// collision colliders
	ColliderSpec* colliderSpecs;

	/// pixelSize
	// if 0, width and height will be inferred from the first sprite's texture's pixelSize
	PixelSize pixelSize;

	/// object's in-game type
	uint8 inGameType;

	/// physical specification
	PhysicalProperties* physicalProperties;

} EntitySpec;

typedef const EntitySpec EntityROMSpec;


// an entity associated with a position
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

	// entity's id
	int16 id;

	// name
	char* name;

	// the children
	struct PositionedEntity* childrenSpecs;

	// extra info
	void* extraInfo;

	// force load
	bool loadRegardlessOfPosition;

} PositionedEntity;

typedef const PositionedEntity PositionedEntityROMSpec;

/// @ingroup stage-entities
class Entity : Container
{
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

	/// Linked list of attached sprites
	VirtualList sprites;

	/// Linked list of attached wireframes
	VirtualList wireframes;

	/// Linked list of attached colliders
	VirtualList colliders;

	/// Linked list of attached behaviors
	VirtualList behaviors;

	/// Pointer to the spec that defines how to initialize the entity 
	EntitySpec* entitySpec;

	/// Diplacement between the entity's bounding box's center and its local position
	/// used to speed up the visibility check of the entity withing the camera's frustum
	Vector3D* centerDisplacement;

	/// @publicsection
	static Entity instantiate(const EntitySpec* const entitySpec, int16 internalId, const char* const name, const PositionedEntity* const positionedEntity);
	static Entity loadEntity(const PositionedEntity* const positionedEntity, int16 internalId);
	static Entity loadEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId);
	static PixelRightBox getBoundingBoxFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition);

	void constructor(EntitySpec* entitySpec, int16 internalId, const char* const name);

	EntitySpec* getSpec();

	int16 getInternalId();

	EntityFactory getEntityFactory();

	void setNormalizedDirection(NormalizedDirection normalizedDirection);
	NormalizedDirection getNormalizedDirection();

	Entity spawnChildEntity(const PositionedEntity* const positionedEntity);
	void addChildEntities(const PositionedEntity* childrenSpecs);
	void addChildEntitiesDeferred(const PositionedEntity* childrenSpecs);
	
	Entity getChildById(int16 id);

	Behavior addBehavior(BehaviorSpec* behaviorSpec);
	bool getBehaviors(ClassPointer classPointer, VirtualList behaviors);

	Sprite addSprite(SpriteSpec* spriteSpec, SpriteManager spriteManager);
	void removeSprite(Sprite sprite);
	void addSprites(SpriteSpec** spriteSpecs, bool destroyOldSprites);
	VirtualList getSprites();
	void removeSprites();
	
	Wireframe addWireframe(WireframeSpec* wireframeSpec, WireframeManager wireframeManager);
	void addWireframes(WireframeSpec** wireframeSpecs, bool destroyOldWireframes);
	void removeWireframe(Wireframe wireframe);
	VirtualList getWireframes();
	void removeWireframes();
	
	Collider addCollider(ColliderSpec* colliderSpec, CollisionManager collisionManager);
	void addColliders(ColliderSpec* colliderSpecs, bool destroyOldColliders);
	void removeCollider(Collider collider);
	VirtualList getColliders();
	void removeColliders();
	
	void enableCollisions();
	void disableCollisions();
	void checkCollisions(bool activate);
	void registerCollisions(bool value);

	void setCollidersLayers(uint32 layers);
	uint32 getCollidersLayers();

	void setCollidersLayersToIgnore(uint32 layersToIgnore);
	uint32 getCollidersLayersToIgnore();

	bool hasColliders();
	void showColliders();
	void hideColliders();
	

	void setSize(Size size);
	void calculateSize(bool force);

	fixed_t getWidth();
	fixed_t getHeight();
	fixed_t getDepth();

	bool isInCameraRange(int16 padding, bool recursive);

	/// Set the particle systems's spec.
	/// @param particleSystemSpec: Specification that determines how to configure the particle system
	virtual void setSpec(void* entitySpec);

	/// Set any extra info provided by the PositionedEntity struct used to instantiate this entity.
	/// @param extraInfo: Pointer to the extra information that the entity might need
	virtual void setExtraInfo(void* extraInfo);

	/// Check if the entity must be streamed in after being streamed out or destroyed.
	/// @return True if the streaming must spawn this entity back when deleted
	virtual bool alwaysStreamIn();

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
	override void addComponents();

	/// Remove the components that attach to this entity. 	
	override void removeComponents();

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

	/// Default interger message handler for propagateMessage
	/// @param message: Propagated integer message
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override bool handlePropagatedMessage(int32 message);
}


#endif
