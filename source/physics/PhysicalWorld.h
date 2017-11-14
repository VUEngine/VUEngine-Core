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

#ifndef PHYSICAL_WORLD_H_
#define PHYSICAL_WORLD_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Body.h>
#include <SpatialObject.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define PhysicalWorld_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define PhysicalWorld_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(PhysicalWorld);


typedef Body (*BodyAllocator)(SpatialObject, const PhysicalSpecification*);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(PhysicalWorld);

void PhysicalWorld_constructor(PhysicalWorld this);
void PhysicalWorld_destructor(PhysicalWorld this);

void PhysicalWorld_setTimeScale(PhysicalWorld this, fix19_13 timeScale);
u32 PhysicalWorld_getTimeScale(PhysicalWorld this);
void PhysicalWorld_bodyAwake(PhysicalWorld this, Body body);
void PhysicalWorld_bodySleep(PhysicalWorld this, Body body);
Body PhysicalWorld_getBody(PhysicalWorld this, SpatialObject owner);
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this);
fix19_13 PhysicalWorld_getFrictionCoefficient(PhysicalWorld this);
const Vector3D* PhysicalWorld_getGravity(PhysicalWorld this);
bool PhysicalWorld_isSpatialObjectRegistered(PhysicalWorld this, SpatialObject owner);
void PhysicalWorld_print(PhysicalWorld this, int x, int y);
void PhysicalWorld_processAuxiliaryBodyLists(PhysicalWorld this);
Body PhysicalWorld_createBody(PhysicalWorld this, BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification);
void PhysicalWorld_destroyBody(PhysicalWorld this, Body body);
void PhysicalWorld_reset(PhysicalWorld this);
void PhysicalWorld_setGravity(PhysicalWorld this, Acceleration gravity);
void PhysicalWorld_setFrictionCoefficient(PhysicalWorld this, fix19_13 frictionCoefficient);
void PhysicalWorld_start(PhysicalWorld this);
void PhysicalWorld_update(PhysicalWorld this, Clock clock);


#endif
