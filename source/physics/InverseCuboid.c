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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InverseCuboid.h>
#include <Optics.h>
#include <Polygon.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(InverseCuboid, Cuboid);
__CLASS_FRIEND_DEFINITION(Cuboid);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void InverseCuboid_constructor(InverseCuboid this, SpatialObject owner);
bool InverseCuboid_overlapsWithRightCuboids(RightCuboid* first, RightCuboid* second);
bool InverseCuboid_overlapsCuboid(InverseCuboid this, Cuboid other);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(InverseCuboid, SpatialObject owner)
__CLASS_NEW_END(InverseCuboid, owner);

// class's constructor
static void InverseCuboid_constructor(InverseCuboid this, SpatialObject owner)
{
	ASSERT(this, "InverseCuboid::constructor: null this");

	__CONSTRUCT_BASE(owner);
}

// class's destructor
void InverseCuboid_destructor(InverseCuboid this)
{
	ASSERT(this, "InverseCuboid::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// check if two rects overlap
int InverseCuboid_overlaps(InverseCuboid this, Shape shape)
{
	ASSERT(this, "InverseCuboid::overlaps: null this");

	if(__GET_CAST(InverseCuboid, shape))
	{
		return InverseCuboid_overlapsCuboid(this, __SAFE_CAST(Cuboid, shape));
	}

	return false;
}

// check if overlaps with other rect
bool InverseCuboid_overlapsCuboid(InverseCuboid this, Cuboid other)
{
	ASSERT(this, "Cuboid::overlapsCuboid: null this");

	return InverseCuboid_overlapsWithRightCuboids(&this->positionedRightCuboid, &other->positionedRightCuboid);
}

// check if overlaps with other rect
bool InverseCuboid_overlapsWithRightCuboids(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return (first->x0 < second->x0 || first->x1 > second->x1 ||
		first->y0 < second->y0 || first->y1 > second->y1 ||
		first->z0 < second->z0 || first->z1 > second->z1);
}
