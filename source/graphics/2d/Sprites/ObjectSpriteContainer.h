/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, synchronizePosition);		\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, setDirection);				\
	__VIRTUAL_SET(ClassName, ObjectSpriteContainer, calculateParallax);			\
	
#define ObjectSpriteContainer_ATTRIBUTES										\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\
																				\
	/* o sprites */																\
	VirtualList oSprites;														\
																				\
	/* o sprites */																\
	u16 availableObjects;														\
																				\
	/* o sprites */																\
	u16 nextAvailableObject;													\
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
s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite oSprite, int numberOfObjects);
void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite oSprite, int numberOfObjects);
bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, int numberOfObjects);
void ObjectSpriteContainer_setDirection(ObjectSpriteContainer this, int axis, int direction);
VBVec2D ObjectSpriteContainer_getPosition(ObjectSpriteContainer this);
void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, VBVec2D position);
void ObjectSpriteContainer_synchronizePosition(ObjectSpriteContainer this, VBVec3D position3D);
void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z);
void ObjectSpriteContainer_render(ObjectSpriteContainer this);


#endif