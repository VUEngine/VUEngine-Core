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

#include <Cuboid.h>
#include <InverseCuboid.h>
#include <Optics.h>
#include <Polygon.h>
#include <Math.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Cuboid, Shape);
__CLASS_FRIEND_DEFINITION(InverseCuboid);


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

#define MAX_NUMBER_OF_PASSES	100


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Shape SpatialObject_getShape(SpatialObject this);

static u8 Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement, VBVec3D previousPosition, bool (*overlapsFunction) (RightCuboid*, RightCuboid*));
static u8 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, Gap gap, VBVec3D displacement);
static void Cuboid_configurePolygon(Cuboid this, int renew);
static bool Cuboid_overlapsCuboid(Cuboid this, Cuboid other);
static bool Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid other);
static bool Cuboid_overlapsWithRightCuboid(RightCuboid* first, RightCuboid* second);
static bool Cuboid_overlapsWithInverseRightCuboid(RightCuboid* first, RightCuboid* second);



//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Cuboid, SpatialObject owner)
__CLASS_NEW_END(Cuboid, owner);


// class's constructor
void Cuboid_constructor(Cuboid this, SpatialObject owner)
{
	ASSERT(this, "Cuboid::constructor: null this");

	__CONSTRUCT_BASE(owner);

	this->polygon = NULL;
}

// class's destructor
void Cuboid_destructor(Cuboid this)
{
	ASSERT(this, "Cuboid::destructor: null this");

	Cuboid_deleteDirectDrawData(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

SpatialObject owner = NULL;
// check if two rects overlap
u8 Cuboid_overlaps(Cuboid this, Shape shape)
{
	ASSERT(this, "Cuboid::overlaps: null this");

	owner = this->owner;
	if(__GET_CAST(InverseCuboid, shape))
	{
		return Cuboid_overlapsInverseCuboid(this, __SAFE_CAST(InverseCuboid, shape));
	}
	else if(__GET_CAST(Cuboid, shape))
	{
		return Cuboid_overlapsCuboid(this, __SAFE_CAST(Cuboid, shape));
	}

	return false;
}

// check if overlaps with other rect
static bool Cuboid_overlapsWithRightCuboid(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return !(first->x0 > second->x1 || first->x1 < second->x0 ||
			 first->y0 > second->y1 || first->y1 < second->y0 ||
			 first->z0 > second->z1 || first->z1 < second->z0);
}

// check if overlaps with other rect
static bool Cuboid_overlapsWithInverseRightCuboid(RightCuboid* first, RightCuboid* second)
{
	ASSERT(first, "Cuboid::overlapsWithRightCuboids: null first");
	ASSERT(second, "Cuboid::overlapsWithRightCuboids: null second");

	// test for collision
	return (first->x0 < second->x0 || first->x1 > second->x1 ||
			 first->y0 < second->y0 || first->y1 > second->y1 ||
			 first->z0 < second->z0 || first->z1 > second->z1);
}

// check if overlaps with other rect
bool Cuboid_overlapsCuboid(Cuboid this, Cuboid other)
{
	ASSERT(this, "Cuboid::overlapsCuboid: null this");

	return Cuboid_overlapsWithRightCuboid(&this->positionedRightCuboid, &other->positionedRightCuboid);
}

bool Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid other)
{
	ASSERT(this, "Cuboid::overlapsInverseCuboid: null this");

	return Cuboid_overlapsWithInverseRightCuboid(&this->positionedRightCuboid, &other->positionedRightCuboid);
}

void Cuboid_setup(Cuboid this)
{
	ASSERT(this, "Cuboid::setup: null this");

	// cuboid's center if placed on P(0, 0, 0)
	this->rightCuboid.x1 = ITOFIX19_13((int)__VIRTUAL_CALL(u16, SpatialObject, getWidth, this->owner) >> 1);
	this->rightCuboid.y1 = ITOFIX19_13((int)__VIRTUAL_CALL(u16, SpatialObject, getHeight, this->owner) >> 1);
	this->rightCuboid.z1 = ITOFIX19_13((int)__VIRTUAL_CALL(u16, SpatialObject, getDepth, this->owner) >> 0);

	this->rightCuboid.x0 = -this->rightCuboid.x1;
	this->rightCuboid.y0 = -this->rightCuboid.y1;
	this->rightCuboid.z0 = 0;//-this->rightCuboid.z1;

	// if owner does not move
	if(!this->moves)
	{
		// position the shape to avoid in real time calculation
		const VBVec3D* ownerPosition = __VIRTUAL_CALL_UNSAFE(const VBVec3D*, SpatialObject, getPosition, this->owner);
		Gap ownerGap = __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, this->owner);

		// calculate gap on each side of the rightCuboid
		this->rightCuboid.x0 += ownerPosition->x + ITOFIX19_13(ownerGap.left);
		this->rightCuboid.x1 += ownerPosition->x - ITOFIX19_13(ownerGap.right);
		this->rightCuboid.y0 += ownerPosition->y + ITOFIX19_13(ownerGap.up);
		this->rightCuboid.y1 += ownerPosition->y - ITOFIX19_13(ownerGap.down);
		this->rightCuboid.z0 += ownerPosition->z;
		this->rightCuboid.z1 += ownerPosition->z;
	}

	this->positionedRightCuboid = this->rightCuboid;

	// no more setup needed
	this->ready = true;
}

