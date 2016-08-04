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

#ifndef COLLISION_SOLVER_H_
#define COLLISION_SOLVER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

enum Axis
{
	kXAxis = 0,
	kYAxis,
	kZAxis,
	kLastAxis
};


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define CollisionSolver_METHODS(ClassName)																\
	    Object_METHODS(ClassName)																		\

#define CollisionSolver_SET_VTABLE(ClassName)															\
	    Object_SET_VTABLE(ClassName)																	\

#define CollisionSolver_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* owner */																						\
        SpatialObject owner;																			\
        /* last collinding object */																	\
        VirtualList lastCollidingSpatialObject[kLastAxis];												\
        /* owner's positions for collision handling */													\
        const VBVec3D* ownerPositionToCheck;															\
        const VBVec3D* ownerPositionToWrite;															\
        VBVec3D ownerPreviousPosition;																	\
        /* flags to apply friction on each axis */														\
        GeneralAxisFlag sensibleToFriction;																\

__CLASS(CollisionSolver);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionSolver, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite);

void CollisionSolver_constructor(CollisionSolver this, SpatialObject owner, const VBVec3D* ownerPositionToCheck, const VBVec3D* ownerPositionToWrite);
void CollisionSolver_destructor(CollisionSolver this);
void CollisionSolver_resetCollisionStatusOnAxis(CollisionSolver this, int movementAxis);
const VBVec3D* CollisionSolver_getOwnerPreviousPosition(CollisionSolver this);
void CollisionSolver_setOwnerPreviousPosition(CollisionSolver this, VBVec3D position);
int CollisionSolver_getAxisOfFutureCollision(CollisionSolver this, const Acceleration* acceleration, const Shape shape);
void CollisionSolver_alignToCollidingSpatialObject(CollisionSolver this, SpatialObject collidingSpatialObject, int axisOfCollision, const Scale* scale, bool registerObject);
int CollisionSolver_getAxisOfCollision(CollisionSolver this, SpatialObject collidingSpatialObject, int movementAxis, VBVec3D displacement);
int CollisionSolver_resolveCollision(CollisionSolver this, VirtualList collidingSpatialObjects, int movementAxis, VBVec3D displacement, const Scale* scale, bool registerObjects);
void CollisionSolver_alignTo(CollisionSolver this, SpatialObject spatialObject, int axis, int pad);
Force CollisionSolver_getSurroundingFriction(CollisionSolver this);
fix19_13 CollisionSolver_getCollisingSpatialObjectsTotalElasticity(CollisionSolver this, int axis);


#endif
