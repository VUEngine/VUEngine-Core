/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CollisionManager.h>
#include <DebugConfig.h>
#include <Entity.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "EntityFactory.h"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

typedef uint32 (*StreamingPhase)(void*);

static const StreamingPhase _streamingPhases[] =
{
	&EntityFactory::instantiateEntities,
	&EntityFactory::transformEntities,
	&EntityFactory::makeReadyEntities
};

static int32 _streamingPhasesCount = sizeof(_streamingPhases) / sizeof(StreamingPhase);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void EntityFactory::constructor()
{
	// construct base object
	Base::constructor();

	this->entitiesToInstantiate = new VirtualList();
	this->entitiesToTransform = new VirtualList();
	this->entitiesToMakeReady = new VirtualList();
	this->spawnedEntities = new VirtualList();

	this->streamingPhase = 0;
}

// class's destructor
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

	node = this->entitiesToMakeReady->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	delete this->entitiesToMakeReady;
	this->entitiesToMakeReady = NULL;

	node = this->spawnedEntities->head;

	for(; NULL != node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		delete positionedEntityDescription;
	}

	delete this->spawnedEntities;
	this->spawnedEntities = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

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
	positionedEntityDescription->spritesCreated = false;
	positionedEntityDescription->wireframesCreated = false;
	positionedEntityDescription->collidersCreated = false;
	positionedEntityDescription->behaviorsCreated = false;
	positionedEntityDescription->componentIndex = 0;

	VirtualList::pushBack(this->entitiesToInstantiate, positionedEntityDescription);
}

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
			if(Entity::areAllChildrenInstantiated(positionedEntityDescription->entity))
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

			positionedEntityDescription->entity = Entity::loadEntityDeferred(positionedEntityDescription->positionedEntity, positionedEntityDescription->internalId);
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

