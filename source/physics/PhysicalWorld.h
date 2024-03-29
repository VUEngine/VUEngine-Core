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

#include <ListenerObject.h>
#include <Body.h>
#include <SpatialObject.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef Body (*BodyAllocator)(SpatialObject, const PhysicalProperties*, uint16 axisSubjectToGravity);


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __PHYSICS_TIME_ELAPSED			__FIX7_9_EXT_DIV(__1I_FIX7_9_EXT, __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__TARGET_FPS), __I_TO_FIX7_9_EXT(__PHYSICS_TIME_ELAPSED_DIVISOR)))


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class PhysicalWorld : ListenerObject
{
	// list of registered bodies
	VirtualList	bodies;
	// gravity
	Vector3D gravity;
	// frictionCoefficient
	fixed_t frictionCoefficient;
	// time scale
	fixed_t timeScale;
	// Time scale is handled here, not in the Body class
	uint8 cycle;
	uint8 remainingSkipCycles;
	uint8 skipCycles;
	bool dirty;

	/// @publicsection
	void constructor();
	void destructor();
	void setTimeScale(fixed_t timeScale);
	uint32 getTimeScale();
	void bodyAwake(Body body);
	void bodySleep(Body body);
	void bodySetInactive(Body body);
	Body getBody(SpatialObject owner);
	fixed_t getElapsedTime();
	fixed_t getFrictionCoefficient();
	const Vector3D* getGravity();
	bool isSpatialObjectRegistered(SpatialObject owner);
	void print(int32 x, int32 y);
	void purgeBodyLists();
	Body createBody(SpatialObject owner, const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity);
	void destroyBody(Body body);
	void reset();
	void setGravity(Vector3D gravity);
	void setFrictionCoefficient(fixed_t frictionCoefficient);
	void start();
	void update(Clock clock);
}


#endif
