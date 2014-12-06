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
 
#ifndef BODY_H_
#define BODY_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <Mass.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//movement type
#define	__UNIFORM_MOVEMENT		0x00
#define	__ACCELERATED_MOVEMENT	0x01

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Body_METHODS															\
		Object_METHODS															\

#define Body_SET_VTABLE(ClassName)												\
		Object_SET_VTABLE(ClassName)											\

#define Body_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* mass */																	\
	Mass mass;																	\
																				\
	/* owner */																	\
	Object owner;																\
																				\
	/* direction */																\
	Force appliedForce;															\
																				\
	/* friction sourrounding object */											\
	Force friction;																\
																				\
	/* spatial position */														\
	VBVec3D position;															\
																				\
	/* velocity on each instante */												\
	Velocity velocity;															\
																				\
	/* acelearion structure */													\
	Acceleration acceleration;													\
																				\
	/* movement type on each axis */											\
	MovementType movementType;													\
																				\
	/* elasticity */															\
	fix19_13 elasticity;														\
																				\
	/* raise flag to make the body active */									\
	u8 active: 1;																\
																				\
	/* raise flag to update body's physices */									\
	u8 awake: 1;																\

// A Body which represent a generic object inside a Stage
__CLASS(Body);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Body, __PARAMETERS(Object owner, fix19_13 weight));

// class's destructor
void Body_destructor(Body this);

// set game entity
void Body_setOwner(Body this, Object owner);

// get game entity
Object Body_getOwner(Body this);

// update
void Body_update(Body this, const Acceleration* gravity, fix19_13 elapsedTime);

// retrieve last displacement
VBVec3D Body_getLastDisplacement(Body this);

// set active
void Body_setActive(Body this, u8 active);

// is active?
u8 Body_isActive(Body this);

// retrieve position
VBVec3D Body_getPosition(Body this);

// retrieve position
void Body_setPosition(Body this, const VBVec3D* position, Object caller);

// get elasticiy
fix19_13 Body_getElasticity(Body this);

// set elasticity
void Body_setElasticity(Body this, fix19_13 elasticity);

// get friction
Force Body_getFriction(Body this);

// set friction
void Body_setFriction(Body this, Force friction);

// retrieve state
u8 Body_isAwake(Body body);

// go to sleep
void Body_sleep(Body body);

// clear force
void Body_clearForce(Body this);

// apply force
void Body_applyForce(Body this, const Force* force, int clear);

// apply gravity
void Body_applyGravity(Body this, const Acceleration* gravity);

// add a force
void Body_addForce(Body this, const Force* force);

// where I'm moving or not
u8 Body_isMoving(Body this);

// retrieve velocity
Velocity Body_getVelocity(Body this);

// retrieve acceleration
Acceleration Body_getAcceleration(Body this);

// retrieve movement type
MovementType Body_getMovementType(Body this);

// set movement type to accelerated
void Body_moveAccelerated(Body this, int axis);

// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity);

// print physical data
void Body_printPhysics(Body this, int x, int y);

// stop movement over an axis
void Body_stopMovement(Body this, int axis);

// bounce back
void Body_bounce(Body this, int axis, fix19_13 otherBodyElasticity);

// take a hit
void Body_takeHitFrom(Body this, Body other);


#endif /*BODY_H_*/
