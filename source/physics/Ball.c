/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal 'me engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <SpaceMath.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <HardwareManager.h>
#include <VirtualList.h>
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

	this->center = (Vector3D){0, 0, 0};
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

void Ball_position(Ball this, const Vector3D* position, const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size)
{
	ASSERT(this, "Ball::position: null this");

	this->center = *position;
	this->radius = size->z >> 1;

	if(size->x > size->y)
	{
		if(size->x > size->z)
		{
			this->radius = size->x >> 1;
		}
	}
	else if(size->y > size->z)
	{
		this->radius = size->y >> 1;
	}

	__CALL_BASE_METHOD(Shape, position, this, position, rotation, scale, size);
}

void Ball_project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max)
{
	// project this onto the current normal
	fix10_6 dotProduct = Vector3D_dotProduct(vector, center);

	*min = dotProduct - radius;
	*max = dotProduct + radius;

	if(*min > *max)
	{
		fix10_6 aux = *min;
		*min = *max;
		*max = aux;
	}
}

CollisionInformation Ball_testForCollision(Ball this, Shape shape, Vector3D displacement, fix10_6 sizeIncrement)
{
	ASSERT(this, "Ball::testForCollision: null this");

	// save state
	Vector3D center = this->center;
	fix10_6 radius = this->radius;
	this->radius += sizeIncrement;

	// add displacement
	this->center.x += displacement.x;
	this->center.y += displacement.y;
	this->center.z += displacement.z;

	// test for collision on displaced center
	CollisionInformation collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), __SAFE_CAST(Shape, this), shape);

	// restore state
	this->center = center;
	this->radius = radius;

	return collisionInformation;
}

Vector3D Ball_getPosition(Ball this)
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

	Printing_text(Printing_getInstance(), "C:         " , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.x), x + 2, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.y), x + 6, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.z), x + 10, y++, NULL);

	Printing_text(Printing_getInstance(), "X:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.x - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.x + this->radius), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Y:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.y - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.y + this->radius), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Z:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.z - this->radius), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX10_6_TO_I(this->center.z + this->radius), x + 7, y++, NULL);
}
