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
	SmallRightCuboid smallRightCuboid;
	s16 id;
	long distance;

} PositionedEntityDescription;

typedef struct EntityDescription
{
	PositionedEntity* positionedEntity;
	Entity entity;

} EntityDescription;

//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
static void EntityFactory_constructor(EntityFactory this);
static void EntityFactory_initializeEntities(EntityFactory this);
static void EntityFactory_transformEntities(EntityFactory this);

typedef void (*StreamingPhase)(EntityFactory);

static const StreamingPhase _streamingPhases[] =
{
    &EntityFactory_initializeEntities,
    &EntityFactory_transformEntities,
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

	this->entitiesToInitialize = __NEW(VirtualList);
	this->entitiesToTransform = __NEW(VirtualList);
	this->delayPerCycle = 0;
    this->streamingPhase = 0;
    this->streamingCycleCounter = 0;
}

// class's destructor
void EntityFactory_destructor(EntityFactory this)
{
	ASSERT(this, "EntityFactory::destructor: null this");

	if(this->entitiesToInitialize)
	{
		VirtualNode node = this->entitiesToInitialize->head;

		for(; node; node = node->next)
		{
			EntityDescription* entityDescription = (EntityDescription*)node->data;

			__DELETE(entityDescription->entity);
			__DELETE_BASIC(entityDescription);
		}

		__DELETE(this->entitiesToInitialize);
		this->entitiesToInitialize = NULL;
	}

	if(this->entitiesToTransform)
    {
        VirtualNode node = this->entitiesToTransform->head;

        for(; node; node = node->next)
        {
            EntityDescription* entityDescription = (EntityDescription*)node->data;

            __DELETE(entityDescription->entity);
            __DELETE_BASIC(entityDescription);
        }

        __DELETE(this->entitiesToTransform);
        this->entitiesToTransform = NULL;
    }

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void EntityFactory_spawnEntity(EntityFactory this, PositionedEntity* positionedEntity, Object requester, EventListener callback, s16 id)
{
	ASSERT(this, "EntityFactory::spawnEntity: null this");

    Entity entity = Entity_loadFromDefinitionWithoutInitilization(positionedEntity, id);

	ASSERT(entity, "EntityFactory::spawnEntity: entity not loaded");

    Object_addEventListener(__SAFE_CAST(Object, entity), requester, callback, __EVENT_ENTITY_LOADED);

    EntityDescription* entityDescription = __NEW_BASIC(EntityDescription);
    entityDescription->positionedEntity = positionedEntity;
    entityDescription->entity = entity;
    VirtualList_pushBack(this->entitiesToInitialize, entityDescription);
}

void EntityFactory_setDelayPerCycle(EntityFactory this, int delayPerCycle)
{
	ASSERT(this, "EntityFactory::setDelayPerCycle: null this");

	this->delayPerCycle = delayPerCycle;
}

// initialize loaded entities
static void EntityFactory_initializeEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::initializeEntities: null this");

	if(!this->entitiesToInitialize->head)
	{
	    return;
	}

    EntityDescription* entityDescription = (EntityDescription*)this->entitiesToInitialize->head->data;

    __VIRTUAL_CALL(Entity, initialize, entityDescription->entity);

    VirtualList_removeElement(this->entitiesToInitialize, entityDescription);
    VirtualList_pushBack(this->entitiesToTransform, entityDescription);
}

// intialize loaded entities
static void EntityFactory_transformEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::transformEntities: null this");

    if(!this->entitiesToTransform->head)
    {
        return;
    }

	// static to avoid call to _memcpy
	static Transformation environmentTransform __INITIALIZED_DATA_SECTION_ATTRIBUTE =
	{
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// local rotation
			{0, 0, 0},
			// global rotation
			{0, 0, 0},
			// local scale
			{ITOFIX7_9(1), ITOFIX7_9(1)},
			// global scale
			{ITOFIX7_9(1), ITOFIX7_9(1)}
	};

    EntityDescription* entityDescription = (EntityDescription*)this->entitiesToTransform->head->data;

    // apply transformations
    __VIRTUAL_CALL(Container, initialTransform, entityDescription->entity, &environmentTransform);

    // call ready method
    __VIRTUAL_CALL(Entity, ready, entityDescription->entity);

    VirtualList_removeElement(this->entitiesToTransform, entityDescription);
    __DELETE_BASIC(entityDescription);

    Object_fireEvent(__SAFE_CAST(Object, entityDescription->entity), __EVENT_ENTITY_LOADED);

    Object_removeAllEventListeners(__SAFE_CAST(Object, entityDescription), __EVENT_ENTITY_LOADED);
}

void EntityFactory_prepareEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareEntities: null this");

    static int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);
	int streamingDelayPerCycle = this->delayPerCycle >> __FRAME_CYCLE;
	int streamingCycleDuration = streamingDelayPerCycle / streamingPhases;

	if(++this->streamingCycleCounter >= streamingCycleDuration)
	{
		this->streamingCycleCounter  = 0;

        if(++this->streamingPhase >= streamingPhases)
        {
            this->streamingPhase = 0;
        }

        _streamingPhases[this->streamingPhase](this);
	}
}

void EntityFactory_prepareAllEntities(EntityFactory this)
{
	ASSERT(this, "EntityFactory::prepareAllEntities: null this");

    while(this->entitiesToTransform->head)
    {
        EntityFactory_initializeEntities(this);
    }

    while(this->entitiesToInitialize->head)
    {
        EntityFactory_transformEntities(this);
    }
}
