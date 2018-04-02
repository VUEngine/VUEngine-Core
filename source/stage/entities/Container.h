/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef CONTAINER_H_
#define CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S MACROS
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

#define __INVALIDATE_TRANSFORMATION			0x0F
#define __INVALIDATE_POSITION				0x01
#define __INVALIDATE_ROTATION				0x02
#define __INVALIDATE_SCALE					0x04
#define __INVALIDATE_PROJECTION				0x08


#define __MAX_CONTAINER_NAME_LENGTH			8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Container_METHODS(ClassName)																	\
		SpatialObject_METHODS(ClassName)																\
		__VIRTUAL_DEC(ClassName, void, iAmDeletingMyself);												\
		__VIRTUAL_DEC(ClassName, void, update, u32);													\
		__VIRTUAL_DEC(ClassName, void, setupGraphics);													\
		__VIRTUAL_DEC(ClassName, void, releaseGraphics);												\
		__VIRTUAL_DEC(ClassName, void, transform, const Transformation*, u8);							\
		__VIRTUAL_DEC(ClassName, void, synchronizeGraphics);											\
		__VIRTUAL_DEC(ClassName, void, initialTransform, const Transformation*, u32);					\
		__VIRTUAL_DEC(ClassName, void, setLocalPosition, const Vector3D* position);						\
		__VIRTUAL_DEC(ClassName, void, setLocalRotation, const Rotation* rotation);						\
		__VIRTUAL_DEC(ClassName, bool, handlePropagatedMessage, int message);							\
		__VIRTUAL_DEC(ClassName, void, addChild, Container child);										\
		__VIRTUAL_DEC(ClassName, void, removeChild, Container child, bool deleteChild);					\
		__VIRTUAL_DEC(ClassName, void, changeEnvironment, Transformation* environmentTransform);		\
		__VIRTUAL_DEC(ClassName, void, suspend);														\
		__VIRTUAL_DEC(ClassName, void, resume);															\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, int, passMessage, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);\


// define the virtual methods
#define Container_SET_VTABLE(ClassName)																	\
		SpatialObject_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, Container, iAmDeletingMyself);											\
		__VIRTUAL_SET(ClassName, Container, update);													\
		__VIRTUAL_SET(ClassName, Container, setupGraphics);												\
		__VIRTUAL_SET(ClassName, Container, releaseGraphics);											\
		__VIRTUAL_SET(ClassName, Container, transform);													\
		__VIRTUAL_SET(ClassName, Container, synchronizeGraphics);										\
		__VIRTUAL_SET(ClassName, Container, initialTransform);											\
		__VIRTUAL_SET(ClassName, Container, setLocalPosition);											\
		__VIRTUAL_SET(ClassName, Container, setLocalRotation);											\
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
		Transformation transformation;																		\
		/* children list */																				\
		VirtualList children;																			\
		/* removed children list */																		\
		VirtualList removedChildren;																	\
		/* parent */																					\
		Container parent;																				\
		/* name */																						\
		char* name;																						\
		/* flag for parent to know to delete it */														\
		u8 deleteMe;																					\
		/* flag to hide the entity */																	\
		u8 hidden;																						\
		/* flag to recalculate global transformations */												\
		u8 invalidateGlobalTransformation;																\

__CLASS(Container);


//---------------------------------------------------------------------------------------------------------
//										MISC
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Container, const char* const name);

void Container_constructor(Container this, const char* const name);
void Container_destructor(Container this);

void Container_addChild(Container this, Container child);
void Container_applyEnvironmentToTransformation(Container this, const Transformation* environmentTransform);
void Container_changeEnvironment(Container this, Transformation* environmentTransform);
void Container_concatenateTransform(Transformation *environmentTransform, Transformation* transformation);
void Container_deleteMyself(Container this);
void Container_iAmDeletingMyself(Container this);
int Container_doKeyHold(Container this, int pressedKey);
int Container_doKeyPressed(Container this, int pressedKey);
int Container_doKeyUp(Container this, int pressedKey);
Container Container_getChildByName(Container this, char* childName, bool recursive);
int Container_getChildCount(Container this);
Transformation Container_getEnvironmentTransform(Container this);
Transformation* Container_getTransform(Container this);
const Vector3D* Container_getGlobalPosition(Container this);
s16 Container_getId(Container this);
const Vector3D* Container_getLocalPosition(Container this);
const Rotation* Container_getLocalRotation(Container this);
const Scale* Container_getLocalScale(Container this);
char* Container_getName(Container this);
Container Container_getParent(Container this);
bool Container_handlePropagatedMessage(Container this, int message);
void Container_hide(Container this);
void Container_initialTransform(Container this, Transformation* environmentTransform, u32 recursive);
void Container_invalidateGlobalPosition(Container this);
void Container_invalidateGlobalRotation(Container this);
void Container_invalidateGlobalScale(Container this);
void Container_invalidateGlobalTransformation(Container this);
bool Container_isHidden(Container this);
int Container_passMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args);
int Container_onPropagatedMessage(Container this, va_list args);
void Container_purgeChildren(Container this);
int Container_propagateMessage(Container this, int (*propagatedMessageHandler)(Container this, va_list args), ...);
void Container_setupGraphics(Container this);
void Container_releaseGraphics(Container this);
void Container_removeChild(Container this, Container child, bool deleteChild);
void Container_resume(Container this);
void Container_setLocalPosition(Container this, const Vector3D* position);
void Container_setLocalRotation(Container this, const Rotation* rotation);
void Container_setLocalScale(Container this, const Scale* scale);
void Container_setName(Container this, const char* const name);
void Container_show(Container this);
void Container_suspend(Container this);
void Container_transform(Container this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void Container_transformNonVirtual(Container this, const Transformation* environmentTransform);
void Container_update(Container this, u32 elapsedTime);
void Container_synchronizeGraphics(Container this);


#endif
