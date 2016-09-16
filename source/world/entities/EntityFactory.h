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
#include <Entity.h>


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
        /* the pivot node for streaming */ 																\
        VirtualNode streamingHeadNode;																	\
        /* the EntityFactory entities to test for streaming */ 											\
        VirtualList entitiesToTest;																        \
        /* the removed entities */ 																		\
        VirtualList removedEntities;																	\
        /* streaming's uninitialized entities */ 														\
        VirtualList entitiesToInitialize;																\
        /* streaming's non yet transformed entities */ 													\
        VirtualList entitiesToTransform;																\
        /* counter to control the streaming phses */													\
        int streamingCycleCounter;																		\
        /* index for method to execute */													            \
        int streamingPhase;                                                                             \
        /* cycle delay */													                            \
		int delayPerCycle;

// declare a EntityFactory, which holds the objects in a game world
__CLASS(EntityFactory);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(EntityFactory);

void EntityFactory_destructor(EntityFactory this);
int EntityFactory_prepareEntities(EntityFactory this);
void EntityFactory_prepareAllEntities(EntityFactory this);
void EntityFactory_spawnEntity(EntityFactory this, PositionedEntity* positionedEntity, Object requester, EventListener callback, s16 id);
void EntityFactory_setDelayPerCycle(EntityFactory this, int delayPerCycle);

#endif