// prepare the shape to be checked
void Cuboid_position(Cuboid this)
{
	ASSERT(this, "Cuboid::position: null this");

	Gap gap = __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, this->owner);

	// get owner's position
	const VBVec3D* myOwnerPosition = __VIRTUAL_CALL_UNSAFE(const VBVec3D*, SpatialObject, getPosition, this->owner);
	Velocity velocity = __VIRTUAL_CALL_UNSAFE(Velocity, SpatialObject, getVelocity, this->owner);

	// calculate positioned rightCuboid	
	this->positionedRightCuboid.x0 = this->rightCuboid.x0 + myOwnerPosition->x + ITOFIX19_13(gap.left);
	this->positionedRightCuboid.y0 = this->rightCuboid.y0 + myOwnerPosition->y + ITOFIX19_13(gap.up);
	this->positionedRightCuboid.z0 = this->rightCuboid.z0 + myOwnerPosition->z;
	this->positionedRightCuboid.x1 = this->rightCuboid.x1 + myOwnerPosition->x - ITOFIX19_13(gap.right);
	this->positionedRightCuboid.y1 = this->rightCuboid.y1 + myOwnerPosition->y - ITOFIX19_13(gap.down);
	this->positionedRightCuboid.z1 = this->rightCuboid.z1 + myOwnerPosition->z;

	VBVec3D lorentzFactor = 
	{
		FIX19_13_DIV(velocity.x, __LIGHT_SPEED),
		FIX19_13_DIV(velocity.y, __LIGHT_SPEED),
		FIX19_13_DIV(velocity.z, __LIGHT_SPEED)
	};
	
	this->positionedRightCuboid.x0 -= 0 > velocity.x? FIX19_13_MULT(velocity.x, lorentzFactor.x) : 0;
	this->positionedRightCuboid.y0 -= 0 > velocity.y? FIX19_13_MULT(velocity.y, lorentzFactor.y) : 0;
	this->positionedRightCuboid.z0 -= 0 > velocity.z? FIX19_13_MULT(velocity.z, lorentzFactor.z) : 0;

	this->positionedRightCuboid.x1 += 0 < velocity.x? FIX19_13_MULT(velocity.x, lorentzFactor.x) : 0;
	this->positionedRightCuboid.y1 += 0 < velocity.y? FIX19_13_MULT(velocity.y, lorentzFactor.y) : 0;
	this->positionedRightCuboid.z1 += 0 < velocity.z? FIX19_13_MULT(velocity.z, lorentzFactor.z) : 0;

	// not checked yet
	this->checked = false;
}

// retrieve rightCuboid
RightCuboid Cuboid_getRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getRightCuboid: null this");

	return this->rightCuboid;
}

// retrieve rightCuboid
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getPositionedRightCuboid: null this");

	return this->positionedRightCuboid;
}

