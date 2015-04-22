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

#ifndef OBJECT_SPRITE_H_
#define OBJECT_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSprite_METHODS													\
	Sprite_METHODS																\

// declare the virtual methods which are redefined
#define ObjectSprite_SET_VTABLE(ClassName)										\
	Sprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, ObjectSprite, render);								\
	__VIRTUAL_SET(ClassName, ObjectSprite, getPosition);						\
	__VIRTUAL_SET(ClassName, ObjectSprite, setPosition);						\
	__VIRTUAL_SET(ClassName, ObjectSprite, synchronizePosition);				\
	__VIRTUAL_SET(ClassName, ObjectSprite, setDirection);						\
	__VIRTUAL_SET(ClassName, ObjectSprite, calculateParallax);					\
	__VIRTUAL_SET(ClassName, ObjectSprite, hide);								\

#define ObjectSprite_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\
																				\
	/* parent sprite */															\
	ObjectSpriteContainer objectSpriteContainer;								\
																				\
	/* positioning */															\
	VBVec2D position;															\
																				\
	/* object index */															\
	int objectIndex;															\
																				\
	/* number of objects */														\
	u8 totalObjects;															\
	
// declare a ObjectSprite, which holds a texture and a drawing specification
__CLASS(ObjectSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct ObjectSpriteDefinition
{
	// the class type
	void* allocator;

	// texture to use with the sprite
	TextureDefinition* textureDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// flag to indicate in which display to show the bg texture
	u16 display;

	// parallax modifier to achieve better control over display
	s8 parallaxDisplacement;

} ObjectSpriteDefinition;

typedef const ObjectSpriteDefinition ObjectSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectSprite, const ObjectSpriteDefinition* oSpriteDefinition);

void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* oSpriteDefinition);
void ObjectSprite_destructor(ObjectSprite this);
void ObjectSprite_setDirection(ObjectSprite this, int axis, int direction);
VBVec2D ObjectSprite_getPosition(ObjectSprite this);
void ObjectSprite_setPosition(ObjectSprite this, VBVec2D position);
void ObjectSprite_synchronizePosition(ObjectSprite this, VBVec3D position3D);
void ObjectSprite_calculateParallax(ObjectSprite this, fix19_13 z);
void ObjectSprite_render(ObjectSprite this);
int ObjectSprite_getObjectIndex(ObjectSprite this);
void ObjectSprite_hide(ObjectSprite this);

#endif