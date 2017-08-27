/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal 'me engine for the Nintendo Virtual Boy
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

#include <Cuboid.h>
#include <InverseCuboid.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <Printing.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Cuboid
 * @extends Shape
 * @ingroup physics
 */
__CLASS_DEFINITION(Cuboid, Shape);
__CLASS_FRIEND_DEFINITION(InverseCuboid);


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

#define __MAX_NUMBER_OF_PASSES			10
#define __FLOAT_0_5_F					0x00001000

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static u16 Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement, VBVec3D previousPosition, bool (*overlapsFunction) (RightCuboid*, RightCuboid*));
static u16 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement);
static void Cuboid_configurePolyhedron(Cuboid this, int renew);
static bool Cuboid_overlapsCuboid(Cuboid this, Cuboid other);
static bool Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid other);
static bool Cuboid_overlapsWithRightCuboid(RightCuboid* first, RightCuboid* second);
static bool Cuboid_overlapsWithInverseRightCuboid(RightCuboid* first, RightCuboid* second);



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Cuboid, SpatialObject owner)
__CLASS_NEW_END(Cuboid, owner);


// class's constructor
void Cuboid_constructor(Cuboid this, SpatialObject owner)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);

	this->polyhedron = NULL;
}

// class's destructor
void Cuboid_destructor(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::destructor: null this");

	Cuboid_hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// check if two rectangles overlap
bool Cuboid_overlaps(Cuboid this, Shape shape)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::overlaps: null this");

	if(__IS_INSTANCE_OF(Cuboid, shape))
	{
		return Cuboid_overlapsCuboid(this, __SAFE_CAST(Cuboid, shape));
	}
	else if(__IS_INSTANCE_OF(InverseCuboid, shape))
	{
		return Cuboid_overlapsInverseCuboid(this, __SAFE_CAST(InverseCuboid, shape));
	}

	return false;
}

// check if overlaps with other rect
static bool Cuboid_overlapsWithRightCuboid(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return !((first->x0 > second->x1) | (first->x1 < second->x0) |
			 (first->y0 > second->y1) | (first->y1 < second->y0) |
			 (first->z0 > second->z1) | (first->z1 < second->z0));
}

// check if overlaps with other rect
static bool Cuboid_overlapsWithInverseRightCuboid(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return ((first->x0 < second->x0) | (first->x1 > second->x1) |
			 (first->y0 < second->y0) | (first->y1 > second->y1) |
			 (first->z0 < second->z0) | (first->z1 > second->z1));
}

// check if overlaps with other rect
bool Cuboid_overlapsCuboid(Cuboid this, Cuboid other)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::overlapsCuboid: null this");

	return Cuboid_overlapsWithRightCuboid(&this->positionedRightCuboid, &other->positionedRightCuboid);
}

bool Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid other)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::overlapsInverseCuboid: null this");

	return Cuboid_overlapsWithInverseRightCuboid(&this->positionedRightCuboid, &other->positionedRightCuboid);
}

void Cuboid_setup(Cuboid this, const VBVec3D* ownerPosition, const Size* size, const VBVec3D* displacement, bool moves)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::setup: null this");

	this->displacement = *displacement;

	// cuboid's center if placed on P(0, 0, 0)
	this->rightCuboid.x1 = ITOFIX19_13(size->x >> 1);
	this->rightCuboid.y1 = ITOFIX19_13(size->y >> 1);
	this->rightCuboid.z1 = ITOFIX19_13(size->z >> 1);

	this->rightCuboid.x0 = -this->rightCuboid.x1;
	this->rightCuboid.y0 = -this->rightCuboid.y1;
	this->rightCuboid.z0 = -this->rightCuboid.z1;

	// if owner does not move
	if(!moves)
	{
		// position the shape to avoid in real time calculation
		this->rightCuboid.x0 += ownerPosition->x + displacement->x;
		this->rightCuboid.x1 += ownerPosition->x + displacement->x;
		this->rightCuboid.y0 += ownerPosition->y + displacement->y;
		this->rightCuboid.y1 += ownerPosition->y + displacement->y;
		this->rightCuboid.z0 += ownerPosition->z + displacement->z;
		this->rightCuboid.z1 += ownerPosition->z + displacement->z;
	}

	this->positionedRightCuboid = this->rightCuboid;

	// no more setup needed
	this->ready = true;
}

