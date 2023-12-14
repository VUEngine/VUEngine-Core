/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Collider.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __COLLISION_ALL_LAYERS		0xFFFFFFFF


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class SpatialObject;
class Clock;

/// @ingroup physics
class CollisionManager : ListenerObject
{
	// a list of registered shapes
	VirtualList	shapes;
	// counters for statistics
	uint16 lastCycleCheckProducts;
	uint16 lastCycleCollisionChecks;
	uint16 lastCycleCollisions;
	uint16 collisionChecks;
	uint16 collisions;
	uint16 checkCycles;
	bool checkCollidersOutOfCameraRange;
	bool dirty;

	/// @publicsection
	void constructor();
	void hideColliders();
	void print(int32 x, int32 y);
	Collider createCollider(SpatialObject owner, const ColliderSpec* shapeSpec);
	void destroyCollider(Collider collider);
	void reset();
	void showColliders();
	void setCheckCollidersOutOfCameraRange(bool value);
	uint32 update(Clock clock);
	void purgeDestroyedColliders();
}


#endif
