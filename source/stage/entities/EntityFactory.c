/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <EntityFactory.h>
#include <Entity.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#endif


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

typedef u32 (*StreamingPhase)(void*);

static const StreamingPhase _streamingPhases[] =
{
	&EntityFactory::instantiateEntities,
	&EntityFactory::transformEntities,
	&EntityFactory::makeReadyEntities
};

static int _streamingPhasesCount = sizeof(_streamingPhases) / sizeof(StreamingPhase);


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

	for(; node; node = node->next)
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

	for(; node; node = node->next)
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

	for(; node; node = node->next)
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

	for(; node; node = node->next)
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

void EntityFactory::spawnEntity(const PositionedEntity* positionedEntity, Container parent, EventListener callback, s16 internalId)
{
	if(!positionedEntity || !parent)
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
	positionedEntityDescription->spriteSpecIndex = 0;
	positionedEntityDescription->shapeSpecIndex = 0;
	positionedEntityDescription->transformedShapeSpecIndex = 0;

	VirtualList::pushBack(this->entitiesToInstantiate, positionedEntityDescription);
}

u32 EntityFactory::instantiateEntities()
{
	ASSERT(this, "EntityFactory::spawnEntities: null spawnEntities");

	if(!this->entitiesToInstantiate->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToInstantiate->head->data;

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(positionedEntityDescription->entity)
		{
			if(Entity::areAllChildrenInstantiated(positionedEntityDescription->entity))
			{
				if(0 <= positionedEntityDescription->spriteSpecIndex && Entity::addSpriteFromSpecAtIndex(positionedEntityDescription->entity, positionedEntityDescription->spriteSpecIndex++))
				{
					return __ENTITY_PENDING_PROCESSING;
				}
				else
				{
					positionedEntityDescription->spriteSpecIndex = -1;
				}

				if(0 <= positionedEntityDescription->shapeSpecIndex && Entity::addShapeFromSpecAtIndex(positionedEntityDescription->entity, positionedEntityDescription->shapeSpecIndex++))
				{
					return __ENTITY_PENDING_PROCESSING;
				}
				else
				{
					positionedEntityDescription->shapeSpecIndex = -1;
				}

				VirtualList::pushBack(this->entitiesToTransform, positionedEntityDescription);
				VirtualList::removeElement(this->entitiesToInstantiate, positionedEntityDescription);

				return __ENTITY_PROCESSED;
			}

			return __ENTITY_PENDING_PROCESSING;
		}
		else
		{
			positionedEntityDescription->entity = Entity::loadEntityDeferred(positionedEntityDescription->positionedEntity, positionedEntityDescription->internalId);
			ASSERT(positionedEntityDescription->entity, "EntityFactory::spawnEntities: entity not loaded");

			if(positionedEntityDescription->callback)
			{
				Object::addEventListener(positionedEntityDescription->entity, Object::safeCast(positionedEntityDescription->parent), positionedEntityDescription->callback, kEventEntityLoaded);
			}
		}
	}
	else
	{
		VirtualList::removeElement(this->entitiesToInstantiate, positionedEntityDescription);
		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}

// transformation spawned entities
u32 EntityFactory::transformEntities()
{
	if(!this->entitiesToTransform->head)
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
			positionedEntityDescription->transformed = true;

			Transformation* environmentTransform = Container::getTransform(positionedEntityDescription->parent);

			Container::initialTransform(positionedEntityDescription->entity, environmentTransform, false);

			return __ENTITY_PENDING_PROCESSING;
		}

		if(0 <= positionedEntityDescription->transformedShapeSpecIndex && Entity::transformShapeAtSpecIndex(positionedEntityDescription->entity, positionedEntityDescription->transformedShapeSpecIndex++))
		{
			return __ENTITY_PENDING_PROCESSING;
		}
		else
		{
			positionedEntityDescription->transformedShapeSpecIndex = -1;
		}

		if(Entity::areAllChildrenTransformed(positionedEntityDescription->entity))
		{
			VirtualList::pushBack(this->entitiesToMakeReady, positionedEntityDescription);
			VirtualList::removeElement(this->entitiesToTransform, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeElement(this->entitiesToTransform, positionedEntityDescription);

		if(!isDeleted(positionedEntityDescription->entity))
		{
			Entity::deleteMyself(positionedEntityDescription->entity);
		}

		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory::makeReadyEntities()
{
	if(!this->entitiesToMakeReady->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToMakeReady->head->data;

	if(!isDeleted(positionedEntityDescription->parent))
	{
		if(Entity::areAllChildrenReady(positionedEntityDescription->entity))
		{
			// Maybe it is needed another list and phase for this
			Entity::synchronizeGraphics(positionedEntityDescription->entity);

			// Must add the child to its parent before making it ready
			Container::addChild(positionedEntityDescription->parent, Container::safeCast(positionedEntityDescription->entity));

			// call ready method
			Entity::ready(positionedEntityDescription->entity, false);

			VirtualList::pushBack(this->spawnedEntities, positionedEntityDescription);
			VirtualList::removeElement(this->entitiesToMakeReady, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeElement(this->entitiesToMakeReady, positionedEntityDescription);

		// don't need to delete the created entity since the parent takes care of that at this point

		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory::cleanUp()
{
	if(!this->spawnedEntities->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->spawnedEntities->head->data;

	if(!isDeleted(positionedEntityDescription->parent) && !isDeleted(positionedEntityDescription->entity))
	{
		if(positionedEntityDescription->callback)
		{
			Entity::fireEvent(positionedEntityDescription->entity, kEventEntityLoaded);
			NM_ASSERT(!isDeleted(positionedEntityDescription->entity), "EntityFactory::cleanUp: deleted entity during kEventEntityLoaded");

			Entity::removeAllEventListeners(positionedEntityDescription->entity, kEventEntityLoaded);
		}

		VirtualList::removeElement(this->spawnedEntities, positionedEntityDescription);
		delete positionedEntityDescription;

		return __ENTITY_PROCESSED;
	}
	else
	{
		VirtualList::removeElement(this->spawnedEntities, positionedEntityDescription);
		delete positionedEntityDescription;
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory::prepareEntities()
{
	EntityFactory::cleanUp(this);

	if(this->streamingPhase >= _streamingPhasesCount)
	{
		this->streamingPhase = 0;
	}

	u32 result = _streamingPhases[this->streamingPhase](this);

	int counter = _streamingPhasesCount;

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

u32 EntityFactory::hasEntitiesPending()
{
	return VirtualList::getSize(this->entitiesToInstantiate) ||
			VirtualList::getSize(this->entitiesToTransform) ||
			VirtualList::getSize(this->entitiesToMakeReady);
}

int EntityFactory::getPhase()
{
	return this->streamingPhase >= _streamingPhasesCount ? 0 : this->streamingPhase;
}


// Something is not working properly
void EntityFactory::prepareAllEntities()
{
	while(this->entitiesToInstantiate->head)
	{
		EntityFactory::instantiateEntities(this);
	}

	while(this->entitiesToTransform->head)
	{
		EntityFactory::transformEntities(this);
	}

	while(this->entitiesToMakeReady->head)
	{
		EntityFactory::makeReadyEntities(this);
	}

	EntityFactory::cleanUp(this);
}

#ifdef __PROFILE_STREAMING
void EntityFactory::showStatus(int x, int y)
{	int xDisplacement = 18;

	Printing::text(Printing::getInstance(), "Factory's status", x, y++, NULL);
	Printing::text(Printing::getInstance(), "", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Phase: ", x, y, NULL);
	Printing::int(Printing::getInstance(), this->streamingPhase, x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "Entities pending...", x, y++, NULL);

	Printing::text(Printing::getInstance(), "1 Instantiation:			", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "2 Transformation:			", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->entitiesToTransform), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "3 Make ready:			", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->entitiesToMakeReady), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "4 Call listeners:			", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
