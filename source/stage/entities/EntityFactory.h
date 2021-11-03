/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>
#include <Container.h>
#include <Shape.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ENTITY_PENDING_PROCESSING		0x00
#define __LIST_EMPTY					0x01
#define __ENTITY_PROCESSED				0x02


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Entity;

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

	/// collision shapes
	ShapeSpec* shapeSpecs;

	/// pixelSize
	// if 0, width and height will be inferred from the first sprite's texture's pixelSize
	PixelSize pixelSize;

	/// object's in-game type
	uint8 inGameType;

	/// physical specification
	PhysicalSpecification* physicalSpecification;

} EntitySpec;

typedef const EntitySpec EntityROMSpec;


// an entity associated with a position
typedef struct PositionedEntity
{
	// pointer to the entity spec in ROM
	EntitySpec* entitySpec;

	// position in the screen coordinates
	ScreenPixelVector onScreenPosition;

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

/**
 * Positioned Entity Description
 *
 * @memberof EntityFactory
 */
typedef struct PositionedEntityDescription
{
	const PositionedEntity* positionedEntity;
	Container parent;
	Entity entity;
	EventListener callback;
	int16 spriteSpecIndex;
	int16 shapeSpecIndex;
	int16 transformedShapeSpecIndex;
	int16 internalId;
	bool transformed;
} PositionedEntityDescription;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class EntityFactory : Object
{
	// the EntityFactory entities to test for streaming
	VirtualList entitiesToInstantiate;
	// streaming's non yet transformed entities
	VirtualList entitiesToTransform;
	// streaming's non yet transformed entities
	VirtualList entitiesToMakeReady;
	// entities loaded
	VirtualList spawnedEntities;
	// index for method to execute
	int32 streamingPhase;

	/// @publicsection
	void constructor();
	uint32 prepareEntities();
	void prepareAllEntities();
	void spawnEntity(const PositionedEntity* positionedEntity, Container parent, EventListener callback, int16 internalId);
	uint32 hasEntitiesPending();
	uint32 instantiateEntities();
	uint32 transformEntities();
	uint32 makeReadyEntities();
	uint32 callLoadedEntities();
	void showStatus(int32 x, int32 y);
}


#endif
