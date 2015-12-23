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

#ifndef MANAGED_ENTITY_H_
#define MANAGED_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Sprite.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ManagedEntity_METHODS																			\
		Entity_METHODS																					\

#define ManagedEntity_SET_VTABLE(ClassName)																\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ManagedEntity, initialTransform);										\
		__VIRTUAL_SET(ClassName, ManagedEntity, transform);												\

#define ManagedEntity_ATTRIBUTES																		\
																										\
	/* it is derived from*/																			\
	Entity_ATTRIBUTES																					\
																										\
	/* sprites' list */																					\
	VirtualList managedSprites;																			\
																										\
	/* previous 2d projected position */																\
	VBVec2D previous2DPosition;																			\

__CLASS(ManagedEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef EntityDefinition ManagedEntityDefinition;
typedef const ManagedEntityDefinition ManagedEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ManagedEntity, ManagedEntityDefinition* managedEntityDefinition, s16 id, const char* const name);

void ManagedEntity_constructor(ManagedEntity this, ManagedEntityDefinition* managedEntityDefinition, s16 id, const char* const name);
void ManagedEntity_destructor(ManagedEntity this);
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform);
void ManagedEntity_transform(ManagedEntity this, const Transformation* environmentTransform);


#endif