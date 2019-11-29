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
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef Body (*BodyAllocator)(SpatialObject, const PhysicalSpecification*, u16 axisSubjectToGravity);


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __PHYSICS_TIME_ELAPSED			__FIX10_6_DIV(__1I_FIX10_6, __I_TO_FIX10_6(__TARGET_FPS / __PHYSICS_TIME_ELAPSED_DIVISOR))
#define __TOTAL_USABLE_BODIES			128


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class PhysicalWorld : Object
{
	// list of registered bodies
	VirtualList	bodies;
	// a list of bodies which must detect collisions
	VirtualList	activeBodies;
	// gravity
	Acceleration gravity;
	// frictionCoefficient
	fix10_6 frictionCoefficient;
	// body to check for gravity
	VirtualNode bodyToCheckForGravityNode;
	// time scale
	fix10_6 timeScale;
	// Time scale is handled here, not in the Body class
	u8 remainingSkipCycles;
	u8 skipCycles;

	/// @publicsection
	void constructor();
	void destructor();
	void setTimeScale(fix10_6 timeScale);
	u32 getTimeScale();
	void bodyAwake(Body body);
	void bodySleep(Body body);
	void bodySetInactive(Body body);
	Body getBody(SpatialObject owner);
	fix10_6 getElapsedTime();
	fix10_6 getFrictionCoefficient();
	const Vector3D* getGravity();
	bool isSpatialObjectRegistered(SpatialObject owner);
	void print(int x, int y);
	void purgeBodyLists();
	Body createBody(BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axisSubjectToGravity);
	void destroyBody(Body body);
	void reset();
	void setGravity(Acceleration gravity);
	void setFrictionCoefficient(fix10_6 frictionCoefficient);
	void start();
	void update(Clock clock);
}


#endif
