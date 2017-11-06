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

#ifndef OBJECT_SPRITE_CONTAINER_MANAGER_H_
#define OBJECT_SPRITE_CONTAINER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSpriteContainer.h>
#include <ObjectTexture.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __TOTAL_OBJECT_SEGMENTS 	4


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSpriteContainerManager_METHODS(ClassName)													\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define ObjectSpriteContainerManager_SET_VTABLE(ClassName)												\
		Object_SET_VTABLE(ClassName)																	\


// declare a ObjectSpriteContainerManager, which holds a texture and a drawing specification
__CLASS(ObjectSpriteContainerManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ObjectSpriteContainerManager ObjectSpriteContainerManager_getInstance();
void ObjectSpriteContainerManager_destructor(ObjectSpriteContainerManager this);

ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager this, int numberOfObjects, fix19_13 z);
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainerBySegment(ObjectSpriteContainerManager this, int segment);
void ObjectSpriteContainerManager_print(ObjectSpriteContainerManager this, int x, int y);
void ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager this);
void ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager this, s16 size[__TOTAL_OBJECT_SEGMENTS], fix19_13 z[__TOTAL_OBJECT_SEGMENTS]);
void ObjectSpriteContainerManager_setZPosition(ObjectSpriteContainerManager this, int spt, fix19_13 z);

#endif
