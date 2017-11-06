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
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __CUBOID_NORMALS	3
#define __CUBOID_VERTEXES	8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Cuboid_METHODS(ClassName)																		\
		Shape_METHODS(ClassName)																		\

#define Cuboid_SET_VTABLE(ClassName)																	\
		Shape_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, Cuboid, overlaps);														\
		__VIRTUAL_SET(ClassName, Cuboid, computeMinimumTranslationVector);								\
		__VIRTUAL_SET(ClassName, Cuboid, setup);														\
		__VIRTUAL_SET(ClassName, Cuboid, testIfCollision);												\
		__VIRTUAL_SET(ClassName, Cuboid, getPosition);													\
		__VIRTUAL_SET(ClassName, Cuboid, getSurroundingRightCuboid);									\
		__VIRTUAL_SET(ClassName, Cuboid, getPositionedSurroundingRightCuboid);							\
		__VIRTUAL_SET(ClassName, Cuboid, hide);															\
		__VIRTUAL_SET(ClassName, Cuboid, show);															\
		__VIRTUAL_SET(ClassName, Cuboid, print);														\

#define Cuboid_ATTRIBUTES																				\
		Shape_ATTRIBUTES																				\
		/**
		 * @var Normals		normals
		 * @brief			the normals of the cuboid
		 * @memberof 		Cuboid
		 */																								\
		Normals* normals;																				\
		/**
		 * @var Polyhedron	polyhedron
		 * @brief			for debugging purposes
		 * @memberof 		Cuboid
		 */																								\
		Polyhedron polyhedron;																			\
		/**
		 * @var VBVec3D		rotationVertexDisplacement
		 * @brief			for rotation purposes
		 * @memberof 		Cuboid
		 */																								\
		VBVec3D rotationVertexDisplacement;																\
		/**
		 * @var RightCuboid	rightCuboid
		 * @brief			the rectangle
		 * @memberof 		Cuboid
		 */																								\
		RightCuboid rightCuboid;																		\

__CLASS(Cuboid);


typedef struct Normals
{
	VBVec3D vectors[__CUBOID_NORMALS];
} Normals;

//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Cuboid, SpatialObject owner);

void Cuboid_constructor(Cuboid this, SpatialObject owner);
void Cuboid_destructor(Cuboid this);

RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this);
RightCuboid Cuboid_getRightCuboid(Cuboid this);
CollisionInformation Cuboid_overlaps(Cuboid this, Shape shape);
VBVec3D Cuboid_computeMinimumTranslationVector(Cuboid this, Shape collidingShape);
void Cuboid_setup(Cuboid this, const VBVec3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
bool Cuboid_testIfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement);
VBVec3D Cuboid_getPosition(Cuboid this);
RightCuboid Cuboid_getSurroundingRightCuboid(Cuboid this);
RightCuboid Cuboid_getPositionedSurroundingRightCuboid(Cuboid this);
void Cuboid_show(Cuboid this);
void Cuboid_hide(Cuboid this);
void Cuboid_print(Cuboid this, int x, int y);


#endif
