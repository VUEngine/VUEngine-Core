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

#ifndef CONTAINER_H_
#define CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <Object.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Container_METHODS														\
		Object_METHODS															\
		__VIRTUAL_DEC(update);													\
		__VIRTUAL_DEC(transform);												\
		__VIRTUAL_DEC(initialTransform);										\
		__VIRTUAL_DEC(setLocalPosition);										\
		__VIRTUAL_DEC(doMessage);												\
		__VIRTUAL_DEC(addChild);												\

// define the virtual methods
#define Container_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Container, update);							\
		__VIRTUAL_SET(ClassName, Container, transform);							\
		__VIRTUAL_SET(ClassName, Container, initialTransform);					\
		__VIRTUAL_SET(ClassName, Container, setLocalPosition);					\
		__VIRTUAL_SET(ClassName, Container, doMessage);							\
		__VIRTUAL_SET(ClassName, Container, addChild);							\

#define Container_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* children list */															\
	VirtualList children;														\
																				\
	/* removed children list */													\
	VirtualList removedChildren;												\
																				\
	/* parent */																\
	Container parent;															\
																				\
	/* entity's id */															\
	u16 id;																		\
																				\
	/* 3d transformation */														\
	Transformation transform;													\
																				\
	/* flag to recalculate global position */									\
	VBVec3D invalidateGlobalPosition;											\

__CLASS(Container);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Container, __PARAMETERS(s16 id));

void Container_constructor(Container this, s16 id);
void Container_destructor(Container this);
void Container_addChild(Container this, Container child);
void Container_removeChild(Container this, Container child);
void Container_update(Container this);
int Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...);
Transformation Container_getEnvironmentTransform(Container this);
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transform);
void Container_transform(Container this, Transformation* environmentTransform);
void Container_initialTransform(Container this, Transformation* environmentTransform);
VBVec3D Container_getGlobalPosition(Container this);
VBVec3D Container_getLocalPosition(Container this);
void Container_setLocalPosition(Container this, VBVec3D position);
int Container_onMessage(Container this, va_list args);
int Container_doKeyPressed(Container this, int pressedKey);
int Container_doKeyUp(Container this, int pressedKey);
int Container_doKeyHold(Container this, int pressedKey);
int Container_doMessage(Container this, int message);
s16 Container_getId(Container this);
int Container_getChildCount(Container this);
VirtualList Container_getChildren(Container this);


#endif