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

#ifndef SHAPE_H_
#define SHAPE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Shape_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, int, overlaps, Shape shape);											\
		__VIRTUAL_DEC(ClassName, void, setup);															\
		__VIRTUAL_DEC(ClassName, void, position);														\
		__VIRTUAL_DEC(ClassName, int, getAxisOfCollision, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition);												\
		__VIRTUAL_DEC(ClassName, int, testIfCollision, SpatialObject collidingSpatialObject, VBVec3D displacement);																	\
		__VIRTUAL_DEC(ClassName, void, deleteDirectDrawData);											\
		__VIRTUAL_DEC(ClassName, void, draw);															\
		__VIRTUAL_DEC(ClassName, void, print, int x, int y);                                            \

#define Shape_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\

#define Shape_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* the entity to which the shape belongs */														\
        SpatialObject owner;																			\
        /* flag to know if the shapes below to an entity which moves */									\
        u8 moves;																				        \
        /* whether it has been checked for collision in current update */								\
        u8 checked;																				        \
        /* flag to know if setup is needed */															\
        u8 ready;																				        \
        /* flag to check against other shapes */														\
        u8 checkForCollisions;																	        \

__CLASS(Shape);

enum ShapeTypes
{
	kNoShape = 0,
	kCircle,
	kCuboid,
	kInverseCuboid,
};


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Shape_constructor(Shape this, SpatialObject owner);
void Shape_destructor(Shape this);
SpatialObject Shape_getOwner(Shape this);
void Shape_setActive(Shape this, bool active);
bool Shape_isActive(Shape this);
bool Shape_moves(Shape this);
bool Shape_isChecked(Shape this);
void Shape_checked(Shape this, bool checked);
bool Shape_isReady(Shape this);
void Shape_setReady(Shape this, bool ready);
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions);
bool Shape_checkForCollisions(Shape this);
void Shape_draw(Shape this);
void Shape_deleteDirectDrawData(Shape this);
void Shape_print(Shape this, int x, int y);


#endif
