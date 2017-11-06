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

#include <Ball.h>
#include <Box.h>
#include <InverseBox.h>
#include <CollisionHelper.h>
#include <Vector.h>
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
 * @class	Ball
 * @extends Shape
 * @ingroup physics
 */
__CLASS_DEFINITION(Ball, Shape);


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

#define __MAX_NUMBER_OF_PASSES			10
#define __FLOAT_0_5_F					0x00001000


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static u16 Ball_testIfCollisionWithBall(Ball this, Ball Ball, VBVec3D displacement);
static void Ball_configureWireframe(Ball this, int renew);



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Ball, SpatialObject owner)
__CLASS_NEW_END(Ball, owner);


// class's constructor
void Ball_constructor(Ball this, SpatialObject owner)
{
	ASSERT(this, "Ball::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);

	this->center = (VBVec3D){0, 0, 0};
	this->radius = 0;
	this->sphere = NULL;
}

// class's destructor
void Ball_destructor(Ball this)
{
	ASSERT(this, "Ball::destructor: null this");

	Ball_hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void Ball_setup(Ball this, const VBVec3D* position, const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size)
{
	ASSERT(this, "Ball::setup: null this");

	this->center = *position;
	this->radius = __I_TO_FIX19_13(size->z);

	if(size->x > size->y)
	{
		if(size->x > size->z)
		{
			this->radius = __I_TO_FIX19_13(size->x);
		}
	}
	else if(size->y > size->z)
	{
		this->radius = __I_TO_FIX19_13(size->y);
	}

	// no more setup needed
	this->ready = true;
}

// check if two rectangles overlap
CollisionInformation Ball_overlaps(Ball this, Shape shape)
{
	ASSERT(this, "Ball::overlaps: null this");

	return CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), __SAFE_CAST(Shape, this), shape);
}

VBVec3D Ball_getMinimumOverlappingVector(Ball this, Shape shape)
{
	ASSERT(this, "Ball::getMinimumOverlappingVector: null this");

	return CollisionHelper_getMinimumOverlappingVector(CollisionHelper_getInstance(), __SAFE_CAST(Shape, this), shape);
}

void Ball_project(VBVec3D center, fix19_13 radius, VBVec3D vector, fix19_13* min, fix19_13* max)
{
	// project this onto the current normal
	fix19_13 dotProduct = Vector_dotProduct(vector, center);
//	fix19_13 dotProduct = Vector_dotProduct(center, vector);

	*min = dotProduct - radius;
	*max = dotProduct + radius;

	if(*min > *max)
	{
		fix19_13 aux = *min;
		*min = *max;
		*max = aux;
	}
}

// test if collision with the entity give the displacement
bool Ball_testIfCollision(Ball this, Shape collidingShape, VBVec3D displacement)
{
	ASSERT(this, "Ball::testIfCollision: null this");

	if(__IS_INSTANCE_OF(Ball, collidingShape))
	{
		return Ball_testIfCollisionWithBall(this, __SAFE_CAST(Ball, collidingShape), displacement);
	}
	// TODO: implement
//	else if(__IS_INSTANCE_OF(InverseBall, shape))

	return false;
}

// test if collision with the entity give the displacement
static u16 Ball_testIfCollisionWithBall(Ball this, Ball Ball, VBVec3D displacement)
{
	ASSERT(this, "Ball::testIfCollisionWithBall: null this");

	return false;
}

VBVec3D Ball_getPosition(Ball this)
{
	ASSERT(this, "Ball::getPosition: null this");

	return this->center;
}

RightBox Ball_getSurroundingRightBox(Ball this)
{
	ASSERT(this, "Ball::getSurroundingRightBox: null this");

	return (RightBox)
	{
		this->center.x - this->radius,
		this->center.y - this->radius,
		this->center.z - this->radius,

		this->center.x + this->radius,
		this->center.y + this->radius,
		this->center.z + this->radius,
	};
}

// configure Polyhedron
static void Ball_configureWireframe(Ball this, int renew)
{
	ASSERT(this, "Ball::draw: null this");

	if(renew)
	{
		Ball_hide(this);
	}
	else if(this->sphere)
	{
		return;
	}

	// create a wireframe
	this->sphere = __NEW(Sphere, this->center, this->radius);
}

// show me
void Ball_show(Ball this)
{
	ASSERT(this, "Ball::draw: null this");

//	Ball_configureWireframe(this, __VIRTUAL_CALL(SpatialObject, moves, this->owner) || !this->ready);
	Ball_configureWireframe(this, true);

	// draw the Polyhedron
	Wireframe_show(__SAFE_CAST(Wireframe, this->sphere));
}

// hide wireframe
void Ball_hide(Ball this)
{
	ASSERT(this, "Ball::hide: null this");

	if(this->sphere)
	{
		// draw the wireframe
		__DELETE(this->sphere);

		this->sphere = NULL;
	}
}

// print debug data
void Ball_print(Ball this, int x, int y)
{
	ASSERT(this, "Ball::print: null this");

	Printing_text(Printing_getInstance(), "X:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.x - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.x + this->radius), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Y:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.y - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.y + this->radius), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Z:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.z - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->center.z + this->radius), x + 7, y++, NULL);
}
