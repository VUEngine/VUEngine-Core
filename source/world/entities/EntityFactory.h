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

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>
#include <Container.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ENTITY_PENDING_PROCESSING     0x00
#define __LIST_EMPTY                    0x01
#define __ENTITY_PROCESSED              0x02


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define EntityFactory_METHODS(ClassName)																\
		Object_METHODS(ClassName)																	    \

// declare the virtual methods which are redefined
#define EntityFactory_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

#define EntityFactory_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																			    \
        /* the EntityFactory entities to test for streaming */ 											\
        VirtualList entitiesToSpawn;																    \
        /* streaming's uninitialized entities */ 														\
        VirtualList entitiesToInitialize;																\
        /* streaming's non yet transformed entities */ 													\
        VirtualList entitiesToTransform;																\
        /* streaming's non yet transformed entities */ 													\
        VirtualList entitiesToMakeReady;															    \
        /* index for method to execute */													            \
        int streamingPhase;                                                                             \

// declare a EntityFactory, which holds the objects in a game world
__CLASS(EntityFactory);

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines an entity in ROM memory
typedef struct EntityDefinition
{
	// the class allocator
	AllocatorPointer allocator;

	// the sprite
	const SpriteDefinition** spritesDefinitions;

} EntityDefinition;

typedef const EntityDefinition EntityROMDef;


// an entity associated with a position
typedef const struct PositionedEntity
{
	// pointer to the entity definition in ROM
	EntityDefinition* entityDefinition;

	// position in the world
	VBVec3D position;

	// name
	char* name;

	// the children
	struct PositionedEntity* childrenDefinitions;

	// extra info
	void* extraInfo;

	// force load
	bool loadRegardlessOfPosition;

} PositionedEntity;

typedef const PositionedEntity PositionedEntityROMDef;

//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(EntityFactory);

void EntityFactory_destructor(EntityFactory this);
void EntityFactory_prepareEntities(EntityFactory this);
void EntityFactory_prepareAllEntities(EntityFactory this);
void EntityFactory_spawnEntity(EntityFactory this, PositionedEntity* positionedEntity, Container parent, EventListener callback, s16 id);

#endif
