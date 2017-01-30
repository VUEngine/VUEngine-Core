/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

#define __OBJECT_CHAR_SHOW_MASK						0xC000
#define __OBJECT_CHAR_HIDE_MASK						0x3FFF


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSprite_METHODS(ClassName)																	\
	    Sprite_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define ObjectSprite_SET_VTABLE(ClassName)																\
        Sprite_SET_VTABLE(ClassName)																	\
        __VIRTUAL_SET(ClassName, ObjectSprite, render);													\
        __VIRTUAL_SET(ClassName, ObjectSprite, getPosition);											\
        __VIRTUAL_SET(ClassName, ObjectSprite, setPosition);											\
        __VIRTUAL_SET(ClassName, ObjectSprite, position);												\
        __VIRTUAL_SET(ClassName, ObjectSprite, setDirection);											\
        __VIRTUAL_SET(ClassName, ObjectSprite, calculateParallax);										\
        __VIRTUAL_SET(ClassName, ObjectSprite, getWorldLayer);											\
        __VIRTUAL_SET(ClassName, ObjectSprite, addDisplacement);										\

#define ObjectSprite_ATTRIBUTES																			\
        /* super's attributes */																		\
        Sprite_ATTRIBUTES																				\
        /* parent sprite */																				\
        ObjectSpriteContainer objectSpriteContainer;													\
        /* positioning */																				\
        VBVec2D position;																				\
        /* object index */																				\
        s16 objectIndex;																				\
        /* number of objects */																			\
        s16 totalObjects;																				\

__CLASS(ObjectSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct ObjectSpriteDefinition
{
	// it has a Sprite definition at the beginning
	SpriteDefinition spriteDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// flag to indicate in which display to show the bg texture
	u16 display;

} ObjectSpriteDefinition;

typedef const ObjectSpriteDefinition ObjectSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectSprite, const ObjectSpriteDefinition* oSpriteDefinition, Object owner);

void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* oSpriteDefinition, Object owner);
void ObjectSprite_destructor(ObjectSprite this);
void ObjectSprite_setDirection(ObjectSprite this, int axis, int direction);
VBVec2D ObjectSprite_getPosition(ObjectSprite this);
void ObjectSprite_setPosition(ObjectSprite this, const VBVec2D* position);
void ObjectSprite_position(ObjectSprite this, const VBVec3D* position3D);
void ObjectSprite_calculateParallax(ObjectSprite this, fix19_13 z);
void ObjectSprite_render(ObjectSprite this);
s16 ObjectSprite_getTotalObjects(ObjectSprite this);
s16 ObjectSprite_getObjectIndex(ObjectSprite this);
void ObjectSprite_setObjectIndex(ObjectSprite this, s16 objectIndex);
u8 ObjectSprite_getWorldLayer(ObjectSprite this);
void ObjectSprite_addDisplacement(ObjectSprite this, const VBVec2D* displacement);


#endif
