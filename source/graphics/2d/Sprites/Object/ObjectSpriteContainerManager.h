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

#ifndef OBJECT_SPRITE_CONTAINER_MANAGER_H_
#define OBJECT_SPRITE_CONTAINER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSpriteContainer.h>
#include <ObjectTexture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __TOTAL_OBJECT_SEGMENTS 	4


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ObjectSpriteContainerManager_METHODS(ClassName)									\
	Object_METHODS(ClassName)																\

// declare the virtual methods which are redefined
#define ObjectSpriteContainerManager_SET_VTABLE(ClassName)						\
	Object_SET_VTABLE(ClassName)												\


// declare a ObjectSpriteContainerManager, which holds a texture and a drawing specification
__CLASS(ObjectSpriteContainerManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ObjectSpriteContainerManager ObjectSpriteContainerManager_getInstance();

void ObjectSpriteContainerManager_destructor(ObjectSpriteContainerManager this);
void ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager this);
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager this, int numberOfObjects, fix19_13 z);
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainerBySegment(ObjectSpriteContainerManager this, int segment);
void ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager this, fix19_13 size[__TOTAL_OBJECT_SEGMENTS], fix19_13 z[__TOTAL_OBJECT_SEGMENTS]);
void ObjectSpriteContainerManager_setZPosition(ObjectSpriteContainerManager this, int spt, fix19_13 z);
void ObjectSpriteContainerManager_print(ObjectSpriteContainerManager this, int x, int y);


#endif
