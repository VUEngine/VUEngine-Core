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
	// Flag to signal if collisions are allowed
	bool collisionsEnabled;
	// Flag to signal if collisions are checked
	bool checkingCollisions;
	// Entity's internal id, set by the engine
	int16 internalId;
	// Used for collisions and streaming
	Size size;
	// Entity factory
	EntityFactory entityFactory;
	// components list
	VirtualList sprites;
	// Colliders for collision detection
	VirtualList colliders;
	// wireframes
	VirtualList wireframes;
	// Bahaviors list
	VirtualList behaviors;
	// Entity's spec
	EntitySpec* entitySpec;
	// Center displacement
	Vector3D* centerDisplacement;

	/// @publicsection
	static Entity instantiate(const EntitySpec* const entitySpec, int16 internalId, const char* const name, const PositionedEntity* const positionedEntity);
	static Entity loadEntity(const PositionedEntity* const positionedEntity, int16 internalId);
	static Entity loadEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId);
	static PixelRightBox getBoundingBoxFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition);
	static Vector3D* calculateGlobalPositionFromSpecByName(const struct PositionedEntity* childrenSpecs, Vector3D environmentPosition, const char* childName);

	void constructor(EntitySpec* entitySpec, int16 internalId, const char* const name);
	void addChildEntities(const PositionedEntity* childrenSpecs);
	void addChildEntitiesDeferred(const PositionedEntity* childrenSpecs);
	bool createSprites();
	bool createWireframes();
	bool createColliders();
	bool createBehaviors();
	Sprite addSprite(SpriteSpec* spriteSpec, SpriteManager spriteManager);
	void addSprites(SpriteSpec** spriteSpecs, bool destroyOldSprites);
	Wireframe addWireframe(WireframeSpec* wireframeSpec, WireframeManager wireframeManager);
	void addWireframes(WireframeSpec** wireframeSpecs, bool destroyOldWireframes);
	Collider addCollider(ColliderSpec* colliderSpec, CollisionManager collisionManager);
	void addColliders(ColliderSpec* colliderSpecs, bool destroyOldColliders);
	Behavior addBehavior(BehaviorSpec* behaviorSpec);
	void destroySprites();
	void calculateSize(bool force);
	Entity addChildEntity(const EntitySpec* entitySpec, int16 internalId, const char* name, const Vector3D* position, void* extraInfo);
	uint32 areAllChildrenInstantiated();
	uint32 areAllChildrenTransformed();
	uint32 areAllChildrenReady();
	Entity getChildById(int16 id);
	EntitySpec* getSpec();
	int32 getMapParallax();
	int16 getId();
	int16 getInternalId();
	VirtualList getSprites();
	VirtualList getWireframes();
	bool getBehaviors(ClassPointer classPointer, VirtualList behaviors);
	void setAnimation(void (*animation)());
	void checkCollisions(bool activate);
	void enableCollisions(bool value);
	void registerCollisions(bool value);
	bool doesAllowCollisions();
	bool hasColliders();
	void showColliders();
	void hideColliders();
	NormalizedDirection getNormalizedDirection();
	uint32 getCollidersLayers();
	void setCollidersLayers(uint32 layers);
	uint32 getCollidersLayersToIgnore();
	void setCollidersLayersToIgnore(uint32 layersToIgnore);
	bool isSpriteVisible(Sprite sprite, int32 pad);
	VirtualList getColliders();
	void updateSprites(uint32 updatePosition, uint32 updateScale, uint32 updateRotation, uint32 updateProjection);
	void setSpec(void* entitySpec);
	void setSize(Size size);
	bool isInCameraRange(int16 padding, bool recursive);

	virtual void setNormalizedDirection(NormalizedDirection normalizedDirection);
	virtual void setExtraInfo(void* extraInfo);
	virtual bool respawn();

	override void addChild(Container child);
	override void createComponents();
	override void initialTransform(const Transformation* environmentTransform);
	override void setTransparent(uint8 transparent);
	override bool handlePropagatedMessage(int32 message);
	override const Size* getSize();
	override fixed_t getWidth();
	override fixed_t getHeight();
	override fixed_t getDepth();
	override void suspend();
	override void resume();
	override bool isSubjectToGravity(Vector3D gravity);
	override void show();
	override void hide();
	override fixed_t getBounciness();
	override fixed_t getFrictionCoefficient();
	override uint32 getInGameType();
	override void destroyComponents();
}


#endif
