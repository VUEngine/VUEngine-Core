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

#ifndef CUBOID_H_
#define CUBOID_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Polygon.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Cuboid_METHODS																					\
		Shape_METHODS																					\

#define Cuboid_SET_VTABLE(ClassName)																	\
		Shape_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, Cuboid, draw);															\
		__VIRTUAL_SET(ClassName, Cuboid, overlaps);														\
		__VIRTUAL_SET(ClassName, Cuboid, setup);														\
		__VIRTUAL_SET(ClassName, Cuboid, position);														\
		__VIRTUAL_SET(ClassName, Cuboid, getAxisOfCollision);											\
		__VIRTUAL_SET(ClassName, Cuboid, testIfCollision);												\
		__VIRTUAL_SET(ClassName, Cuboid, deleteDirectDrawData);											\
		__VIRTUAL_SET(ClassName, Cuboid, draw);															\
		__VIRTUAL_SET(ClassName, Cuboid, print);														\

#define Cuboid_ATTRIBUTES																				\
																										\
	/* super's attributes */																			\
	Shape_ATTRIBUTES;																					\
																										\
	/* the rectangle */																					\
	RightCuboid rightCuboid;																			\
																										\
	/* the rightCuboid to check */																		\
	RightCuboid positionedRightCuboid;																	\
																										\
	/* for debugging purposes */																		\
	Polygon polygon;																					\

__CLASS(Cuboid);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Cuboid, SpatialObject owner);

void Cuboid_constructor(Cuboid this, SpatialObject owner);
void Cuboid_destructor(Cuboid this);
u8 Cuboid_overlaps(Cuboid this, Shape shape);
void Cuboid_setup(Cuboid this);
void Cuboid_position(Cuboid this);
RightCuboid Cuboid_getRightCuboid(Cuboid this);
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this);
u8 Cuboid_getAxisOfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition);
u8 Cuboid_testIfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition);
void Cuboid_draw(Cuboid this);
void Cuboid_deleteDirectDrawData(Cuboid this);
void Cuboid_print(Cuboid this, int x, int y);


#endif