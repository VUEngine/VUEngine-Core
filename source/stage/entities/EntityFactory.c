/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <EntityFactory.h>
#include <Entity.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#include <Printing.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


// define the EntityFactory
__CLASS_DEFINITION(EntityFactory, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct PositionedEntityDescription
{
	PositionedEntity* positionedEntity;
	Container parent;
	Entity entity;
	EventListener callback;
	s16 id;
    u16 transformed;
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

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
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

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
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

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
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

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
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
    positionedEntityDescription->transformed = false;

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

    if(*(u32*)positionedEntityDescription->parent)
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

    if(*(u32*)positionedEntityDescription->parent)
    {
        ASSERT(positionedEntityDescription->entity, "EntityFactory::initializeEntities: entity not loaded");

        if(Entity_areAllChildrenInitialized(positionedEntityDescription->entity))
        {
            __VIRTUAL_CALL(Entity, initialize, positionedEntityDescription->entity, false);

            VirtualList_pushBack(this->entitiesToTransform, positionedEntityDescription);
            VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);

            return __ENTITY_PROCESSED;
        }

        return __ENTITY_PENDING_PROCESSING;
    }
    else
    {
        VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);

        if(*(u32*)positionedEntityDescription->entity)
        {
            __DELETE(positionedEntityDescription->entity);
        }

        __DELETE_BASIC(positionedEntityDescription);
    }

    return __ENTITY_PROCESSED;
}

// transform spawned entities
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

    if(*(u32*)positionedEntityDescription->parent)
    {
        if(!positionedEntityDescription->transformed)
        {
            positionedEntityDescription->transformed = true;

            Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, positionedEntityDescription->parent));

            __VIRTUAL_CALL(Container, initialTransform, positionedEntityDescription->entity, &environmentTransform, false);
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

        if(*(u32*)positionedEntityDescription->entity)
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

    if(*(u32*)positionedEntityDescription->parent)
    {
        if(Entity_areAllChildrenReady(positionedEntityDescription->entity))
        {
            __VIRTUAL_CALL(Container, addChild, positionedEntityDescription->parent, __SAFE_CAST(Container, positionedEntityDescription->entity));

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

        if(*(u32*)positionedEntityDescription->entity)
        {
            __DELETE(positionedEntityDescription->entity);
        }

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

    if(*(u32*)positionedEntityDescription->parent && *(u32*)positionedEntityDescription->entity)
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

    this->streamingPhase += __ENTITY_PENDING_PROCESSING != result? 1 : 0;

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
    return this->streamingPhase >= _streamingPhasesCount? 0: this->streamingPhase;
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
    int xDisplacement = 16;

    Printing_text(Printing_getInstance(), "", x, y++, NULL);

    Printing_text(Printing_getInstance(), "Entities pending...", x, y++, NULL);

    Printing_text(Printing_getInstance(), "Instantiation:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Initialization:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToInitialize), x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Transformation:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToTransform), x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Make ready:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->entitiesToMakeReady), x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Call listeners:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), VirtualList_getSize(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