// prepare the shape to be checked
void Cuboid_position(Cuboid this, const VBVec3D* myOwnerPosition, bool isAffectedByRelativity)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::position: null this");

	// calculate positioned rightCuboid
	this->positionedRightCuboid.x0 = this->rightCuboid.x0 + myOwnerPosition->x + this->displacement.x;
	this->positionedRightCuboid.y0 = this->rightCuboid.y0 + myOwnerPosition->y + this->displacement.y;
	this->positionedRightCuboid.z0 = this->rightCuboid.z0 + myOwnerPosition->z + this->displacement.z;
	this->positionedRightCuboid.x1 = this->rightCuboid.x1 + myOwnerPosition->x + this->displacement.x;
	this->positionedRightCuboid.y1 = this->rightCuboid.y1 + myOwnerPosition->y + this->displacement.y;
	this->positionedRightCuboid.z1 = this->rightCuboid.z1 + myOwnerPosition->z + this->displacement.z;

	if(isAffectedByRelativity)
	{
		Velocity velocity = __VIRTUAL_CALL(SpatialObject, getVelocity, this->owner);

		VBVec3D lorentzFactor =
		{
			FIX19_13_MULT(velocity.x, FIX19_13_DIV(velocity.x, __LIGHT_SPEED)),
			FIX19_13_MULT(velocity.y, FIX19_13_DIV(velocity.y, __LIGHT_SPEED)),
			FIX19_13_MULT(velocity.z, FIX19_13_DIV(velocity.z, __LIGHT_SPEED)),
		};

		this->positionedRightCuboid.x1 += lorentzFactor.x;
		this->positionedRightCuboid.x0 -= lorentzFactor.x;
		this->positionedRightCuboid.y1 += lorentzFactor.y;
		this->positionedRightCuboid.y0 -= lorentzFactor.y;
		this->positionedRightCuboid.z1 += lorentzFactor.z;
		this->positionedRightCuboid.z0 -= lorentzFactor.z;
	}

	// not checked yet
	this->checked = false;
}

// retrieve rightCuboid
RightCuboid Cuboid_getRightCuboid(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getRightCuboid: null this");

	return this->rightCuboid;
}

// retrieve rightCuboid
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getPositionedRightCuboid: null this");

	return this->positionedRightCuboid;
}

// determine axis of collision
u16 Cuboid_getAxisOfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement, VBVec3D previousPosition)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getAxisOfCollision: null this");
	ASSERT(collidingShape, "Cuboid::getAxisOfCollision: null collidingSpatialObject");

	if(__IS_INSTANCE_OF(Cuboid, collidingShape))
	{
		return Cuboid_getAxisOfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, collidingShape), displacement, previousPosition, &Cuboid_overlapsWithRightCuboid);
	}
	else if(__IS_INSTANCE_OF(InverseCuboid, collidingShape))
	{
		return Cuboid_getAxisOfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, collidingShape), displacement, previousPosition, &Cuboid_overlapsWithInverseRightCuboid);
	}

	return 0;
}

