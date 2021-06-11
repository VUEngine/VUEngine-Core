/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
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
class CollisionManager : Object
{
	// a list of registered shapes
	VirtualList	shapes;
	// a list of shapes that check for collisions against other shapes
	VirtualList	activeForCollisionCheckingShapes;
	// counters for statistics
	u32 lastCycleCollisionChecks;
	u32 lastCycleCollisions;
	u32 collisionChecks;
	u32 collisions;
	u32 checkCycles;
	bool clearActiveForCollisionCheckingShapes;

	/// @publicsection
	void constructor();
	void hideShapes();
	void print(int x, int y);
	Shape createShape(SpatialObject owner, const ShapeSpec* shapeSpec);
	void destroyShape(Shape shape);
	void reset();
	void activeCollisionCheckForShape(Shape shape, bool activate);
	void showShapes();
	u32 update(Clock clock);
}


#endif
