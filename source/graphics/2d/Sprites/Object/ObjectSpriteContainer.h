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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_CHAR_OBJECTS	1024


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, show);											\
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, hide);											\
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, addDisplacement);								\
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, setMode);										\
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, writeTextures);									\
		__VIRTUAL_SET(ClassName, ObjectSpriteContainer, areTexturesWritten);							\

#define ObjectSpriteContainer_ATTRIBUTES																\
		Sprite_ATTRIBUTES																				\
		/**
		 * @var VirtualList	objectSprites
		 * @brief			object sprites
		 * @memberof		ObjectSpriteContainer
		 */																								\
		VirtualList objectSprites;																		\
		/**
		 * @var VirtualNode	node
		 * @brief			for z sorting
		 * @memberof		ObjectSpriteContainer
		 */																								\
		VirtualNode node;																				\
		VirtualNode previousNode;																		\
		/**
		 * @var VirtualNode	objectSpriteNodeToDefragment
		 * @brief			next object sprite node to defragment
		 * @memberof		ObjectSpriteContainer
		 */																								\
		VirtualNode objectSpriteNodeToDefragment;														\
		/**
		 * @var fix19_13	z
		 * @brief			for WORLD sorting
		 * @memberof		ObjectSpriteContainer
		 */																								\
		fix19_13 z;																						\
		/**
		 * @var int			freedObjectIndex
		 * @brief			used for defragmentation
		 * @memberof		ObjectSpriteContainer
		 */																								\
		int freedObjectIndex;																			\
		/**
		 * @var int			firstObjectIndex
		 * @brief			first object index
		 * @memberof		ObjectSpriteContainer
		 */																								\
		int firstObjectIndex;																			\
		/**
		 * @var int			totalObjects
		 * @brief			total objects
		 * @memberof		ObjectSpriteContainer
		 */																								\
		int totalObjects;																				\
		/**
		 * @var int			availableObjects
		 * @brief			OBJs available
		 * @memberof		ObjectSpriteContainer
		 */																								\
		int availableObjects;																			\
		/**
		 * @var int			spt
		 * @brief			spt index
		 * @memberof		ObjectSpriteContainer
		 */																								\
		int spt;																						\
		/**
		 * @var bool		removingObjectSprite
		 * @brief			flag to halt defragmentation while sprite removal is taking place
		 * @memberof		ObjectSpriteContainer
		 */																								\
		bool removingObjectSprite;																		\

__CLASS(ObjectSpriteContainer);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, int spt, int totalObjects, int firstObjectIndex);
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this);

void ObjectSpriteContainer_addDisplacement(ObjectSpriteContainer this, const VBVec2D* displacement);
s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects);
void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z);
int ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this);
int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this);
VBVec2D ObjectSpriteContainer_getPosition(ObjectSpriteContainer this);
int ObjectSpriteContainer_getNextFreeObjectIndex(ObjectSpriteContainer this);
int ObjectSpriteContainer_getTotalUsedObjects(ObjectSpriteContainer this);
bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, s32 numberOfObjects);
void ObjectSpriteContainer_hide(ObjectSpriteContainer this);
void ObjectSpriteContainer_position(ObjectSpriteContainer this, const VBVec3D* position);
void ObjectSpriteContainer_print(ObjectSpriteContainer this, int x, int y);
void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s32 numberOfObjects);
void ObjectSpriteContainer_render(ObjectSpriteContainer this);
void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, const VBVec2D* position);
void ObjectSpriteContainer_show(ObjectSpriteContainer this);
void ObjectSpriteContainer_setMode(ObjectSpriteContainer this, u16 display, u16 mode);
bool ObjectSpriteContainer_writeTextures(ObjectSpriteContainer this);
bool ObjectSpriteContainer_areTexturesWritten(ObjectSpriteContainer this);


#endif
