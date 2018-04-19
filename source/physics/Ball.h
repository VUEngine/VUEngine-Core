/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef BALL_H_
#define BALL_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Sphere.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Ball_METHODS(ClassName)																			\
		Shape_METHODS(ClassName)																		\

#define Ball_SET_VTABLE(ClassName)																		\
		Shape_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, Ball, position);														\
		__VIRTUAL_SET(ClassName, Ball, testForCollision);												\
		__VIRTUAL_SET(ClassName, Ball, getPosition);													\
		__VIRTUAL_SET(ClassName, Ball, getSurroundingRightBox);											\
		__VIRTUAL_SET(ClassName, Ball, configureWireframe);												\
		__VIRTUAL_SET(ClassName, Ball, print);															\

#define Ball_ATTRIBUTES																					\
		Shape_ATTRIBUTES																				\
		/**
		 * @var fix10_6*	radius
		 * @brief			the radius of the ball
		 * @memberof 		Box
		 */																								\
		fix10_6 radius;																					\
		/**
		 * @var Vector3D		center
		 * @brief			the center of the ball
		 * @memberof 		Box
		 */																								\
		Vector3D center;																				\

__CLASS(Ball);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Ball, SpatialObject owner);

void Ball_constructor(Ball this, SpatialObject owner);
void Ball_destructor(Ball this);

void Ball_position(Ball this, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
void Ball_project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max);
CollisionInformation Ball_testForCollision(Ball this, Shape shape, Vector3D displacement, fix10_6 sizeIncrement);
Vector3D Ball_getPosition(Ball this);
RightBox Ball_getSurroundingRightBox(Ball this);
void Ball_configureWireframe(Ball this);
void Ball_print(Ball this, int x, int y);

#endif
