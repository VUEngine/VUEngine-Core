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

#ifndef COLLISIONS_CONTAINER_ENTITY_H_
#define COLLISIONS_CONTAINER_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define CollisionsContainerEntity_METHODS(ClassName)													\
		Entity_METHODS(ClassName)																		\

#define CollisionsContainerEntity_SET_VTABLE(ClassName)													\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, update);									\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, transform);									\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, updateVisualRepresentation);				\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, handleMessage);								\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, updateSpritePosition);						\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, updateSpriteRotation);				        \
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, updateSpriteScale);				            \
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, ready);										\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, handlePropagatedMessage);					\
		__VIRTUAL_SET(ClassName, CollisionsContainerEntity, passMessage);								\

#define CollisionsContainerEntity_ATTRIBUTES															\
        /* it is derived from */																		\
        Entity_ATTRIBUTES																				\

__CLASS(CollisionsContainerEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef EntityDefinition CollisionsContainerEntityDefinition;
typedef const CollisionsContainerEntityDefinition CollisionsContainerEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionsContainerEntity, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, const char* const name);

void CollisionsContainerEntity_constructor(CollisionsContainerEntity this, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, const char* const name);
void CollisionsContainerEntity_destructor(CollisionsContainerEntity this);
void CollisionsContainerEntity_ready(CollisionsContainerEntity this, u32 recursive);
void CollisionsContainerEntity_update(CollisionsContainerEntity this, u32 elapsedTime);
void CollisionsContainerEntity_transform(CollisionsContainerEntity this, const Transformation* environmentTransform);
void CollisionsContainerEntity_updateVisualRepresentation(CollisionsContainerEntity this);
bool CollisionsContainerEntity_handleMessage(CollisionsContainerEntity this, Telegram telegram);
bool CollisionsContainerEntity_updateSpritePosition(CollisionsContainerEntity this);
bool CollisionsContainerEntity_updateSpriteRotation(CollisionsContainerEntity this);
bool CollisionsContainerEntity_updateSpriteScale(CollisionsContainerEntity this);
bool CollisionsContainerEntity_handlePropagatedMessage(CollisionsContainerEntity this, int message);
int CollisionsContainerEntity_passMessage(CollisionsContainerEntity this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);


#endif
