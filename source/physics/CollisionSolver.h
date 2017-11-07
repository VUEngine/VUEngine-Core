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

#ifndef COLLISION_SOLVER_H_
#define COLLISION_SOLVER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

enum Axis
{
	kXAxis = 0,
	kYAxis,
	kZAxis,
	kLastAxis
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define CollisionSolver_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

#define CollisionSolver_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

#define CollisionSolver_ATTRIBUTES																		\
		Object_ATTRIBUTES																				\
		/**
		 * @var SpatialObject 	owner
		 * @brief				owner
		 * @memberof			CollisionSolver
		 */																								\
		SpatialObject owner;																			\
		/**
		 * @var VirtualList 	lastCollidingShape[kLastAxis]
		 * @brief				last colliding shape lists
		 * @memberof			CollisionSolver
		 */																								\
		VirtualList lastCollidingShape[kLastAxis];														\
		/**
		 * @var GeneralAxisFlag	sensibleToFriction
		 * @brief				flags to apply friction on each axis
		 * @memberof			CollisionSolver
		 */																								\
		GeneralAxisFlag sensibleToFriction;																\

__CLASS(CollisionSolver);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionSolver, SpatialObject owner);

void CollisionSolver_constructor(CollisionSolver this, SpatialObject owner);
void CollisionSolver_destructor(CollisionSolver this);

u16 CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape);
fix19_13 CollisionSolver_getSurroundingElasticity(CollisionSolver this, u16 axis);
Force CollisionSolver_getSurroundingFriction(CollisionSolver this);
void CollisionSolver_resetCollisionStatusOnAxis(CollisionSolver this, u16 movementAxis);
bool CollisionSolver_resolveCollision(CollisionSolver this, const CollisionInformation* collisionInformation);


#endif
