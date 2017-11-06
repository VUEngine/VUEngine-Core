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

#include <InverseBox.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	InverseBox
 * @extends Box
 * @ingroup physics
 */
__CLASS_DEFINITION(InverseBox, Box);
__CLASS_FRIEND_DEFINITION(Box);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void InverseBox_constructor(InverseBox this, SpatialObject owner);
bool InverseBox_overlapsWithRightBoxs(RightBox* first, RightBox* second);
bool InverseBox_overlapsBox(InverseBox this, Box other);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(InverseBox, SpatialObject owner)
__CLASS_NEW_END(InverseBox, owner);

/**
 * Class constructor
 *
 * @memberof	InverseBox
 * @public
 *
 * @param this	Function scope
 * @param owner
 */
static void InverseBox_constructor(InverseBox this, SpatialObject owner)
{
	ASSERT(this, "InverseBox::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);
}

/**
 * Class destructor
 *
 * @memberof	InverseBox
 * @public
 *
 * @param this	Function scope
 */
 void InverseBox_destructor(InverseBox this)
{
	ASSERT(this, "InverseBox::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Check if two rectangles overlap
 *
 * @memberof	InverseBox
 * @public
 *
 * @param this	Function scope
 * @param shape Shape to check against
 *
 * @return		Whether it overlaps
 */
CollisionInformation InverseBox_overlaps(InverseBox this, Shape shape)
{
	ASSERT(this, "InverseBox::overlaps: null this");

	return (CollisionInformation){NULL, {0, 0, 0}, __NO_AXIS};

/*	if(__IS_INSTANCE_OF(InverseBox, shape))
	{
		return InverseBox_overlapsBox(this, __SAFE_CAST(Box, shape));
	}

	return false;
	*/
}

/**
 * Check if overlaps with other Box
 *
 * @memberof	InverseBox
 * @public
 *
 * @param this	Function scope
 * @param other Box to check against
 *
 * @return		Whether it overlaps
 */
bool InverseBox_overlapsBox(InverseBox this, Box other)
{
	ASSERT(this, "Box::overlapsBox: null this");

	return InverseBox_overlapsWithRightBoxs(&this->rightBox, &other->rightBox);
}

/**
 * Check if overlaps with other Box
 *
 * @memberof		InverseBox
 * @public
 *
 * @param first		RightBox to check
 * @param second 	RightBox to check against
 *
 * @return			Whether they overlap or not
 */
bool InverseBox_overlapsWithRightBoxs(RightBox* first, RightBox* second)
{
	ASSERT(first, "Box::overlapsWithRightBoxs: null first");
	ASSERT(second, "Box::overlapsWithRightBoxs: null second");

	// test for collision
	return ((first->x0 < second->x0) | (first->x1 > second->x1) |
		(first->y0 < second->y0) | (first->y1 > second->y1) |
		(first->z0 < second->z0) | (first->z1 > second->z1));
}
