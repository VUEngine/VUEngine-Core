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

#ifndef PHYSICAL_WORLD_H_
#define PHYSICAL_WORLD_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Body.h>
#include <SpatialObject.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define PhysicalWorld_METHODS																			\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define PhysicalWorld_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(PhysicalWorld);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(PhysicalWorld);

void PhysicalWorld_constructor(PhysicalWorld this);
void PhysicalWorld_destructor(PhysicalWorld this);
Body PhysicalWorld_registerBody(PhysicalWorld this, SpatialObject owner, fix19_13 mass);
void PhysicalWorld_unregisterBody(PhysicalWorld this, SpatialObject owner);
void PhysicalWorld_processRemovedBodies(PhysicalWorld this);
void PhysicalWorld_start(PhysicalWorld this);
void PhysicalWorld_update(PhysicalWorld this, Clock clock);
void PhysicalWorld_reset(PhysicalWorld this);
bool PhysicalWorld_isSpatialObjectRegistered(PhysicalWorld this, SpatialObject owner);
Body PhysicalWorld_getBody(PhysicalWorld this, SpatialObject owner);
fix19_13 PhysicalWorld_getFriction(PhysicalWorld this);
void PhysicalWorld_setFriction(PhysicalWorld this, fix19_13 friction);
void PhysicalWorld_bodyAwaked(PhysicalWorld this, Body body);
void PhysicalWorld_bodySleep(PhysicalWorld this, Body body);
void PhysicalWorld_setGravity(PhysicalWorld this, Acceleration gravity);
const VBVec3D* PhysicalWorld_getGravity(PhysicalWorld this);
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this);
void PhysicalWorld_print(PhysicalWorld this, int x, int y);


#endif