// determine axis of collision
u8 Cuboid_getAxisOfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition)
{
	ASSERT(this, "Cuboid::getAxisOfCollision: null this");
	ASSERT(collidingSpatialObject, "Cuboid::getAxisOfCollision: null collidingSpatialObject");

	Shape shape = __VIRTUAL_CALL(Shape, SpatialObject, getShape, collidingSpatialObject);

	if(__GET_CAST(InverseCuboid, shape))
	{
		return Cuboid_getAxisOfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, shape), displacement, previousPosition, &Cuboid_overlapsWithInverseRightCuboid);
	}
	else if(__GET_CAST(Cuboid, shape))
	{
		return Cuboid_getAxisOfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, shape), displacement, previousPosition, &Cuboid_overlapsWithRightCuboid);
	}

	return 0;
}

// determine axis of collision
static u8 Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement, VBVec3D previousPosition, bool (*overlapsFunction) (RightCuboid*, RightCuboid*))
{
	ASSERT(this, "Cuboid::getAxisOfCollisionWithCuboid: null this");

	Gap gap = __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, this->owner);

	VBVec3D displacementIncrement =
	{
			FIX19_13_MULT(displacement.x, FTOFIX19_13(0.05f)),
			FIX19_13_MULT(displacement.y, FTOFIX19_13(0.05f)),
			FIX19_13_MULT(displacement.z, FTOFIX19_13(0.05f))
	};
	
	//NM_ASSERT(displacementIncrement.x || displacementIncrement.y || displacementIncrement.z, "Cuboid::getAxisOfCollisionWithCuboid: 0 displacementIncrement");

	// needed to calculate the axis to ignore
	displacement.x = 0;
	displacement.y = 0;
	displacement.z = 0;

	// get colliding entity's rightcuboid
	RightCuboid otherRightCuboid = {
				
		cuboid->positionedRightCuboid.x0,
		cuboid->positionedRightCuboid.y0,
		cuboid->positionedRightCuboid.z0,
		cuboid->positionedRightCuboid.x1,
		cuboid->positionedRightCuboid.y1,
		cuboid->positionedRightCuboid.z1
	};
		
	// setup a cuboid representing the previous position
	RightCuboid positionedRightCuboid =
	{
        this->rightCuboid.x0 + previousPosition.x + ITOFIX19_13(gap.left),
        this->rightCuboid.y0 + previousPosition.y + ITOFIX19_13(gap.up),
        this->rightCuboid.z0 + previousPosition.z - displacement.z,

        this->rightCuboid.x1 + previousPosition.x - ITOFIX19_13(gap.right),
        this->rightCuboid.y1 + previousPosition.y - ITOFIX19_13(gap.down),
        this->rightCuboid.z1 + previousPosition.z - displacement.z,
	};
	
	int numberOfAxis = 0;
	u8 axisOfCollision = 0;
	u8 axisToIgnore = __XAXIS | __YAXIS | __ZAXIS;
	int passes = 0;

	CACHE_ENABLE;

	if(displacementIncrement.x || displacementIncrement.y || displacementIncrement.z)
	{
		axisToIgnore = 0;
		
		// check for a collision on a single axis at a time
		do
		{
			axisOfCollision = 0;
	
			if(displacementIncrement.x)
		    {
				positionedRightCuboid.x0 += displacement.x;
				positionedRightCuboid.x1 += displacement.x;
	
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					if(!displacement.x)
					{
						axisToIgnore |= __XAXIS;
					}
	
					axisOfCollision |= __XAXIS;
					numberOfAxis++;
				}
	
				positionedRightCuboid.x0 -= displacement.x;
				positionedRightCuboid.x1 -= displacement.x;
			}
			else
			{
				axisToIgnore |= __XAXIS;
			}
	
			if(displacementIncrement.y)
	        {
				positionedRightCuboid.y0 += displacement.y;
				positionedRightCuboid.y1 += displacement.y;
	
				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					if(!displacement.y)
					{
						axisToIgnore |= __YAXIS;
					}
	
					axisOfCollision |= __YAXIS;
					numberOfAxis++;
				}
	
				positionedRightCuboid.y0 -= displacement.y;
				positionedRightCuboid.y1 -= displacement.y;
			}
			else
			{
				axisToIgnore |= __YAXIS;
			}
	
			if(displacementIncrement.z)
	        {
				positionedRightCuboid.z0 += displacement.z;
				positionedRightCuboid.z1 += displacement.z;
	
				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					if(!displacement.z)
					{
						axisToIgnore |= __ZAXIS;
					}
	
					axisOfCollision |= __ZAXIS;
					numberOfAxis++;
				}
	
				positionedRightCuboid.z0 -= displacement.z;
				positionedRightCuboid.z1 -= displacement.z;
			}
			else
			{
				axisToIgnore |= __ZAXIS;
			}
	
			if(0 == numberOfAxis)
	        {
				displacement.x += displacementIncrement.x;
				displacement.y += displacementIncrement.y;
				displacement.z += displacementIncrement.z;
				
				positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x + ITOFIX19_13(gap.left);
				positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y + ITOFIX19_13(gap.up);
				positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z - displacement.z;
				positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x - ITOFIX19_13(gap.right);
				positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y - ITOFIX19_13(gap.down);
				positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z - displacement.z;
			}
		}
		
		while (0 == numberOfAxis && ++passes < MAX_NUMBER_OF_PASSES);
	}

	// if not axis of collision was found
	if((passes >= MAX_NUMBER_OF_PASSES && !axisOfCollision) || (__XAXIS | __YAXIS | __ZAXIS) == axisToIgnore)
	{
		axisToIgnore = 0;
		
		passes = 0;
		displacement.x = 0;
		displacement.y = 0;
		displacement.z = 0;
		
		positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x + ITOFIX19_13(gap.left);
		positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y + ITOFIX19_13(gap.up);
		positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z - displacement.z;
		positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x - ITOFIX19_13(gap.right);
		positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y - ITOFIX19_13(gap.down);
		positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z - displacement.z;

		// test for collision carrying the displacement across all axixes
		do
		{
			numberOfAxis = 0;
			axisOfCollision = 0;

			if(displacementIncrement.x)
		    {
				positionedRightCuboid.x0 += displacement.x;
				positionedRightCuboid.x1 += displacement.x;

				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					axisOfCollision |= __XAXIS;
					break;
				}
			}

			if(displacementIncrement.y)
	        {
				positionedRightCuboid.y0 += displacement.y;
				positionedRightCuboid.y1 += displacement.y;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					axisOfCollision |= __YAXIS;
					break;
		
				}
			}

			if(displacementIncrement.z)
	        {
				positionedRightCuboid.z0 += displacement.z;
				positionedRightCuboid.z1 += displacement.z;

				// test for collision
				if(overlapsFunction(&positionedRightCuboid, &otherRightCuboid))
	            {
					axisOfCollision |= __ZAXIS;
				}
			}

			if(0 == numberOfAxis)
	        {
				displacement.x += displacementIncrement.x;
				displacement.y += displacementIncrement.y;
				displacement.z += displacementIncrement.z;
				
				positionedRightCuboid.x0 = this->rightCuboid.x0 + previousPosition.x + ITOFIX19_13(gap.left);
				positionedRightCuboid.y0 = this->rightCuboid.y0 + previousPosition.y + ITOFIX19_13(gap.up);
				positionedRightCuboid.z0 = this->rightCuboid.z0 + previousPosition.z - displacement.z;
				positionedRightCuboid.x1 = this->rightCuboid.x1 + previousPosition.x - ITOFIX19_13(gap.right);
				positionedRightCuboid.y1 = this->rightCuboid.y1 + previousPosition.y - ITOFIX19_13(gap.down);
				positionedRightCuboid.z1 = this->rightCuboid.z1 + previousPosition.z - displacement.z;
			}
		}
		while (0 == numberOfAxis && ++passes < MAX_NUMBER_OF_PASSES);
	}
	
	ASSERT(numberOfAxis || passes < MAX_NUMBER_OF_PASSES, "Cuboid::getAxisOfCollisionWithCuboid: max number of passes exceded");

	CACHE_DISABLE;
	
	return axisOfCollision & ~axisToIgnore;
}

