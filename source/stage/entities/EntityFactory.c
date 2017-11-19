/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

/**
 * @class	EntityFactory
 * @extends Object
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(EntityFactory, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * Positioned Entity Description
 *
 * @memberof EntityFactory
 */
typedef struct PositionedEntityDescription
{
	PositionedEntity* positionedEntity;
	Container parent;
	Entity entity;
	EventListener callback;
	int spriteDefinitionIndex;
	s16 id;
	bool transformed;
	bool initialized;
} PositionedEntityDescription;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
static void EntityFactory_constructor(EntityFactory this);
u32 EntityFactory_instantiateEntities(EntityFactory this);
u32 EntityFactory_initializeEntities(EntityFactory this);
u32 EntityFactory_transformEntities(EntityFactory this);
u32 EntityFactory_makeReadyEntities(EntityFactory this);
u32 EntityFactory_spawnEntities(EntityFactory this);

typedef u32 (*StreamingPhase)(EntityFactory);

static const StreamingPhase _streamingPhases[] =
{
	&EntityFactory_instantiateEntities,
	&EntityFactory_initializeEntities,
	&EntityFactory_transformEntities,
	&EntityFactory_makeReadyEntities
};

static int _streamingPhasesCount = sizeof(_streamingPhases) / sizeof(StreamingPhase);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(EntityFactory)
__CLASS_NEW_END(EntityFactory);

