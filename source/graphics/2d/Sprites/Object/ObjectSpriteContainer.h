/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef OBJECT_SPRITE_CONTAINER_H_
#define OBJECT_SPRITE_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER	256


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSpriteContainer_METHODS											\
	Sprite_METHODS																\

// declare the virtual methods which are redefined
#define ObjectSpriteContainer_SET_VTABLE(ClassName)								\
	Sprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, render);					\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, getPosition);				\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, setPosition);				\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, positione);					\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, setDirection);				\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, calculateParallax);			\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, show);						\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, hide);						\

#define ObjectSpriteContainer_ATTRIBUTES										\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\
																				\
	/* object sprites */														\
	VirtualList objectSprites;													\
																				\
	/* next object sprite to defragment */										\
	VirtualNode objectSpriteToDefragment;										\
																				\
	/* for WORLD sorting */														\
	fix19_13 z;																	\
																				\
	/* used for defragmentation */												\
	int freedObjectIndex;														\
																				\
	/* OBJs available */														\
	u16 availableObjects;														\
																				\
	/* spt index */																\
	u8 spt;																		\

// declare a ObjectSpriteContainer, which holds a texture and a drawing specification
__CLASS(ObjectSpriteContainer);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, u8 spt);
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this);
s16 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects);
void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s16 numberOfObjects);
bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, s16 numberOfObjects);
void ObjectSpriteContainer_setDirection(ObjectSpriteContainer this, int axis, int direction);
const VBVec2D* ObjectSpriteContainer_getPosition(ObjectSpriteContainer this);
void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, const VBVec2D* position);
void ObjectSpriteContainer_positione(ObjectSpriteContainer this, VBVec3D position3D);
void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z);
void ObjectSpriteContainer_render(ObjectSpriteContainer this);
void ObjectSpriteContainer_show(ObjectSpriteContainer this);
void ObjectSpriteContainer_hide(ObjectSpriteContainer this);
u16 ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this);
int ObjectSpriteContainer_getTotalUsedObjects(ObjectSpriteContainer this);
int ObjectSpriteContainer_getNextFreeObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this);
void ObjectSpriteContainer_print(ObjectSpriteContainer this, int x, int y);


#endif