// transformation spawned entities
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
			Entity::invalidateGlobalTransformation(positionedEntityDescription->entity);
			Entity::transform(positionedEntityDescription->entity, environmentTransform, false);

			positionedEntityDescription->transformed = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		if(!positionedEntityDescription->spritesCreated)
		{
			EntitySpec* entitySpec = Entity::getSpec(positionedEntityDescription->entity);
			
			if(NULL != entitySpec && NULL != entitySpec->spriteSpecs && NULL != entitySpec->spriteSpecs[positionedEntityDescription->componentIndex])
			{
				bool createdComponent = NULL != Entity::addSprite(positionedEntityDescription->entity, entitySpec->spriteSpecs[positionedEntityDescription->componentIndex], SpriteManager::getInstance());
				positionedEntityDescription->componentIndex++;

				if(createdComponent)
				{
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedEntityDescription->spritesCreated = true;
			positionedEntityDescription->componentIndex = 0;
		}

		if(!positionedEntityDescription->wireframesCreated)
		{
			EntitySpec* entitySpec = Entity::getSpec(positionedEntityDescription->entity);
			
			if(NULL != entitySpec && NULL != entitySpec->wireframeSpecs && NULL != entitySpec->wireframeSpecs[positionedEntityDescription->componentIndex])
			{
				bool createdComponent = NULL != Entity::addWireframe(positionedEntityDescription->entity, entitySpec->wireframeSpecs[positionedEntityDescription->componentIndex], WireframeManager::getInstance());
				positionedEntityDescription->componentIndex++;

				if(createdComponent)
				{
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedEntityDescription->wireframesCreated = true;
			positionedEntityDescription->componentIndex = 0;
		}

		if(!positionedEntityDescription->collidersCreated)
		{
			EntitySpec* entitySpec = Entity::getSpec(positionedEntityDescription->entity);
			
			if(NULL != entitySpec && NULL != entitySpec->colliderSpecs[positionedEntityDescription->componentIndex].allocator)
			{
				bool createdComponent = NULL != Entity::addCollider(positionedEntityDescription->entity, &entitySpec->colliderSpecs[positionedEntityDescription->componentIndex], VUEngine::getCollisionManager(VUEngine::getInstance()));
				positionedEntityDescription->componentIndex++;

				if(createdComponent)
				{
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedEntityDescription->collidersCreated = true;
			positionedEntityDescription->componentIndex = 0;
		}

		if(!positionedEntityDescription->behaviorsCreated)
		{
			EntitySpec* entitySpec = Entity::getSpec(positionedEntityDescription->entity);
			
			if(NULL != entitySpec && NULL != entitySpec->behaviorSpecs && NULL != entitySpec->behaviorSpecs[positionedEntityDescription->componentIndex])
			{
				bool createdComponent = NULL != Entity::addBehavior(positionedEntityDescription->entity, entitySpec->behaviorSpecs[positionedEntityDescription->componentIndex]);
				positionedEntityDescription->componentIndex++;

				if(createdComponent)
				{
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedEntityDescription->behaviorsCreated = true;
			positionedEntityDescription->componentIndex = 0;
		}

		if(Entity::areAllChildrenTransformed(positionedEntityDescription->entity))
		{
			VirtualList::pushBack(this->entitiesToMakeReady, positionedEntityDescription);
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

uint32 EntityFactory::makeReadyEntities()
{
	if(NULL == this->entitiesToMakeReady->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToMakeReady->head->data;

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(!positionedEntityDescription->graphicsSynchronized)
		{
			Entity::calculateSize(positionedEntityDescription->entity, false);
			Entity::invalidateGlobalTransformation(positionedEntityDescription->entity);
			positionedEntityDescription->graphicsSynchronized = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		if(Entity::areAllChildrenReady(positionedEntityDescription->entity))
		{
			NM_ASSERT(!isDeleted(positionedEntityDescription->parent), "EntityFactory::makeReadyEntities: deleted parent");

			// Must add the child to its parent before making it ready
			Container::addChild(positionedEntityDescription->parent, Container::safeCast(positionedEntityDescription->entity));

			VirtualList::pushBack(this->spawnedEntities, positionedEntityDescription);
			VirtualList::removeData(this->entitiesToMakeReady, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->entitiesToMakeReady, positionedEntityDescription);

		// don't need to delete the created entity since the parent takes care of that at this point

		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}

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

uint32 EntityFactory::prepareEntities()
{
	EntityFactory::cleanUp(this);

	if(this->streamingPhase >= _streamingPhasesCount)
	{
		this->streamingPhase = 0;
	}

	uint32 result = _streamingPhases[this->streamingPhase](this);

	int32 counter = _streamingPhasesCount;

	while(__LIST_EMPTY == result)
	{
		if(!--counter)
		{
			return false;
		}

		this->streamingPhase++;

		if(this->streamingPhase >= _streamingPhasesCount)
		{
			this->streamingPhase = 0;
		}

		result = _streamingPhases[this->streamingPhase](this);
	}

	this->streamingPhase += __ENTITY_PENDING_PROCESSING != result ? 1 : 0;

	return __LIST_EMPTY != result;
}

uint32 EntityFactory::hasEntitiesPending()
{
	return NULL != this->entitiesToInstantiate->head ||
			NULL != this->entitiesToTransform->head ||
			NULL != this->entitiesToMakeReady->head;
}

int32 EntityFactory::getPhase()
{
	return this->streamingPhase >= _streamingPhasesCount ? 0 : this->streamingPhase;
}


// Something is not working properly
void EntityFactory::prepareAllEntities()
{
	while(!isDeleted(this->entitiesToInstantiate->head))
	{
		EntityFactory::instantiateEntities(this);
	}

	while(!isDeleted(this->entitiesToTransform->head))
	{
		EntityFactory::transformEntities(this);
	}

	while(!isDeleted(this->entitiesToMakeReady->head))
	{
		EntityFactory::makeReadyEntities(this);
	}

	EntityFactory::cleanUp(this);
}

#ifndef __SHIPPING
#ifdef __PROFILE_STREAMING
void EntityFactory::showStatus(int32 x, int32 y)
{	int32 xDisplacement = 18;

	Printing::text(Printing::getInstance(), "Factory's status", x, y++, NULL);
	Printing::text(Printing::getInstance(), "", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Phase: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->streamingPhase, x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "Entities pending...", x, y++, NULL);

	Printing::text(Printing::getInstance(), "1 Instantiation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "2 Transformation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToTransform), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "3 Make ready:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToMakeReady), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "4 Call listeners:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
#endif
