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

#define Cuboid_METHODS															\
		Shape_METHODS															\

#define Cuboid_SET_VTABLE(ClassName)											\
		Shape_SET_VTABLE(ClassName)												\
		__VIRTUAL_SET(ClassName, Cuboid, draw);									\
		__VIRTUAL_SET(ClassName, Cuboid, overlaps);								\
		__VIRTUAL_SET(ClassName, Cuboid, setup);								\
		__VIRTUAL_SET(ClassName, Cuboid, positione);							\
		__VIRTUAL_SET(ClassName, Cuboid, getAxisOfCollision);					\
		__VIRTUAL_SET(ClassName, Cuboid, testIfCollision);						\
		__VIRTUAL_SET(ClassName, Cuboid, deleteDirectDrawData);					\
		__VIRTUAL_SET(ClassName, Cuboid, draw);									\
		__VIRTUAL_SET(ClassName, Cuboid, print);								\

typedef struct Rightcuboid
{
	/* left upper corner */
	fix19_13 x0;
	fix19_13 y0;
	fix19_13 z0;

	/* right down corner */
	fix19_13 x1;
	fix19_13 y1;
	fix19_13 z1;

} Rightcuboid;

#define Cuboid_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Shape_ATTRIBUTES;															\
																				\
	/* the rectangle */															\
	Rightcuboid rightCuboid;													\
																				\
	/* the rightCuboid to check */												\
	Rightcuboid positionedRightcuboid;											\
																				\
	/* for debugging purposes */												\
	Polygon polygon;															\

// A Cuboid which represent a generic object inside a Stage
__CLASS(Cuboid);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Cuboid, Entity owner);

void Cuboid_destructor(Cuboid this);
bool Cuboid_overlaps(Cuboid this, Shape shape);
void Cuboid_setup(Cuboid this);
void Cuboid_positione(Cuboid this);
Rightcuboid Cuboid_getRightcuboid(Cuboid this);
Rightcuboid Cuboid_getPositionedRightcuboid(Cuboid this);
int Cuboid_getAxisOfCollision(Cuboid this, Entity collidingEntity, VBVec3D displacement);
int Cuboid_testIfCollision(Cuboid this, Entity collidingEntity, VBVec3D displacement);
void Cuboid_draw(Cuboid this);
void Cuboid_deleteDirectDrawData(Cuboid this);
void Cuboid_print(Cuboid this, int x, int y);


#endif