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
#include <Polygon.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Cuboid_METHODS(ClassName)																		\
		Shape_METHODS(ClassName)																		\

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
		/* super's attributes */																		\
		Shape_ATTRIBUTES																				\
		/* the rectangle */																				\
		RightCuboid rightCuboid;																		\
		/* the rightCuboid to check */																	\
		RightCuboid positionedRightCuboid;																\
		/* for debugging purposes */																	\
		Polygon polygon;																				\

__CLASS(Cuboid);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Cuboid, SpatialObject owner);

void Cuboid_constructor(Cuboid this, SpatialObject owner);
void Cuboid_destructor(Cuboid this);
int Cuboid_overlaps(Cuboid this, Shape shape);
void Cuboid_setup(Cuboid this);
void Cuboid_position(Cuboid this);
RightCuboid Cuboid_getRightCuboid(Cuboid this);
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this);
int Cuboid_getAxisOfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition);
int Cuboid_testIfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement);
void Cuboid_draw(Cuboid this);
void Cuboid_deleteDirectDrawData(Cuboid this);
void Cuboid_print(Cuboid this, int x, int y);


#endif
