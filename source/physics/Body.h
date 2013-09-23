/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
#include <Actor.h>
#include <Mass.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define Body_METHODS								\
		Object_METHODS								\


#define Body_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)							\
	

#define Body_ATTRIBUTES									\
														\
	/* super's attributes */							\
	Object_ATTRIBUTES;									\
														\
	/* mass */											\
	Mass mass;											\
														\
	/* owner */											\
	Actor owner;										\
														\
	/* direction */										\
	Force appliedForce;									\
														\
	/* spatial position */								\
	VBVec3D position;									\
														\
	/* actor velocity on each instante */				\
	Velocity velocity;									\
														\
	/* state of actor's movement over each axis */		\
	/*MovementState movementState;*/					\
														\
	/* acelearion structure */							\
	Acceleration acceleration;							\
														\
	Direction previousDirection;						\
														\
	/* time for movement over each axis	*/				\
	unsigned long time;									\
														\
	/* type of movement (accelerated or not) */			\
	int movementType;									\
														\
	/* a pointer to the object I'm walking on */		\
	/* to being able to turn aroung and don't */		\
	/* fall to dead */									\
														\
	/* raise flag to make the body active */			\
	int active: 1;										\
														\
	/* raise flag to update body's physices */			\
	int awake: 1;										\
														\
	Actor objectBelow;

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
__CLASS_NEW_DECLARE(Body, __PARAMETERS(Actor owner, Mass mass));

// class's destructor
void Body_destructor(Body this);

// set game entity
void Body_setOwner(Body this, Actor owner);

// get game entity
Actor Body_getOwner(Body this);

// update
void Body_update(Body this, const VBVec3D* gravity);

// set active
void Body_setActive(Body this, int active);

// is active?
int Body_isActive(Body this);

// retrieve position
VBVec3D Body_getPosition(Body this);

// retrieve position
void Body_setPosition(Body this, const VBVec3D* position, Actor caller);

// retrieve state
int Body_isAwake(Body body);

// awake body
void Body_awake(Body body);

// go to sleep
void Body_sleep(Body body);

// add a force
void Body_addForce(Body this, const Force* force);

// where I'm moving or not
int Body_isMoving(Body this);

// retrieve velocity
Velocity Body_getVelocity(Body this);

// set movement type
void Body_setMovementType(Body this, int movementType);



#endif /*BODY_H_*/
