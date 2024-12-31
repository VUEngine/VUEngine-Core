/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ColliderManager.h>
#include <DebugConfig.h>
#include <Entity.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "EntityFactory.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================


friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

typedef uint32 (*InstantiationPhase)(void*);

/// @memberof EntityFactory
typedef struct PositionedEntityDescription
{
	const PositionedEntity* positionedEntity;
	Container parent;
	Entity entity;
	EventListener callback;
	int16 internalId;
	uint8 componentIndex;
	bool componentsCreated;
	bool wireframesCreated;
	bool collidersCreated;
	bool behaviorsCreated;
	bool transformed;
	bool graphicsSynchronized;

} PositionedEntityDescription;


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

static const InstantiationPhase _instantiationPhases[] =
{
	&EntityFactory::instantiateEntities,
	&EntityFactory::transformEntities,
	&EntityFactory::addChildEntities
};

static int32 _instantiationPhasesCount = sizeof(_instantiationPhases) / sizeof(InstantiationPhase);


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void EntityFactory::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->entitiesToInstantiate = new VirtualList();
	this->entitiesToTransform = new VirtualList();
	this->entitiesToAddAsChildren = new VirtualList();
	this->spawnedEntities = new VirtualList();

	this->instantiationPhase = 0;
}
//---------------------------------------------------------------------------------------------------------
void EntityFactory::destructor()
{
	VirtualNode node = this->entitiesToInstantiate->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	delete this->entitiesToInstantiate;
	this->entitiesToInstantiate = NULL;

	node = this->entitiesToTransform->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	delete this->entitiesToTransform;
	this->entitiesToTransform = NULL;

	node = this->entitiesToAddAsChildren->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	delete this->entitiesToAddAsChildren;
	this->entitiesToAddAsChildren = NULL;

	node = this->spawnedEntities->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		delete positionedEntityDescription;
	}

	delete this->spawnedEntities;
	this->spawnedEntities = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void EntityFactory::spawnEntity(const PositionedEntity* positionedEntity, Container parent, EventListener callback, int16 internalId)
{
	if(NULL == positionedEntity || NULL == parent)
	{
		return;
	}

	PositionedEntityDescription* positionedEntityDescription = new PositionedEntityDescription;

	positionedEntityDescription->positionedEntity = positionedEntity;
	positionedEntityDescription->parent = parent;
	positionedEntityDescription->entity = NULL;
	positionedEntityDescription->callback = callback;
	positionedEntityDescription->internalId = internalId;
	positionedEntityDescription->transformed = false;
	positionedEntityDescription->graphicsSynchronized = false;
	positionedEntityDescription->componentsCreated = false;
	positionedEntityDescription->componentIndex = 0;

	VirtualList::pushBack(this->entitiesToInstantiate, positionedEntityDescription);
}
//---------------------------------------------------------------------------------------------------------
bool EntityFactory::createNextEntity()
{
	EntityFactory::cleanUp(this);

	if(this->instantiationPhase >= _instantiationPhasesCount)
	{
		this->instantiationPhase = 0;
	}

	uint32 result = _instantiationPhases[this->instantiationPhase](this);

	int32 counter = _instantiationPhasesCount;

	while(__LIST_EMPTY == result)
	{
		if(!--counter)
		{
			return false;
		}

		this->instantiationPhase++;

		if(this->instantiationPhase >= _instantiationPhasesCount)
		{
			this->instantiationPhase = 0;
		}

		result = _instantiationPhases[this->instantiationPhase](this);
	}

	this->instantiationPhase += __ENTITY_PENDING_PROCESSING != result ? 1 : 0;

	return __LIST_EMPTY != result;
}
//---------------------------------------------------------------------------------------------------------
bool EntityFactory::hasEntitiesPending()
{
	return NULL != this->entitiesToInstantiate->head ||
			NULL != this->entitiesToTransform->head ||
			NULL != this->entitiesToAddAsChildren->head;
}
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
#ifdef __PROFILE_STREAMING
void EntityFactory::print(int32 x, int32 y)
{	int32 xDisplacement = 18;

	Printing::text(Printing::getInstance(), "Factory's status", x, y++, NULL);
	Printing::text(Printing::getInstance(), "", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Phase: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->instantiationPhase, x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "Entities pending...", x, y++, NULL);

	Printing::text(Printing::getInstance(), "1 Instantiation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "2 Transformation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToTransform), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "3 Make ready:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToAddAsChildren), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "4 Call listeners:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
#endif
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
uint32 EntityFactory::instantiateEntities()
{
	ASSERT(this, "EntityFactory::spawnEntities: null spawnEntities");

	if(NULL == this->entitiesToInstantiate->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToInstantiate->head->data;

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(!isDeleted(positionedEntityDescription->entity))
		{
			EntityFactory entityFactory = Entity::getEntityFactory(positionedEntityDescription->entity);

			if(NULL == entityFactory || __LIST_EMPTY == EntityFactory::instantiateEntities(entityFactory))
			{
				VirtualList::pushBack(this->entitiesToTransform, positionedEntityDescription);
				VirtualList::removeData(this->entitiesToInstantiate, positionedEntityDescription);

				return __ENTITY_PROCESSED;
			}

			return __ENTITY_PENDING_PROCESSING;
		}
		else
		{
			NM_ASSERT(!isDeleted(positionedEntityDescription), "EntityFactory::spawnEntities: deleted positionedEntityDescription");
			NM_ASSERT(NULL != positionedEntityDescription->positionedEntity, "EntityFactory::spawnEntities: null positionedEntity");
			NM_ASSERT(NULL != positionedEntityDescription->positionedEntity->entitySpec, "EntityFactory::spawnEntities: null spec");
			NM_ASSERT(NULL != positionedEntityDescription->positionedEntity->entitySpec->allocator, "EntityFactory::spawnEntities: no allocator defined");

			positionedEntityDescription->entity = Entity::createEntityDeferred(positionedEntityDescription->positionedEntity, positionedEntityDescription->internalId);
			NM_ASSERT(positionedEntityDescription->entity, "EntityFactory::spawnEntities: entity not loaded");

			if(!isDeleted(positionedEntityDescription->entity) && NULL != positionedEntityDescription->callback)
			{
				Entity::addEventListener(positionedEntityDescription->entity, ListenerObject::safeCast(positionedEntityDescription->parent), positionedEntityDescription->callback, kEventEntityLoaded);
			}
		}
	}
	else
	{
		VirtualList::removeData(this->entitiesToInstantiate, positionedEntityDescription);
		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}
//---------------------------------------------------------------------------------------------------------
uint32 EntityFactory::transformEntities()
{
	if(NULL == this->entitiesToTransform->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToTransform->head->data;
	ASSERT(positionedEntityDescription->entity, "EntityFactory::transformEntities: null entity");
	ASSERT(positionedEntityDescription->parent, "EntityFactory::transformEntities: null parent");

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(!positionedEntityDescription->transformed)
		{
			const Transformation* environmentTransform = Entity::getTransformation(positionedEntityDescription->parent);
			Entity::invalidateTransformation(positionedEntityDescription->entity);
			Entity::transform(positionedEntityDescription->entity, environmentTransform, false);

			positionedEntityDescription->transformed = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		if(!positionedEntityDescription->componentsCreated)
		{
			EntitySpec* entitySpec = Entity::getSpec(positionedEntityDescription->entity);
			
			if(NULL != entitySpec && NULL != entitySpec->componentSpecs && NULL != entitySpec->componentSpecs[positionedEntityDescription->componentIndex])
			{
				bool createdComponent = NULL != Entity::addComponent(positionedEntityDescription->entity, (ComponentSpec*)entitySpec->componentSpecs[positionedEntityDescription->componentIndex]);
				positionedEntityDescription->componentIndex++;

				if(createdComponent)
				{
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedEntityDescription->componentsCreated = true;
			positionedEntityDescription->componentIndex = 0;
		}

		EntityFactory entityFactory = Entity::getEntityFactory(positionedEntityDescription->entity);

		if(NULL == entityFactory || __LIST_EMPTY == EntityFactory::transformEntities(entityFactory))
		{
			VirtualList::pushBack(this->entitiesToAddAsChildren, positionedEntityDescription);
			VirtualList::removeData(this->entitiesToTransform, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->entitiesToTransform, positionedEntityDescription);

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}
//---------------------------------------------------------------------------------------------------------
uint32 EntityFactory::addChildEntities()
{
	if(NULL == this->entitiesToAddAsChildren->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToAddAsChildren->head->data;

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(!positionedEntityDescription->graphicsSynchronized)
		{
			Entity::calculateSize(positionedEntityDescription->entity);
			Entity::invalidateTransformation(positionedEntityDescription->entity);
			positionedEntityDescription->graphicsSynchronized = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		EntityFactory entityFactory = Entity::getEntityFactory(positionedEntityDescription->entity);

		if(NULL == entityFactory || __LIST_EMPTY == EntityFactory::addChildEntities(entityFactory))
		{
			NM_ASSERT(!isDeleted(positionedEntityDescription->parent), "EntityFactory::addChildEntities: deleted parent");

			// Must add the child to its parent before making it ready
			Container::addChild(positionedEntityDescription->parent, Container::safeCast(positionedEntityDescription->entity));

			VirtualList::pushBack(this->spawnedEntities, positionedEntityDescription);
			VirtualList::removeData(this->entitiesToAddAsChildren, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->entitiesToAddAsChildren, positionedEntityDescription);

		// don't need to delete the created entity since the parent takes care of that at this point

		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}
//---------------------------------------------------------------------------------------------------------
uint32 EntityFactory::cleanUp()
{
	if(NULL == this->spawnedEntities->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->spawnedEntities->head->data;

	if(!isDeleted(positionedEntityDescription->parent) && !isDeleted(positionedEntityDescription->entity))
	{
		if(NULL != positionedEntityDescription->callback)
		{
			Entity::fireEvent(positionedEntityDescription->entity, kEventEntityLoaded);
			NM_ASSERT(!isDeleted(positionedEntityDescription->entity), "EntityFactory::cleanUp: deleted entity during kEventEntityLoaded");
			Entity::removeEventListeners(positionedEntityDescription->entity, NULL, kEventEntityLoaded);
		}

		VirtualList::removeData(this->spawnedEntities, positionedEntityDescription);
		delete positionedEntityDescription;

		return __ENTITY_PROCESSED;
	}
	else
	{
		VirtualList::removeData(this->spawnedEntities, positionedEntityDescription);
		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}
//---------------------------------------------------------------------------------------------------------
