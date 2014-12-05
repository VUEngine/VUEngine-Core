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


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <stdarg.h>
#include <Object.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// the root class for everything else!!

// declare the virtual methods
#define Container_METHODS								\
		Object_METHODS									\
		__VIRTUAL_DEC(update);							\
		__VIRTUAL_DEC(transform);						\
		__VIRTUAL_DEC(initialTransform);				\
		__VIRTUAL_DEC(setLocalPosition);				\
		__VIRTUAL_DEC(doMessage);						\
		__VIRTUAL_DEC(addChild);						\
		
	

// define the virtual methods
#define Container_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Container, update);				\
		__VIRTUAL_SET(ClassName, Container, transform);				\
		__VIRTUAL_SET(ClassName, Container, initialTransform);		\
		__VIRTUAL_SET(ClassName, Container, setLocalPosition);		\
		__VIRTUAL_SET(ClassName, Container, doMessage);				\
		__VIRTUAL_SET(ClassName, Container, addChild);				\
	

#define Container_ATTRIBUTES							\
														\
	/* super's attributes */							\
	Object_ATTRIBUTES;									\
														\
	/* children list */									\
	VirtualList children;								\
														\
	/* removed children list */							\
	VirtualList removedChildren;						\
														\
	/* parent */										\
	Container parent;									\
														\
	/* entity's id */									\
	u16 ID;												\
														\
	/* 3d transformation */								\
	Transformation transform;							\
														\
	/* flag to recalculate global position */			\
	VBVec3D invalidateGlobalPosition;

__CLASS(Container);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's allocator
__CLASS_NEW_DECLARE(Container, __PARAMETERS(s16 ID));

// class's constructor
void Container_constructor(Container this, s16 ID);

// class's destructor
void Container_destructor(Container this);

// add a child Container
void Container_addChild(Container this, Container child);

// remove child Container
void Container_removeChild(Container this, Container child);

// update each Container's child
void Container_update(Container this);

// propagate an event to the children wrapper
int Container_propagateEvent(Container this, int (*event)(Container this, va_list args), ...);

// retrieve environment transformation
Transformation Container_getEnvironmentTransform(Container this);

// contatenate transform
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transform);
	
// transform
void Container_transform(Container this, Transformation* environmentTransform);

// intinial transform
void Container_initialTransform(Container this, Transformation* environmentTransform);

// retrieve global position
VBVec3D Container_getGlobalPosition(Container this);

// retrieve local position
VBVec3D Container_getLocalPosition(Container this);

//set class's local position
void Container_setLocalPosition(Container this, VBVec3D position);

// process user input
int Container_onMessage(Container this, va_list args);

// process user input
int Container_doKeyPressed(Container this, int pressedKey);

// process user input
int Container_doKeyUp(Container this, int pressedKey);

// process user input
int Container_doKeyHold(Container this, int pressedKey);

// process message
int Container_doMessage(Container this, int message);

//retrieve object's in game index
s16 Container_getID(Container this);

// retrieve child count
int Container_getChildCount(Container this);

// retrieve children
VirtualList Container_getChildren(Container this);

#endif /* CONTAINER_H_ */
