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

/*
// position
       1 X = 01
      10 Y = 02
     100 Z = 04
// rotation
    1000 X = 08
   10000 Y = 10
  100000 Z = 20
//scale
 1000000 X = 40
10000000 Y = 80
*/

#define __INVALIDATE_TRANSFORMATION			0xFF
#define __INVALIDATE_POSITION			    0x07
#define __INVALIDATE_ROTATION			    0x38
#define __INVALIDATE_SCALE			        0xC0


#define __MAX_CONTAINER_NAME_LENGTH			8


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Container_METHODS(ClassName)																	\
		SpatialObject_METHODS(ClassName)																\
		__VIRTUAL_DEC(ClassName, void, update, u32);													\
		__VIRTUAL_DEC(ClassName, void, transform, const Transformation*);			                    \
		__VIRTUAL_DEC(ClassName, void, updateVisualRepresentation);										\
		__VIRTUAL_DEC(ClassName, void, initialTransform, const Transformation*, u32);	                \
		__VIRTUAL_DEC(ClassName, void, setLocalPosition, const VBVec3D* position);						\
		__VIRTUAL_DEC(ClassName, bool, handlePropagatedMessage, int message);							\
		__VIRTUAL_DEC(ClassName, void, addChild, Container child);										\
		__VIRTUAL_DEC(ClassName, void, removeChild, Container child);									\
		__VIRTUAL_DEC(ClassName, void, changeEnvironment, Transformation* environmentTransform);		\
		__VIRTUAL_DEC(ClassName, void, suspend);														\
		__VIRTUAL_DEC(ClassName, void, resume);															\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, int, passMessage, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);\


// define the virtual methods
#define Container_SET_VTABLE(ClassName)																	\
		SpatialObject_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, Container, update);													\
		__VIRTUAL_SET(ClassName, Container, transform);													\
		__VIRTUAL_SET(ClassName, Container, updateVisualRepresentation);								\
		__VIRTUAL_SET(ClassName, Container, initialTransform);											\
		__VIRTUAL_SET(ClassName, Container, setLocalPosition);											\
		__VIRTUAL_SET(ClassName, Container, handlePropagatedMessage);									\
		__VIRTUAL_SET(ClassName, Container, addChild);													\
		__VIRTUAL_SET(ClassName, Container, changeEnvironment);											\
		__VIRTUAL_SET(ClassName, Container, removeChild);												\
		__VIRTUAL_SET(ClassName, Container, suspend);													\
		__VIRTUAL_SET(ClassName, Container, resume);													\
		__VIRTUAL_SET(ClassName, Container, show);														\
		__VIRTUAL_SET(ClassName, Container, hide);														\
		__VIRTUAL_SET(ClassName, Container, passMessage);												\

#define Container_ATTRIBUTES																			\
        /* super's attributes */																		\
        SpatialObject_ATTRIBUTES																		\
        /* 3d transformation */																			\
        Transformation transform;																		\
        /* children list */																				\
        VirtualList children;																			\
        /* removed children list */																		\
        VirtualList removedChildren;																	\
        /* parent */																					\
        Container parent;																				\
        /* name */																						\
        char* name;																						\
        /* flag for parent to know to delete it */														\
        u8 deleteMe;																				    \
        /* flag to hide the entity */																	\
        u8 hidden;                                                                                      \
        /* flag to recalculate global transformations */												\
        u8 invalidateGlobalTransformation;														        \

__CLASS(Container);


//---------------------------------------------------------------------------------------------------------
// 										MISC
//---------------------------------------------------------------------------------------------------------

// needed because of interdependency between Shape's and SpatialObject's headers
Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Container, const char* const name);

void Container_constructor(Container this, const char* const name);
void Container_destructor(Container this);
void Container_deleteMyself(Container this);
void Container_addChild(Container this, Container child);
void Container_removeChild(Container this, Container child);
void Container_processRemovedChildren(Container this);
void Container_update(Container this, u32 elapsedTime);
int Container_propagateMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), ...);
Transformation Container_getEnvironmentTransform(Container this);
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transform);
void Container_changeEnvironment(Container this, Transformation* environmentTransform);
void Container_transformNonVirtual(Container this, const Transformation* environmentTransform);
void Container_transform(Container this, const Transformation* environmentTransform);
void Container_updateVisualRepresentation(Container this);
void Container_initialTransform(Container this, Transformation* environmentTransform, u32 recursive);
void Container_applyEnvironmentToTransformation(Container this, const Transformation* environmentTransform);
const VBVec3D* Container_getGlobalPosition(Container this);
const VBVec3D* Container_getLocalPosition(Container this);
void Container_setLocalPosition(Container this, const VBVec3D* position);
const Rotation* Container_getLocalRotation(Container this);
void Container_setLocalRotation(Container this, const Rotation* rotation);
const Scale* Container_getLocalScale(Container this);
void Container_setLocalScale(Container this, const Scale* scale);
void Container_invalidateGlobalTransformation(Container this);
void Container_invalidateGlobalPosition(Container this, u8 axisToInvalidate);
void Container_invalidateGlobalRotation(Container this, u8 axisToInvalidate);
void Container_invalidateGlobalScale(Container this, u8 axisToInvalidate);
int Container_onPropagatedMessage(Container this, va_list args);
int Container_doKeyPressed(Container this, int pressedKey);
int Container_doKeyUp(Container this, int pressedKey);
int Container_doKeyHold(Container this, int pressedKey);
bool Container_handlePropagatedMessage(Container this, int message);
s16 Container_getId(Container this);
Container Container_getParent(Container this);
int Container_getChildCount(Container this);
VirtualList Container_getChildren(Container this);
void Container_setName(Container this, const char* const  name);
char* Container_getName(Container this);
Container Container_getChildByName(Container this, char* childName, bool recursive);
void Container_suspend(Container this);
void Container_resume(Container this);
void Container_show(Container this);
void Container_hide(Container this);
bool Container_isHidden(Container this);
int Container_passMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);

#endif