// class's constructor
static void EntityFactory_constructor(EntityFactory this)
{
	ASSERT(this, "EntityFactory::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	this->entitiesToInstantiate = __NEW(VirtualList);
	this->entitiesToInitialize = __NEW(VirtualList);
	this->entitiesToTransform = __NEW(VirtualList);
	this->entitiesToMakeReady = __NEW(VirtualList);
	this->spawnedEntities = __NEW(VirtualList);

	this->streamingPhase = 0;
}

// class's destructor
void EntityFactory_destructor(EntityFactory this)
{
	ASSERT(this, "EntityFactory::destructor: null this");

	VirtualNode node = this->entitiesToInstantiate->head;

	for(; node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	__DELETE(this->entitiesToInstantiate);
	this->entitiesToInstantiate = NULL;

	node = this->entitiesToInitialize->head;

	for(; node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	__DELETE(this->entitiesToInitialize);
	this->entitiesToInitialize = NULL;

	node = this->entitiesToTransform->head;

	for(; node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	__DELETE(this->entitiesToTransform);
	this->entitiesToTransform = NULL;

	node = this->entitiesToMakeReady->head;

	for(; node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	__DELETE(this->entitiesToMakeReady);
	this->entitiesToMakeReady = NULL;


	node = this->spawnedEntities->head;

	for(; node; node = node->next)
	{
		PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

		__DELETE_BASIC(positionedEntityDescription);
	}

	__DELETE(this->spawnedEntities);
	this->spawnedEntities = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void EntityFactory_spawnEntity(EntityFactory this, PositionedEntity* positionedEntity, Container parent, EventListener callback, s16 id)
{
	ASSERT(this, "EntityFactory::spawnEntity: null this");

	if(!positionedEntity || !parent)
	{
		return;
	}

	PositionedEntityDescription* positionedEntityDescription = __NEW_BASIC(PositionedEntityDescription);

	positionedEntityDescription->positionedEntity = positionedEntity;
	positionedEntityDescription->parent = parent;
	positionedEntityDescription->entity = NULL;
	positionedEntityDescription->callback = callback;
	positionedEntityDescription->id = id;
	positionedEntityDescription->initialized = false;
	positionedEntityDescription->transformed = false;
	positionedEntityDescription->spriteDefinitionIndex = 0;


	VirtualList_pushBack(this->entitiesToInstantiate, positionedEntityDescription);
}

u32 EntityFactory_instantiateEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::spawnEntities: null spawnEntities");

	if(!this->entitiesToInstantiate->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToInstantiate->head->data;

	if(__IS_OBJECT_ALIVE(positionedEntityDescription->parent))
	{
		if(positionedEntityDescription->entity)
		{
			if(Entity_areAllChildrenInstantiated(positionedEntityDescription->entity))
			{
				VirtualList_pushBack(this->entitiesToInitialize, positionedEntityDescription);
				VirtualList_removeElement(this->entitiesToInstantiate, positionedEntityDescription);

				return __ENTITY_PROCESSED;
			}

			return __ENTITY_PENDING_PROCESSING;
		}
		else
		{
			positionedEntityDescription->entity = Entity_loadEntityDeferred(positionedEntityDescription->positionedEntity, positionedEntityDescription->id);
			ASSERT(positionedEntityDescription->entity, "EntityFactory::spawnEntities: entity not loaded");

			if(positionedEntityDescription->callback)
			{
				Object_addEventListener(__SAFE_CAST(Object, positionedEntityDescription->entity), __SAFE_CAST(Object, positionedEntityDescription->parent), positionedEntityDescription->callback, kEventEntityLoaded);
			}
		}
	}
	else
	{
		VirtualList_removeElement(this->entitiesToInstantiate, positionedEntityDescription);
		__DELETE_BASIC(positionedEntityDescription);
	}

	return __ENTITY_PROCESSED;
}

// initialize loaded entities
u32 EntityFactory_initializeEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::initializeEntities: null this");

	if(!this->entitiesToInitialize->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToInitialize->head->data;

	if(__IS_OBJECT_ALIVE(positionedEntityDescription->parent))
	{
		ASSERT(positionedEntityDescription->entity, "EntityFactory::initializeEntities: entity not loaded");

		if(Entity_areAllChildrenInitialized(positionedEntityDescription->entity))
		{
			if(!positionedEntityDescription->initialized)
			{
				positionedEntityDescription->initialized = true;

				// call ready method
				__VIRTUAL_CALL(Entity, initialize, positionedEntityDescription->entity, false);

				return __ENTITY_PENDING_PROCESSING;
			}

			if(Entity_addSpriteFromDefinitionAtIndex(positionedEntityDescription->entity, positionedEntityDescription->spriteDefinitionIndex++))
			{
				return __ENTITY_PENDING_PROCESSING;
			}

			VirtualList_pushBack(this->entitiesToTransform, positionedEntityDescription);
			VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	return __ENTITY_PROCESSED;
}

// transformation spawned entities
u32 EntityFactory_transformEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::transformEntities: null this");

	if(!this->entitiesToTransform->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToTransform->head->data;
	ASSERT(positionedEntityDescription->entity, "EntityFactory::transformEntities: null entity");
	ASSERT(positionedEntityDescription->parent, "EntityFactory::transformEntities: null parent");

	if(__IS_OBJECT_ALIVE(positionedEntityDescription->parent))
	{
		if(!positionedEntityDescription->transformed)
		{
			positionedEntityDescription->transformed = true;

			__VIRTUAL_CALL(Container, addChild, positionedEntityDescription->parent, __SAFE_CAST(Container, positionedEntityDescription->entity));

			Transformation* environmentTransform = Container_getTransform(__SAFE_CAST(Container, positionedEntityDescription->parent));

			__VIRTUAL_CALL(Container, initialTransform, positionedEntityDescription->entity, environmentTransform, false);
		}

		if(Entity_areAllChildrenTransformed(positionedEntityDescription->entity))
		{
			VirtualList_pushBack(this->entitiesToMakeReady, positionedEntityDescription);
			VirtualList_removeElement(this->entitiesToTransform, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList_removeElement(this->entitiesToTransform, positionedEntityDescription);

		if(__IS_OBJECT_ALIVE(positionedEntityDescription->entity))
		{
			__DELETE(positionedEntityDescription->entity);
		}

		__DELETE_BASIC(positionedEntityDescription);
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory_makeReadyEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::makeReadyEntities: null this");

	if(!this->entitiesToMakeReady->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToMakeReady->head->data;

	if(__IS_OBJECT_ALIVE(positionedEntityDescription->parent))
	{
		if(Entity_areAllChildrenReady(positionedEntityDescription->entity))
		{
			// call ready method
			__VIRTUAL_CALL(Entity, ready, positionedEntityDescription->entity, false);

			VirtualList_pushBack(this->spawnedEntities, positionedEntityDescription);
			VirtualList_removeElement(this->entitiesToMakeReady, positionedEntityDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList_removeElement(this->entitiesToMakeReady, positionedEntityDescription);

		// don't need to delete the created entity since the parent takes care of that at this point

		__DELETE_BASIC(positionedEntityDescription);
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory_cleanUp(EntityFactory this)
{
	ASSERT(this, "EntityFactory::cleanUp: null this");

	if(!this->spawnedEntities->head)
	{
		return __LIST_EMPTY;
	}

	PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->spawnedEntities->head->data;

	if(__IS_OBJECT_ALIVE(positionedEntityDescription->parent) && __IS_OBJECT_ALIVE(positionedEntityDescription->entity))
	{
		if(positionedEntityDescription->callback)
		{
			Object_fireEvent(__SAFE_CAST(Object, positionedEntityDescription->entity), kEventEntityLoaded);

			Object_removeAllEventListeners(__SAFE_CAST(Object, positionedEntityDescription->entity), kEventEntityLoaded);
		}

		VirtualList_removeElement(this->spawnedEntities, positionedEntityDescription);
		__DELETE_BASIC(positionedEntityDescription);

		return __ENTITY_PROCESSED;
	}
	else
	{
		VirtualList_removeElement(this->spawnedEntities, positionedEntityDescription);
		__DELETE_BASIC(positionedEntityDescription);
	}

	return __ENTITY_PROCESSED;
}

u32 EntityFactory_prepareEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareEntities: null this");

	EntityFactory_cleanUp(this);

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

u32 EntityFactory_hasEntitiesPending(EntityFactory this)
{
	ASSERT(this, "EntityFactory::hasEntitiesPending: null this");

	return VirtualList_getSize(this->entitiesToInstantiate) ||
			VirtualList_getSize(this->entitiesToInitialize) ||
			VirtualList_getSize(this->entitiesToTransform) ||
			VirtualList_getSize(this->entitiesToMakeReady);
}

int EntityFactory_getPhase(EntityFactory this)
{
	return this->streamingPhase >= _streamingPhasesCount ? 0 : this->streamingPhase;
}

void EntityFactory_prepareAllEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareAllEntities: null this");

	while(this->entitiesToInstantiate->head)
	{
		EntityFactory_instantiateEntities(this);
	}

	while(this->entitiesToInitialize->head)
	{
		EntityFactory_initializeEntities(this);
	}

	while(this->entitiesToTransform->head)
	{
		EntityFactory_transformEntities(this);
	}

	while(this->entitiesToMakeReady->head)
	{
		EntityFactory_makeReadyEntities(this);
	}
}

#ifdef __PROFILE_STREAMING
void EntityFactory_showStatus(EntityFactory this __attribute__ ((unused)), int x, int y)
{
	ASSERT(this, "EntityFactory::showStreamingProfiling: null this");
	int xDisplacement = 18;

	Printing_text(Printing_getInstance(), "Factory's status", x, y++, NULL);
	Printing_text(Printing_getInstance(), "", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Phase: ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->streamingPhase, x + xDisplacement, y++, NULL);

	Printing_text(Printing_getInstance(), "Entities pending...", x, y++, NULL);

	Printing_text(Printing_getInstance(), "1 Instantiation:			", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

	Printing_text(Printing_getInstance(), "2 Initialization:			", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToInitialize), x + xDisplacement, y++, NULL);

	Printing_text(Printing_getInstance(), "3 Transformation:			", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToTransform), x + xDisplacement, y++, NULL);

	Printing_text(Printing_getInstance(), "4 Make ready:			", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToMakeReady), x + xDisplacement, y++, NULL);

	Printing_text(Printing_getInstance(), "5 Call listeners:			", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
