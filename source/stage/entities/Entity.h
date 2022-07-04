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

#include <Container.h>
#include <EntityFactory.h>
#include <BgmapSprite.h>
#include <ObjectSprite.h>
#include <Telegram.h>
#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class Entity : Container
{
	// Used for collisions and streaming
	Size size;
	// Entity's internal id, set by the engine
	int16 internalId;
	// Entity factory
	EntityFactory entityFactory;
	// sprites list
	VirtualList sprites;
	// Shapes for collision detection
	VirtualList shapes;
	// wireframes
	VirtualList wireframes;
	// Entity's spec
	EntitySpec* entitySpec;
	// Center displacement
	Vector3D* centerDisplacement;
	// Flag to prevent transforming the shapes during the transformation phase
	bool transformShapes;
	// Flag to signal if collisions are allowed
	bool allowCollisions;

	/// @publicsection
	static Entity instantiate(const EntitySpec* const entitySpec, int16 internalId, const char* const name, const PositionedEntity* const positionedEntity);
	static Entity loadEntity(const PositionedEntity* const positionedEntity, int16 internalId);
	static Entity loadEntityDeferred(const PositionedEntity* const positionedEntity, int16 internalId);
	static PixelRightBox getTotalSizeFromSpec(const PositionedEntity* positionedEntity, const PixelVector* environmentPosition);
	static Vector3D* calculateGlobalPositionFromSpecByName(const struct PositionedEntity* childrenSpecs, Vector3D environmentPosition, const char* childName);

	void constructor(EntitySpec* entitySpec, int16 internalId, const char* const name);
	void streamOut();
	void addChildEntities(const PositionedEntity* childrenSpecs);
	void addChildEntitiesDeferred(const PositionedEntity* childrenSpecs);
	Entity addChildEntity(const EntitySpec* entitySpec, int16 internalId, const char* name, const Vector3D* position, void* extraInfo);
	bool addSpriteFromSpecAtIndex(int32 spriteSpecIndex);
	bool addShapeFromSpecAtIndex(int32 shapeSpecIndex);
	bool transformShapeAtSpecIndex(int32 shapeSpecIndex);
	void addSprites(SpriteSpec** spritesSpecs);
	uint32 areAllChildrenInstantiated();
	uint32 areAllChildrenTransformed();
	uint32 areAllChildrenReady();
	Entity getChildById(int16 id);
	EntitySpec* getEntitySpec();
	int32 getMapParallax();
	int16 getId();
	int16 getInternalId();
	VirtualList getSprites();
	VirtualList getWireframes();
	void transformShapes();
	void setAnimation(void (*animation)());
	void activeCollisionChecks(bool activate);
	void allowCollisions(bool value);
	void registerCollisions(bool value);
	bool doesAllowCollisions();
	bool hasShapes();
	void showShapes();
	void hideShapes();
	Direction getDirection();
	uint32 getShapesLayers();
	void setShapesLayers(uint32 layers);
	uint32 getShapesLayersToIgnore();
	void setShapesLayersToIgnore(uint32 layersToIgnore);
	void setTransparent(uint8 transparent);
	bool isSpriteVisible(Sprite sprite, int32 pad);
	void setupShapes();
	bool isVisible(int32 pad, bool recursive);
	VirtualList getShapes();
	void updateSprites(uint32 updatePosition, uint32 updateScale, uint32 updateRotation, uint32 updateProjection);
	bool updateSpritePosition();
	bool updateSpriteRotation();
	bool updateSpriteScale();
	void releaseSprites();
	void setSpec(void* entitySpec);
	virtual void setDirection(Direction direction);
	virtual void setExtraInfo(void* extraInfo);
	virtual bool respawn();
	virtual uint16 getAxisForShapeSyncWithDirection();
	override void iAmDeletingMyself();
	override void initialTransform(const Transformation* environmentTransform, uint32 recursive);
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	override void setLocalPosition(const Vector3D* position);
	override void setLocalRotation(const Rotation* rotation);
	override void synchronizeGraphics();
	override bool handleMessage(Telegram telegram);
	override const Rotation* getRotation();
	override const Scale* getScale();
	override fixed_t getWidth();
	override fixed_t getHeight();
	override fixed_t getDepth();
	override void suspend();
	override void resume();
	override bool isSubjectToGravity(Acceleration gravity);
	override void show();
	override void hide();
	override fixed_t getBounciness();
	override fixed_t getFrictionCoefficient();
	override uint32 getInGameType();
}


#endif