// test if collision with the entity give the displacement
u8 Cuboid_testIfCollision(Cuboid this, SpatialObject collidingSpatialObject, VBVec3D displacement)
{
	ASSERT(this, "Cuboid::testIfCollision: null this");

	Shape shape = __VIRTUAL_CALL(Shape, SpatialObject, getShape, collidingSpatialObject);

	if(__GET_CAST(Cuboid, shape))
    {
		return Cuboid_testIfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, shape), __VIRTUAL_CALL_UNSAFE(Gap, SpatialObject, getGap, collidingSpatialObject), displacement);
	}

	return false;
}

// test if collision with the entity give the displacement
static u8 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, Gap gap, VBVec3D displacement)
{
	ASSERT(this, "Cuboid::testIfCollisionWithCuboid: null this");

	// setup a cuboid representing the previous position
	RightCuboid positionedRightCuboid;
	positionedRightCuboid.x0 = this->positionedRightCuboid.x0;
	positionedRightCuboid.y0 = this->positionedRightCuboid.y0;
	positionedRightCuboid.z0 = this->positionedRightCuboid.z0;
	positionedRightCuboid.x1 = this->positionedRightCuboid.x1;
	positionedRightCuboid.y1 = this->positionedRightCuboid.y1;
	positionedRightCuboid.z1 = this->positionedRightCuboid.z1;

	// get colliding entity's rightcuboid
	RightCuboid otherRightCuboid = {
			
		cuboid->positionedRightCuboid.x0,
		cuboid->positionedRightCuboid.y0,
		cuboid->positionedRightCuboid.z0,
		cuboid->positionedRightCuboid.x1,
		cuboid->positionedRightCuboid.y1,
		cuboid->positionedRightCuboid.z1
	};
		
	u8 axisOfPossibleCollision = 0;

	if(displacement.x)
    {
		positionedRightCuboid.x0 += displacement.x;
		positionedRightCuboid.x1 += displacement.x;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
        {
			axisOfPossibleCollision |= __XAXIS;
		}
	}

	if(displacement.y)
    {
		positionedRightCuboid.y0 += displacement.y;
		positionedRightCuboid.y1 += displacement.y;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
        {
			axisOfPossibleCollision |= __YAXIS;
		}
	}

	if(displacement.z)
    {
		positionedRightCuboid.z0 += displacement.z;
		positionedRightCuboid.z1 += displacement.z;

		// test for collision
		if(Cuboid_overlapsWithRightCuboid(&positionedRightCuboid, &otherRightCuboid))
		{
	    	axisOfPossibleCollision |= __ZAXIS;
		}
	}

	return axisOfPossibleCollision;
}