// determine axis of collision
static u16 Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement, VBVec3D previousPosition, bool (*overlapsFunction) (RightCuboid*, RightCuboid*))
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getAxisOfCollisionWithCuboid: null this");

	VBVec3D displacementIncrement = displacement;

	// get colliding entity's rightcuboid
	RightCuboid otherRightCuboid = cuboid->positionedRightCuboid;

	// setup a cuboid representing the previous position
	RightCuboid positionedRightCuboid =
	{
		this->rightCuboid.x0 + previousPosition.x,
		this->rightCuboid.y0 + previousPosition.y,
		this->rightCuboid.z0 + previousPosition.z,

		this->rightCuboid.x1 + previousPosition.x,
		this->rightCuboid.y1 + previousPosition.y,
		this->rightCuboid.z1 + previousPosition.z,
	};

	u16 axisOfCollision = 0;
	u16 axisToIgnore = 0;
	int passes = 0;

	displacement.x = 0;
	displacement.y = 0;
	displacement.z = 0;

	if(displacementIncrement.x | displacementIncrement.y | displacementIncrement.z)
	{
		axisToIgnore = 0;

		if(!displacementIncrement.x)
		{
			axisToIgnore |= __X_AXIS;
		}

		if(!displacementIncrement.y)
		{
			axisToIgnore |= __Y_AXIS;
		}

		if(!displacementIncrement.z)
		{
			axisToIgnore |= __Z_AXIS;
		}

		// check for a collision on a single axis at a time
		do
		{
			axisOfCollision = 0;

			if(!(__X_AXIS & axisToIgnore))
			{
				positionedRightCuboid.x0 += displacement.x;
				positionedRightCuboid.x1 += displacement.x;

				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __X_AXIS;
					break;
				}

				positionedRightCuboid.x0 -= displacement.x;
				positionedRightCuboid.x1 -= displacement.x;

				displacement.x += displacementIncrement.x;
				positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x;
				positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x;
			}

			if(!(__Y_AXIS & axisToIgnore))
			{
				positionedRightCuboid.y0 += displacement.y;
				positionedRightCuboid.y1 += displacement.y;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __Y_AXIS;
					break;
				}

				positionedRightCuboid.y0 -= displacement.y;
				positionedRightCuboid.y1 -= displacement.y;

				displacement.y += displacementIncrement.y;
				positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y;
				positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y;
			}

			if(!(__Z_AXIS & axisToIgnore))
			{
				positionedRightCuboid.z0 += displacement.z;
				positionedRightCuboid.z1 += displacement.z;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __Z_AXIS;
					break;
				}

				positionedRightCuboid.z0 -= displacement.z;
				positionedRightCuboid.z1 -= displacement.z;

				displacement.z += displacementIncrement.z;
				positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z - displacement.z;
				positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z - displacement.z;
			}
		}

		while(++passes < __MAX_NUMBER_OF_PASSES);
	}


	// if not axis of collision was found
	if((passes >= __MAX_NUMBER_OF_PASSES && !axisOfCollision) || (__X_AXIS | __Y_AXIS | __Z_AXIS) == axisToIgnore)
	{
		axisToIgnore = 0;
		passes = 0;
		displacement = displacementIncrement;

		positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x;
		positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y;
		positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z;
		positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x;
		positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y;
		positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z;

		// test for collision carrying the displacement across all axixes
		do
		{
			axisOfCollision = 0;

			if(displacementIncrement.x)
			{
				positionedRightCuboid.x0 += displacement.x;
				positionedRightCuboid.x1 += displacement.x;

				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __X_AXIS;
					break;
				}

				displacement.x += displacementIncrement.x;
				positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x;
				positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x;
			}

			if(displacementIncrement.y)
			{
				positionedRightCuboid.y0 += displacement.y;
				positionedRightCuboid.y1 += displacement.y;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __Y_AXIS;
					break;
				}

				displacement.y += displacementIncrement.y;
				positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y;
				positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y;
			}

			if(displacementIncrement.z)
			{
				positionedRightCuboid.z0 += displacement.z;
				positionedRightCuboid.z1 += displacement.z;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
				{
					axisOfCollision |= __Z_AXIS;
					break;
				}

				displacement.z += displacementIncrement.z;
				positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z - displacement.z;
				positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z - displacement.z;
			}
		}
		while(++passes < __MAX_NUMBER_OF_PASSES);
	}

	return axisOfCollision & ~axisToIgnore;
}

