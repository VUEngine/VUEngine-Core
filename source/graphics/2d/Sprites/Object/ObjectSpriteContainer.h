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

#ifndef OBJECT_SPRITE_CONTAINER_H_
#define OBJECT_SPRITE_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_CHAR_OBJECTS	1024


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSpriteContainer_METHODS(ClassName)														\
	    Sprite_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define ObjectSpriteContainer_SET_VTABLE(ClassName)														\
        Sprite_SET_VTABLE(ClassName)																	\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, render);										\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, getPosition);									\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, setPosition);									\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, position);										\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, setDirection);									\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, calculateParallax);								\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, show);											\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, hide);											\
        __VIRTUAL_SET(ClassName, ObjectSpriteContainer, addDisplacement);								\

#define ObjectSpriteContainer_ATTRIBUTES																\
        /* super's attributes */																		\
        Sprite_ATTRIBUTES																				\
        /* object sprites */																			\
        VirtualList objectSprites;																		\
        /* for z sorting */																				\
        VirtualNode node;																				\
        VirtualNode previousNode;																		\
        /* next object sprite node to defragment */														\
        VirtualNode objectSpriteNodeToDefragment;														\
        /* for WORLD sorting */																			\
        fix19_13 z;											    										\
        /* used for defragmentation */																	\
        int freedObjectIndex;																			\
        /* first object index */																		\
        int firstObjectIndex;																			\
        /* total objects */																				\
        int totalObjects;																				\
        /* OBJs available */																			\
        int availableObjects;																			\
        /* spt index */																					\
        int spt;																						\
        /* flag to halt defragmentation while sprite removal is taking place */							\
        bool removingObjectSprite;																		\

__CLASS(ObjectSpriteContainer);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, int spt, int totalObjects, int firstObjectIndex);
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this);
s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects);
void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s32 numberOfObjects);
bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, s32 numberOfObjects);
void ObjectSpriteContainer_setDirection(ObjectSpriteContainer this, int axis, int direction);
VBVec2D ObjectSpriteContainer_getPosition(ObjectSpriteContainer this);
void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, const VBVec2D* position);
void ObjectSpriteContainer_position(ObjectSpriteContainer this, const VBVec3D* position);
void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z);
void ObjectSpriteContainer_render(ObjectSpriteContainer this);
void ObjectSpriteContainer_show(ObjectSpriteContainer this);
void ObjectSpriteContainer_hide(ObjectSpriteContainer this);
int ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this);
int ObjectSpriteContainer_getTotalUsedObjects(ObjectSpriteContainer this);
int ObjectSpriteContainer_getNextFreeObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this);
void ObjectSpriteContainer_addDisplacement(ObjectSpriteContainer this, const VBVec2D* displacement);
void ObjectSpriteContainer_print(ObjectSpriteContainer this, int x, int y);


#endif
