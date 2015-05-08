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
#include <SpatialObject.h>
#include <Shape.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_CONTAINER_NAME_LENGTH	16


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Container_METHODS														\
		SpatialObject_METHODS													\
		__VIRTUAL_DEC(update);													\
		__VIRTUAL_DEC(transform);												\
		__VIRTUAL_DEC(initialTransform);										\
		__VIRTUAL_DEC(setLocalPosition);										\
		__VIRTUAL_DEC(doMessage);												\
		__VIRTUAL_DEC(addChild);												\
		__VIRTUAL_DEC(suspend);													\
		__VIRTUAL_DEC(resume);													\

// define the virtual methods
#define Container_SET_VTABLE(ClassName)											\
		SpatialObject_SET_VTABLE(ClassName)										\
		__VIRTUAL_SET(ClassName, Container, update);							\
		__VIRTUAL_SET(ClassName, Container, transform);							\
		__VIRTUAL_SET(ClassName, Container, initialTransform);					\
		__VIRTUAL_SET(ClassName, Container, setLocalPosition);					\
		__VIRTUAL_SET(ClassName, Container, doMessage);							\
		__VIRTUAL_SET(ClassName, Container, addChild);							\
		__VIRTUAL_SET(ClassName, Container, suspend);							\
		__VIRTUAL_SET(ClassName, Container, resume);							\

#define Container_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	SpatialObject_ATTRIBUTES;													\
																				\
	/* 3d transformation */														\
	Transformation transform;													\
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
	/* name */																	\
	char* name;																	\
																				\
	/* entity's id */															\
	s16 id;																		\
																				\
	/* flag for parent to know to delete it */									\
	u8 deleteMe;																\
																				\
	/* flag to recalculate global position */									\
	VBVec3DFlag invalidateGlobalPosition;										\
																				\

__CLASS(Container);


//---------------------------------------------------------------------------------------------------------
// 										MISC
//---------------------------------------------------------------------------------------------------------

// needed because of interdependency between Shape's and SpatialObject's headers
Shape SpatialObject_getShape(SpatialObject this);

//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Container, s16 id);

void Container_constructor(Container this, s16 id);
void Container_destructor(Container this);
void Container_deleteMyself(Container this);
void Container_addChild(Container this, Container child);
void Container_removeChild(Container this, Container child);
void Container_update(Container this);
int Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...);
Transformation Container_getEnvironmentTransform(Container this);
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transform);
void Container_transformNonVirtual(Container this, const Transformation* environmentTransform);
void Container_transform(Container this, const Transformation* environmentTransform);
void Container_initialTransform(Container this, Transformation* environmentTransform);
const VBVec3D* Container_getGlobalPosition(Container this);
const VBVec3D* Container_getLocalPosition(Container this);
void Container_setLocalPosition(Container this, const VBVec3D* position);
const Rotation* Container_getLocalRotation(Container this);
void Container_setLocalRotation(Container this, const Rotation* rotation);
const Scale* Container_getLocalScale(Container this);
void Container_setLocalScale(Container this, const Scale* scale);
int Container_onMessage(Container this, va_list args);
int Container_doKeyPressed(Container this, int pressedKey);
int Container_doKeyUp(Container this, int pressedKey);
int Container_doKeyHold(Container this, int pressedKey);
int Container_doMessage(Container this, int message);
s16 Container_getId(Container this);
Container Container_getParent(Container this);
int Container_getChildCount(Container this);
VirtualList Container_getChildren(Container this);
void Container_setName(Container this, char* name);
char* Container_getName(Container this);
Container Container_getChildByName(Container this, char* childName, bool recursive);
Container Container_getChildById(Container this, s16 id);
void Container_suspend(Container this);
void Container_resume(Container this);


#endif