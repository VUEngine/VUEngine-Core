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
        Object_ATTRIBUTES;																				\

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
