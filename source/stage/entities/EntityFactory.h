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

#include <ListenerObject.h>
#include <Behavior.h>
#include <Body.h>
#include <Entity.h>
#include <Sprite.h>
#include <Collider.h>
#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ENTITY_PENDING_PROCESSING		0x00
#define __LIST_EMPTY					0x01
#define __ENTITY_PROCESSED				0x02


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------


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
	int16 internalId;
	uint8 componentIndex;
	bool spritesCreated;
	bool wireframesCreated;
	bool collidersCreated;
	bool behaviorsCreated;
	bool transformed;
	bool graphicsSynchronized;

} PositionedEntityDescription;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VirtualList;

/// @ingroup stage-entities
class EntityFactory : ListenerObject
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
	uint32 addChildEntities();
	void showStatus(int32 x, int32 y);
}


#endif
