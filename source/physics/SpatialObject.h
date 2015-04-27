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

#ifndef SPATIAL_OBJECT_H_
#define SPATIAL_OBJECT_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <Object.h>
#include <HardwareManager.h>
#include <MiscStructs.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SpatialObject_METHODS													\
		Object_METHODS															\
		__VIRTUAL_DEC(getShape);												\
		__VIRTUAL_DEC(getShapeType);											\
		__VIRTUAL_DEC(moves);													\
		__VIRTUAL_DEC(canMoveOverAxis);											\
		__VIRTUAL_DEC(getWidth);												\
		__VIRTUAL_DEC(getHeight);												\
		__VIRTUAL_DEC(getDepth);													\
		__VIRTUAL_DEC(getGap);													\
		__VIRTUAL_DEC(getPosition);												\
		__VIRTUAL_DEC(getPreviousPosition);										\

// define the virtual methods
#define SpatialObject_SET_VTABLE(ClassName)										\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, SpatialObject, getShape);						\
		__VIRTUAL_SET(ClassName, SpatialObject, getShapeType);					\
		__VIRTUAL_SET(ClassName, SpatialObject, moves);							\
		__VIRTUAL_SET(ClassName, SpatialObject, canMoveOverAxis);				\
		__VIRTUAL_SET(ClassName, SpatialObject, getWidth);						\
		__VIRTUAL_SET(ClassName, SpatialObject, getHeight);						\
		__VIRTUAL_SET(ClassName, SpatialObject, getDepth);						\
		__VIRTUAL_SET(ClassName, SpatialObject, getGap);						\
		__VIRTUAL_SET(ClassName, SpatialObject, getPosition);					\
		__VIRTUAL_SET(ClassName, SpatialObject, getPreviousPosition);			\


#define SpatialObject_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

__CLASS(SpatialObject);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(SpatialObject);

void SpatialObject_constructor(SpatialObject this);
void SpatialObject_destructor(SpatialObject this);
int SpatialObject_getShapeType(SpatialObject this);
bool SpatialObject_moves(SpatialObject this);
bool SpatialObject_canMoveOverAxis(SpatialObject this, const Acceleration* acceleration);
u16 SpatialObject_getWidth(SpatialObject this);
u16 SpatialObject_getHeight(SpatialObject this);
u16 SpatialObject_getDepth(SpatialObject this);
Gap SpatialObject_getGap(SpatialObject this);
const VBVec3D* SpatialObject_getPosition(SpatialObject this);
const VBVec3D* SpatialObject_getPreviousPosition(SpatialObject this);

#endif