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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Mass.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

//movement type
#define	__UNIFORM_MOVEMENT		0x00
#define	__ACCELERATED_MOVEMENT	0x01

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

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
	bool active: true;															\
																				\
	/* raise flag to update body's physices */									\
	bool awake: true;															\

// A Body which represent a generic object inside a Stage
__CLASS(Body);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Body, Object owner, fix19_13 weight);

void Body_destructor(Body this);
void Body_setOwner(Body this, Object owner);
Object Body_getOwner(Body this);
void Body_update(Body this, const Acceleration* gravity, fix19_13 elapsedTime);
VBVec3D Body_getLastDisplacement(Body this);
void Body_setActive(Body this, bool active);
bool Body_isActive(Body this);
VBVec3D Body_getPosition(Body this);
void Body_setPosition(Body this, const VBVec3D* position, Object caller);
fix19_13 Body_getElasticity(Body this);
void Body_setElasticity(Body this, fix19_13 elasticity);
Force Body_getFriction(Body this);
void Body_setFriction(Body this, Force friction);
bool Body_isAwake(Body body);
void Body_sleep(Body body);
void Body_clearForce(Body this);
void Body_applyForce(Body this, const Force* force, int clear);
void Body_applyGravity(Body this, const Acceleration* gravity);
void Body_addForce(Body this, const Force* force);
bool Body_isMoving(Body this);
Velocity Body_getVelocity(Body this);
Acceleration Body_getAcceleration(Body this);
MovementType Body_getMovementType(Body this);
void Body_moveAccelerated(Body this, int axis);
void Body_moveUniformly(Body this, Velocity velocity);
void Body_printPhysics(Body this, int x, int y);
void Body_stopMovement(Body this, int axis);
void Body_bounce(Body this, int axis, fix19_13 otherBodyElasticity);
void Body_takeHitFrom(Body this, Body other);


#endif