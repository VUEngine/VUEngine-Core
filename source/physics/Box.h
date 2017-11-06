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

#ifndef BOX_H_
#define BOX_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Polyhedron.h>

//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BOX_VERTEXES	8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Box_METHODS(ClassName)																			\
		Shape_METHODS(ClassName)																		\

#define Box_SET_VTABLE(ClassName)																		\
		Shape_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, Box, overlaps);														\
		__VIRTUAL_SET(ClassName, Box, getMinimumOverlappingVector);										\
		__VIRTUAL_SET(ClassName, Box, setup);															\
		__VIRTUAL_SET(ClassName, Box, testIfCollision);													\
		__VIRTUAL_SET(ClassName, Box, getPosition);														\
		__VIRTUAL_SET(ClassName, Box, getSurroundingRightBox);											\
		__VIRTUAL_SET(ClassName, Box, hide);															\
		__VIRTUAL_SET(ClassName, Box, show);															\
		__VIRTUAL_SET(ClassName, Box, print);															\

#define Box_ATTRIBUTES																					\
		Shape_ATTRIBUTES																				\
		/**
		 * @var Normals		normals
		 * @brief			the normals of the box
		 * @memberof 		Box
		 */																								\
		Normals* normals;																				\
		/**
		 * @var Polyhedron	polyhedron
		 * @brief			for debugging purposes
		 * @memberof 		Box
		 */																								\
		Polyhedron polyhedron;																			\
		/**
		 * @var VBVec3D		rotationVertexDisplacement
		 * @brief			for rotation purposes
		 * @memberof 		Box
		 */																								\
		VBVec3D rotationVertexDisplacement;																\
		/**
		 * @var RightBox	rightBox
		 * @brief			the rectangle
		 * @memberof 		Box
		 */																								\
		RightBox rightBox;																				\

__CLASS(Box);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Box, SpatialObject owner);

void Box_constructor(Box this, SpatialObject owner);
void Box_destructor(Box this);

void Box_setup(Box this, const VBVec3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
CollisionInformation Box_overlaps(Box this, Shape shape);
VBVec3D Box_getMinimumOverlappingVector(Box this, Shape shape);
bool Box_testIfCollision(Box this, Shape collidingShape, VBVec3D displacement);
VBVec3D Box_getPosition(Box this);
RightBox Box_getSurroundingRightBox(Box this);
void Box_show(Box this);
void Box_hide(Box this);
void Box_print(Box this, int x, int y);


#endif
