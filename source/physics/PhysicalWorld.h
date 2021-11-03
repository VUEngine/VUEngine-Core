/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

typedef Body (*BodyAllocator)(SpatialObject, const PhysicalSpecification*, uint16 axisSubjectToGravity);


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
	uint8 remainingSkipCycles;
	uint8 skipCycles;

	/// @publicsection
	void constructor();
	void destructor();
	void setTimeScale(fix10_6 timeScale);
	uint32 getTimeScale();
	void bodyAwake(Body body);
	void bodySleep(Body body);
	void bodySetInactive(Body body);
	Body getBody(SpatialObject owner);
	fix10_6 getElapsedTime();
	fix10_6 getFrictionCoefficient();
	const Vector3D* getGravity();
	bool isSpatialObjectRegistered(SpatialObject owner);
	void print(int32 x, int32 y);
	void purgeBodyLists();
	Body createBody(BodyAllocator bodyAllocator, SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity);
	void destroyBody(Body body);
	void reset();
	void setGravity(Acceleration gravity);
	void setFrictionCoefficient(fix10_6 frictionCoefficient);
	void start();
	void update(Clock clock);
}


#endif
