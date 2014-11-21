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

#ifndef PHYSICS_WORLD_H_
#define PHYSICS_WORLD_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <Body.h>
#include <Actor.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define PhysicalWorld_METHODS							\
		Object_METHODS									\


// declare the virtual methods which are redefined
#define PhysicalWorld_SET_VTABLE(ClassName)						\
		Object_SET_VTABLE(ClassName)							\


__CLASS(PhysicalWorld);

 


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
PhysicalWorld PhysicalWorld_getInstance();

// class's destructor
void PhysicalWorld_destructor(PhysicalWorld this);

// register a game entity
Body PhysicalWorld_registerBody(PhysicalWorld this, Actor owner, fix19_13 weight);

// remove a shape
void PhysicalWorld_unregisterBody(PhysicalWorld this, Actor owner);

// process removed shapes
void PhysicalWorld_processRemovedBodies(PhysicalWorld this);

// calculate collisions
void PhysicalWorld_start(PhysicalWorld this);

// calculate collisions
void PhysicalWorld_update(PhysicalWorld this);

// unregister all shapes
void PhysicalWorld_reset(PhysicalWorld this);

// check if an entity has been registered
int PhysicalWorld_isEntityRegistered(PhysicalWorld this, Actor owner);

// retrieve owner body
Body PhysicalWorld_getBody(PhysicalWorld this, Actor owner);

// retrieve friction
fix19_13 PhysicalWorld_getFriction(PhysicalWorld this);

// set friction
void PhysicalWorld_setFriction(PhysicalWorld this, fix19_13 friction);

// a body has awaked
void PhysicalWorld_bodyAwaked(PhysicalWorld this, Body body);

// inform of a change in the body
void PhysicalWorld_bodySleep(PhysicalWorld this, Body body);

// set gravity
void PhysicalWorld_setGravity(PhysicalWorld this, Acceleration gravity);

// retrieve gravity
const VBVec3D* PhysicalWorld_getGravity(PhysicalWorld this);

// get last elapsed time
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this);

// print status
void PhysicalWorld_print(PhysicalWorld this, int x, int y);

#endif /*PHYSICS_WORLD_H_*/
