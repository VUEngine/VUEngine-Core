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

#ifndef BODY_H_
#define BODY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// movement type
#define	__UNIFORM_MOVEMENT		0x00
#define	__ACCELERATED_MOVEMENT	0x01


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Body_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, update);															\
		__VIRTUAL_DEC(ClassName, Force, calculateFrictionForce);										\

#define Body_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Body, update);														    \
		__VIRTUAL_SET(ClassName, Body, calculateFrictionForce);										    \

#define Body_ATTRIBUTES																					\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* owner */																						\
        SpatialObject owner;																			\
        /* direction */																					\
        Force appliedForce;																				\
        /* friction surrounding object */																\
        Force friction;																					\
        /* spatial position */																			\
        VBVec3D position;																				\
        /* velocity on each instante */																	\
        Velocity velocity;																				\
        /* acelearion structure */																		\
        Acceleration acceleration;																		\
        /* elasticity */																				\
        fix19_13 elasticity;																			\
        /* mass */																						\
        fix19_13 mass;																					\
        /* movement type on each axis */																\
        MovementType movementType;																		\
        /* raise flag to make the body active */														\
        u8 active;																				        \
        /* raise flag to update body's physics */														\
        u8 awake;																				        \
        /* axis that is subject to gravity */															\
        u8 axisSubjectToGravity;																		\

__CLASS(Body);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 										CLASS' STATIC METHODS
//---------------------------------------------------------------------------------------------------------

void Body_setCurrentWorldFriction(fix19_13 _currentWorldFriction);
void Body_setCurrentElapsedTime(fix19_13 currentElapsedTime);
void Body_setCurrentGravity(const Acceleration* currentGravity);


//---------------------------------------------------------------------------------------------------------
// 										CLASS' INSTANCE METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Body, SpatialObject owner, fix19_13 mass);

void Body_constructor(Body this, SpatialObject owner, fix19_13 mass);
void Body_destructor(Body this);
void Body_setOwner(Body this, SpatialObject owner);
SpatialObject Body_getOwner(Body this);
void Body_update(Body this);
VBVec3D Body_getLastDisplacement(Body this);
u8 Body_getAxisSubjectToGravity(Body this);
void Body_setAxisSubjectToGravity(Body this, u8 axisSubjectToGravity);
void Body_setActive(Body this, bool active);
bool Body_isActive(Body this);
const VBVec3D* Body_getPosition(Body this);
void Body_setPosition(Body this, const VBVec3D* position, SpatialObject caller);
fix19_13 Body_getElasticity(Body this);
void Body_setElasticity(Body this, fix19_13 elasticity);
Force Body_getFriction(Body this);
void Body_setFriction(Body this, Force friction);
fix19_13 Body_getMass(Body this);
void Body_setMass(Body this, fix19_13 mass);
bool Body_isAwake(Body body);
void Body_sleep(Body body);
void Body_clearForce(Body this);
void Body_applyForce(Body this, const Force* force, int clearAxis);
void Body_applyGravity(Body this, const Acceleration* gravity);
void Body_addForce(Body this, const Force* force);
int Body_isMoving(Body this);
Velocity Body_getVelocity(Body this);
Acceleration Body_getAcceleration(Body this);
Force Body_getAppliedForce(Body this);
MovementType Body_getMovementType(Body this);
void Body_clearAcceleration(Body this, int axis);
void Body_moveAccelerated(Body this, int axis);
void Body_moveUniformly(Body this, Velocity velocity);
void Body_printPhysics(Body this, int x, int y);
void Body_stopMovement(Body this, int axis);
void Body_bounce(Body this, int axis, int axisAllowedForBouncing, fix19_13 otherBodyElasticity);
void Body_takeHitFrom(Body this, Body other);
Force Body_calculateFrictionForce(Body this);


#endif
