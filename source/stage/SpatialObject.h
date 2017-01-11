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

#ifndef SPATIAL_OBJECT_H_
#define SPATIAL_OBJECT_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SpatialObject_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void*, getShape);														\
		__VIRTUAL_DEC(ClassName, int, getShapeType);													\
		__VIRTUAL_DEC(ClassName, bool, moves);															\
		__VIRTUAL_DEC(ClassName, int, canMoveOverAxis, const Acceleration* acceleration);				\
		__VIRTUAL_DEC(ClassName, int, getWidth);														\
		__VIRTUAL_DEC(ClassName, int, getHeight);														\
		__VIRTUAL_DEC(ClassName, int, getDepth);														\
		__VIRTUAL_DEC(ClassName, Gap, getGap);															\
		__VIRTUAL_DEC(ClassName, const VBVec3D*, getPosition);											\
		__VIRTUAL_DEC(ClassName, void, setPosition, const VBVec3D* position);							\
		__VIRTUAL_DEC(ClassName, fix19_13, getElasticity);												\
		__VIRTUAL_DEC(ClassName, fix19_13, getFriction);												\
		__VIRTUAL_DEC(ClassName, Velocity, getVelocity);												\

// define the virtual methods
#define SpatialObject_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, SpatialObject, getShape);												\
		__VIRTUAL_SET(ClassName, SpatialObject, getShapeType);											\
		__VIRTUAL_SET(ClassName, SpatialObject, moves);													\
		__VIRTUAL_SET(ClassName, SpatialObject, canMoveOverAxis);										\
		__VIRTUAL_SET(ClassName, SpatialObject, getWidth);												\
		__VIRTUAL_SET(ClassName, SpatialObject, getHeight);												\
		__VIRTUAL_SET(ClassName, SpatialObject, getDepth);												\
		__VIRTUAL_SET(ClassName, SpatialObject, getGap);												\
		__VIRTUAL_SET(ClassName, SpatialObject, getPosition);											\
		__VIRTUAL_SET(ClassName, SpatialObject, setPosition);											\
		__VIRTUAL_SET(ClassName, SpatialObject, getElasticity);											\
		__VIRTUAL_SET(ClassName, SpatialObject, getFriction);											\
		__VIRTUAL_SET(ClassName, SpatialObject, getVelocity);											\


#define SpatialObject_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\

__CLASS(SpatialObject);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(SpatialObject);

void SpatialObject_constructor(SpatialObject this);
void SpatialObject_destructor(SpatialObject this);
int SpatialObject_getShapeType(SpatialObject this);
bool SpatialObject_moves(SpatialObject this);
int SpatialObject_canMoveOverAxis(SpatialObject this, const Acceleration* acceleration);
int SpatialObject_getWidth(SpatialObject this);
int SpatialObject_getHeight(SpatialObject this);
int SpatialObject_getDepth(SpatialObject this);
Gap SpatialObject_getGap(SpatialObject this);
const VBVec3D* SpatialObject_getPosition(SpatialObject this);
void SpatialObject_setPosition(SpatialObject this, const VBVec3D* position);
fix19_13 SpatialObject_getElasticity(SpatialObject this);
fix19_13 SpatialObject_getFriction(SpatialObject this);
Velocity SpatialObject_getVelocity(SpatialObject this);


#endif