// test if collision with the entity give the displacement
bool Cuboid_testIfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::testIfCollision: null this");

	if(__IS_INSTANCE_OF(Cuboid, collidingShape))
	{
		return Cuboid_testIfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, collidingShape), displacement);
	}
	// TODO: implement
	/*
	else if(__IS_INSTANCE_OF(InverseCuboid, shape))
	{
		return Cuboid_getAxisOfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, shape), displacement, previousPosition, &Cuboid_overlapsWithInverseRightCuboid);
	}
	*/

	return false;
}

VBVec3D Cuboid_getPosition(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getPosition: null this");

	VBVec3D position =
	{
		(this->rightCuboid.x1 - this->rightCuboid.x0) >> 1,
		(this->rightCuboid.y1 - this->rightCuboid.y0) >> 1,
		(this->rightCuboid.z1 - this->rightCuboid.z0) >> 1,
	};

	return position;
}

RightCuboid Cuboid_getSurroundingRightCuboid(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::getSurroundingRightCuboid: null this");

	return this->rightCuboid;
}


// test if collision with the entity give the displacement
static u16 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::testIfCollisionWithCuboid: null this");

	// setup a cuboid representing the previous position
	RightCuboid positionedRightCuboid = this->positionedRightCuboid;

	// get colliding entity's rightcuboid
	RightCuboid otherRightCuboid = cuboid->positionedRightCuboid;

	u16 axisOfPossibleCollision = 0;

	if(displacement.x)
	{
		positionedRightCuboid.x0 += displacement.x;
		positionedRightCuboid.x1 += displacement.x;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
		{
			axisOfPossibleCollision |= __X_AXIS;
		}
	}

	if(displacement.y)
	{
		positionedRightCuboid.y0 += displacement.y;
		positionedRightCuboid.y1 += displacement.y;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
		{
			axisOfPossibleCollision |= __Y_AXIS;
		}
	}

	if(displacement.z)
	{
		positionedRightCuboid.z0 += displacement.z;
		positionedRightCuboid.z1 += displacement.z;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
		{
			axisOfPossibleCollision |= __Z_AXIS;
		}
	}

	return axisOfPossibleCollision;
}

// configure Polyhedron
static void Cuboid_configurePolyhedron(Cuboid this, int renew)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::draw: null this");

	if(renew)
	{
		Cuboid_hide(this);
	}
	else if(this->polyhedron)
	{
		return;
	}

	// create a Polyhedron
	this->polyhedron = __NEW(Polyhedron);

	// add vertices
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x1, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x1, this->positionedRightCuboid.y1, this->positionedRightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y1, this->positionedRightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);

	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x1, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x1, this->positionedRightCuboid.y1, this->positionedRightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y1, this->positionedRightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
}

// show me
void Cuboid_show(Cuboid this)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::draw: null this");

	Cuboid_configurePolyhedron(this, __VIRTUAL_CALL(SpatialObject, moves, this->owner) || !this->ready);

	// draw the Polyhedron
	Polyhedron_show(this->polyhedron);
}

// hide polyhedron
void Cuboid_hide(Cuboid this)
{
	if(this->polyhedron)
	{
		// draw the Polyhedron
		__DELETE(this->polyhedron);

		this->polyhedron = NULL;
	}
}

// print debug data
void Cuboid_print(Cuboid this, int x, int y)
{
	ASSERT(__SAFE_CAST(Cuboid, this), "Cuboid::print: null this");

	RightCuboid rightCuboid = this->positionedRightCuboid;

	Printing_text(Printing_getInstance(), "X:" , x, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.x0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.x1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Y:" , x, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.y0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.y1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Z:" , x, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.z0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(rightCuboid.z1), x + 7, y++, NULL);
}
