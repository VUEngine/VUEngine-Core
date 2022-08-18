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
#include <SpatialObject.h>
#include <Shape.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __COLLISION_ALL_LAYERS		0xFFFFFFFF


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class CollisionManager : ListenerObject
{
	// a list of registered shapes
	VirtualList	shapes;
	// a list of shapes that check for collisions against other shapes
	VirtualList	activeForCollisionCheckingShapes;
	// counters for statistics
	uint32 lastCycleCollisionChecks;
	uint32 lastCycleCollisions;
	uint32 collisionChecks;
	uint32 collisions;
	uint32 checkCycles;
	bool checkShapesOutOfCameraRange;

	/// @publicsection
	void constructor();
	void hideShapes();
	void print(int32 x, int32 y);
	Shape createShape(SpatialObject owner, const ShapeSpec* shapeSpec);
	void destroyShape(Shape shape);
	void reset();
	void activeCollisionCheckForShape(Shape shape, bool activate);
	void showShapes();
	void setCheckShapesOutOfCameraRange(bool value);
	uint32 update(Clock clock);
}


#endif