// configure polygon
static void Cuboid_configurePolygon(Cuboid this, int renew)
{
	ASSERT(this, "Cuboid::draw: null this");

	if(renew)
    {
		Cuboid_deleteDirectDrawData(this);
	}
	else if(this->polygon)
    {
		return;
	}

	// create a polygon
	this->polygon = __NEW(Polygon);

	// add vertices
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x1, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x1, this->positionedRightCuboid.y1, this->positionedRightCuboid.z0);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y1, this->positionedRightCuboid.z0);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z0);

	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x1, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x1, this->positionedRightCuboid.y1, this->positionedRightCuboid.z1);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y1, this->positionedRightCuboid.z1);
	Polygon_addVertex(this->polygon, this->positionedRightCuboid.x0, this->positionedRightCuboid.y0, this->positionedRightCuboid.z1);
}

// draw rect
void Cuboid_draw(Cuboid this)
{
	Cuboid_configurePolygon(this, this->moves || !this->ready);

	// draw the polygon
	Polygon_draw(this->polygon, true);
}

// flush direct draw data
void Cuboid_deleteDirectDrawData(Cuboid this)
{
	if(this->polygon)
    {
		__DELETE(this->polygon);

		this->polygon = NULL;
	}
}

// print debug data
void Cuboid_print(Cuboid this, int x, int y)
{
	ASSERT(this, "Cuboid::print: null this");

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