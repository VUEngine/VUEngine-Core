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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InverseCuboid.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	InverseCuboid
 * @extends Cuboid
 * @ingroup physics
 */
__CLASS_DEFINITION(InverseCuboid, Cuboid);
__CLASS_FRIEND_DEFINITION(Cuboid);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void InverseCuboid_constructor(InverseCuboid this, SpatialObject owner);
bool InverseCuboid_overlapsWithRightCuboids(RightCuboid* first, RightCuboid* second);
bool InverseCuboid_overlapsCuboid(InverseCuboid this, Cuboid other);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(InverseCuboid, SpatialObject owner)
__CLASS_NEW_END(InverseCuboid, owner);

/**
 * Class constructor
 *
 * @memberof	InverseCuboid
 * @public
 *
 * @param this	Function scope
 * @param owner
 */
static void InverseCuboid_constructor(InverseCuboid this, SpatialObject owner)
{
	ASSERT(this, "InverseCuboid::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);
}

/**
 * Class destructor
 *
 * @memberof	InverseCuboid
 * @public
 *
 * @param this	Function scope
 */
 void InverseCuboid_destructor(InverseCuboid this)
{
	ASSERT(this, "InverseCuboid::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Check if two rectangles overlap
 *
 * @memberof	InverseCuboid
 * @public
 *
 * @param this	Function scope
 * @param shape Shape to check against
 *
 * @return		Whether it overlaps
 */
CollisionInformation InverseCuboid_overlaps(InverseCuboid this, Shape shape)
{
	ASSERT(this, "InverseCuboid::overlaps: null this");

	return (CollisionInformation){NULL, {0, 0, 0}, __NO_AXIS};

/*	if(__IS_INSTANCE_OF(InverseCuboid, shape))
	{
		return InverseCuboid_overlapsCuboid(this, __SAFE_CAST(Cuboid, shape));
	}

	return false;
	*/
}

/**
 * Check if overlaps with other Cuboid
 *
 * @memberof	InverseCuboid
 * @public
 *
 * @param this	Function scope
 * @param other Cuboid to check against
 *
 * @return		Whether it overlaps
 */
bool InverseCuboid_overlapsCuboid(InverseCuboid this, Cuboid other)
{
	ASSERT(this, "Cuboid::overlapsCuboid: null this");

	return InverseCuboid_overlapsWithRightCuboids(&this->rightCuboid, &other->rightCuboid);
}

/**
 * Check if overlaps with other Cuboid
 *
 * @memberof		InverseCuboid
 * @public
 *
 * @param first		RightCuboid to check
 * @param second 	RightCuboid to check against
 *
 * @return			Whether they overlap or not
 */
bool InverseCuboid_overlapsWithRightCuboids(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return ((first->x0 < second->x0) | (first->x1 > second->x1) |
		(first->y0 < second->y0) | (first->y1 > second->y1) |
		(first->z0 < second->z0) | (first->z1 > second->z1));
}
