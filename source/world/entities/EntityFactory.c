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


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------



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

} PositionedEntityDescription;

//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
static void EntityFactory_constructor(EntityFactory this);
u32 EntityFactory_spawnEntities(EntityFactory this);
u32 EntityFactory_initializeEntities(EntityFactory this);
u32 EntityFactory_transformEntities(EntityFactory this);
u32 EntityFactory_makeReadyEntities(EntityFactory this);
u32 EntityFactory_callLoadedEntities(EntityFactory this);

typedef u32 (*StreamingPhase)(EntityFactory);

static const StreamingPhase _streamingPhases[] =
{
    &EntityFactory_spawnEntities,
    &EntityFactory_initializeEntities,
    &EntityFactory_transformEntities,
    &EntityFactory_makeReadyEntities,
    &EntityFactory_callLoadedEntities
};


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

    this->entitiesToSpawn = __NEW(VirtualList);
	this->entitiesToInitialize = __NEW(VirtualList);
	this->entitiesToTransform = __NEW(VirtualList);
	this->entitiesToMakeReady = __NEW(VirtualList);
	this->loadedEntities = __NEW(VirtualList);

    this->streamingPhase = 0;
    this->streamingCycleCounter = 0;
}

// class's destructor
void EntityFactory_destructor(EntityFactory this)
{
	ASSERT(this, "EntityFactory::destructor: null this");

    VirtualNode node = this->entitiesToSpawn->head;

    for(; node; node = node->next)
    {
        PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
        {
            __DELETE(positionedEntityDescription->entity);
        }

        __DELETE_BASIC(positionedEntityDescription);
    }

    __DELETE(this->entitiesToSpawn);
    this->entitiesToSpawn = NULL;

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

    node = this->loadedEntities->head;

    for(; node; node = node->next)
    {
        PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)node->data;

        if(positionedEntityDescription->entity && *(u32*)positionedEntityDescription->entity)
        {
            __DELETE(positionedEntityDescription->entity);
        }

        __DELETE_BASIC(positionedEntityDescription);
    }

    __DELETE(this->loadedEntities);
    this->loadedEntities = NULL;

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

    VirtualList_pushBack(this->entitiesToSpawn, positionedEntityDescription);
}

u32 EntityFactory_spawnEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::spawnEntities: null spawnEntities");

    if(!this->entitiesToSpawn->head)
    {
        return __LIST_EMPTY;
    }

    PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->entitiesToSpawn->head->data;

    if(*(u32*)positionedEntityDescription->parent)
    {
        if(positionedEntityDescription->entity)
        {
            if(Entity_allChildrenSpawned(positionedEntityDescription->entity))
            {
                VirtualList_pushBack(this->entitiesToInitialize, positionedEntityDescription);
                VirtualList_removeElement(this->entitiesToSpawn, positionedEntityDescription);

                return __ENTITY_PROCESSED;
            }

            return __ENTITY_PENDING_PROCESSING;
        }
        else
        {
            positionedEntityDescription->entity = Entity_loadFromDefinitionWithoutInitilization(positionedEntityDescription->positionedEntity, positionedEntityDescription->id);
            ASSERT(positionedEntityDescription->entity, "EntityFactory::spawnEntities: entity not loaded");

            if(positionedEntityDescription->callback)
            {
                Object_addEventListener(__SAFE_CAST(Object, positionedEntityDescription->entity), __SAFE_CAST(Object, positionedEntityDescription->parent), positionedEntityDescription->callback, __EVENT_ENTITY_LOADED);
            }
        }
    }
    else
    {
        VirtualList_removeElement(this->entitiesToSpawn, positionedEntityDescription);
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

        if(Entity_allChildrenInitialized(positionedEntityDescription->entity))
        {
            __VIRTUAL_CALL(Entity, initialize, positionedEntityDescription->entity);

            VirtualList_pushBack(this->entitiesToTransform, positionedEntityDescription);
            VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);

            return __ENTITY_PROCESSED;
        }

        return __ENTITY_PENDING_PROCESSING;
    }
    else
    {
        VirtualList_removeElement(this->entitiesToInitialize, positionedEntityDescription);
        __DELETE(positionedEntityDescription->entity);
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
        if(Entity_allChildrenTransformed(positionedEntityDescription->entity))
        {
            Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, positionedEntityDescription->parent));

            __VIRTUAL_CALL(Container, initialTransform, positionedEntityDescription->entity, &environmentTransform);

            VirtualList_pushBack(this->entitiesToMakeReady, positionedEntityDescription);
            VirtualList_removeElement(this->entitiesToTransform, positionedEntityDescription);

            return __ENTITY_PROCESSED;
        }

        return __ENTITY_PENDING_PROCESSING;
    }
    else
    {
        VirtualList_removeElement(this->entitiesToTransform, positionedEntityDescription);
        __DELETE(positionedEntityDescription->entity);
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
        if(Entity_allChildrenReady(positionedEntityDescription->entity))
        {
            __VIRTUAL_CALL(Container, addChild, positionedEntityDescription->parent, __SAFE_CAST(Container, positionedEntityDescription->entity));

            // call ready method
            __VIRTUAL_CALL(Entity, ready, positionedEntityDescription->entity);

            VirtualList_pushBack(this->loadedEntities, positionedEntityDescription);
            VirtualList_removeElement(this->entitiesToMakeReady, positionedEntityDescription);

            return __ENTITY_PROCESSED;
        }

        return __ENTITY_PENDING_PROCESSING;
    }
    else
    {
        VirtualList_removeElement(this->entitiesToMakeReady, positionedEntityDescription);
        __DELETE(positionedEntityDescription->entity);
        __DELETE_BASIC(positionedEntityDescription);
    }

    return __ENTITY_PROCESSED;
}

u32 EntityFactory_callLoadedEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::callLoadedEntities: null this");

	if(!this->loadedEntities->head)
	{
	    return __LIST_EMPTY;
	}

    PositionedEntityDescription* positionedEntityDescription = (PositionedEntityDescription*)this->loadedEntities->head->data;

    if(*(u32*)positionedEntityDescription->entity)
    {
        if(Entity_allChildrenLoaded(positionedEntityDescription->entity))
        {
            Object_fireEvent(__SAFE_CAST(Object, positionedEntityDescription->entity), __EVENT_ENTITY_LOADED);

            Object_removeAllEventListeners(__SAFE_CAST(Object, positionedEntityDescription->entity), __EVENT_ENTITY_LOADED);

            VirtualList_removeElement(this->loadedEntities, positionedEntityDescription);
            __DELETE_BASIC(positionedEntityDescription);

            return __ENTITY_PROCESSED;
        }

        return __ENTITY_PENDING_PROCESSING;
    }
    else
    {
        VirtualList_removeElement(this->loadedEntities, positionedEntityDescription);
        __DELETE_BASIC(positionedEntityDescription);
    }

    return __ENTITY_PROCESSED;
}

int EntityFactory_prepareEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareEntities: null this");

    int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

    if(this->streamingPhase >= streamingPhases)
    {
        this->streamingPhase = 0;
    }

    this->streamingPhase += __ENTITY_PENDING_PROCESSING != _streamingPhases[this->streamingPhase](this)? 1 : 0;

    return this->streamingPhase >= streamingPhases - 1;
}

void EntityFactory_prepareAllEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareAllEntities: null this");

    while(this->entitiesToSpawn->head)
    {
        EntityFactory_spawnEntities(this);
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

    while(this->loadedEntities->head)
    {
        EntityFactory_callLoadedEntities(this);
    }
}

