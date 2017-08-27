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

#ifndef CUBOID_H_
#define CUBOID_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Polyhedron.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Cuboid_METHODS(ClassName)																		\
		Shape_METHODS(ClassName)																		\

#define Cuboid_SET_VTABLE(ClassName)																	\
		Shape_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, Cuboid, overlaps);														\
		__VIRTUAL_SET(ClassName, Cuboid, setup);														\
		__VIRTUAL_SET(ClassName, Cuboid, position);														\
		__VIRTUAL_SET(ClassName, Cuboid, getAxisOfCollision);											\
		__VIRTUAL_SET(ClassName, Cuboid, testIfCollision);												\
		__VIRTUAL_SET(ClassName, Cuboid, getPosition);													\
		__VIRTUAL_SET(ClassName, Cuboid, getSurroundingRightCuboid);									\
		__VIRTUAL_SET(ClassName, Cuboid, hide);															\
		__VIRTUAL_SET(ClassName, Cuboid, show);															\
		__VIRTUAL_SET(ClassName, Cuboid, print);														\

#define Cuboid_ATTRIBUTES																				\
		Shape_ATTRIBUTES																				\
		/**
		 * @var RightCuboid	rightCuboid
		 * @brief			the rectangle
		 * @memberof 		Cuboid
		 */																								\
		RightCuboid rightCuboid;																		\
		/**
		 * @var RightCuboid	positionedRightCuboid
		 * @brief			the rightCuboid to check
		 * @memberof 		Cuboid
		 */																								\
		RightCuboid positionedRightCuboid;																\
		/**
		 * @var Polyhedron	polyhedron
		 * @brief			for debugging purposes
		 * @memberof 		Cuboid
		 */																								\
		Polyhedron polyhedron;																			\

__CLASS(Cuboid);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Cuboid, SpatialObject owner);

void Cuboid_constructor(Cuboid this, SpatialObject owner);
void Cuboid_destructor(Cuboid this);

u16 Cuboid_getAxisOfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement, VBVec3D previousPosition);
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this);
RightCuboid Cuboid_getRightCuboid(Cuboid this);
bool Cuboid_overlaps(Cuboid this, Shape shape);
void Cuboid_position(Cuboid this, const VBVec3D* myOwnerPosition, bool isAffectedByRelativity);
void Cuboid_setup(Cuboid this, const VBVec3D* ownerPosition, const Size* size, const VBVec3D* displacement, bool moves);
bool Cuboid_testIfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement);
VBVec3D Cuboid_getPosition(Cuboid this);
RightCuboid Cuboid_getSurroundingRightCuboid(Cuboid this);
void Cuboid_show(Cuboid this);
void Cuboid_hide(Cuboid this);
void Cuboid_print(Cuboid this, int x, int y);


#endif
