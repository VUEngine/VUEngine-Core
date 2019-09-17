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

#include <LineField.h>
#include <Box.h>
#include <InverseBox.h>
#include <CollisionHelper.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void LineField::constructor(SpatialObject owner)
{
	Base::constructor(owner);

	this->a = Vector3D::zero();
	this->b = Vector3D::zero();
	this->normal = Vector3D::zero();
	this->normalLength = __I_TO_FIX7_9(1);
}

// class's destructor
void LineField::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void LineField::position(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size)
{
	if(size->x)
	{

		this->a.x = size->x >> 1;
		this->a.y = 0;
		this->a.y = 0;

		if(rotation->y)
		{
			this->a.x = __FIX10_6_MULT((size->x >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->y)));
			this->a.z = __FIX10_6_MULT((size->x >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->y)));
			this->a.y = 0;
		}
		else if(rotation->z)
		{
			this->a.x = __FIX10_6_MULT((size->x >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->z)));
			this->a.y = __FIX10_6_MULT((size->x >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->z)));
			this->a.z = 0;
		}

		this->b = Vector3D::scalarProduct(this->a, __I_TO_FIX10_6(-1));

		this->a = Vector3D::sum(this->a, *position);
		this->b = Vector3D::sum(this->b, *position);

		fix10_6 dx = this->b.x - this->a.x;
		fix10_6 dy = this->b.y - this->a.y;
		fix10_6 dz = this->b.z - this->a.z;

		fix7_9 normalScale = __I_TO_FIX7_9(1);

		if(scale->x > normalScale)
		{
			normalScale = scale->x;
		}

		if(scale->y > normalScale)
		{
			normalScale = scale->y;
		}

		if(scale->z > normalScale)
		{
			normalScale = scale->z;
		}

		this->normalLength = __FIX10_6_MULT(__PIXELS_TO_METERS(8), __FIX7_9_TO_FIX10_6(normalScale));

		this->normal = Vector3D::normalize((Vector3D){dy, -dx, dz});
	}
	else if(size->y)
	{
		this->a.x = 0;
		this->a.y = size->y >> 1;
		this->a.y = 0;

		if(rotation->x)
		{
			this->a.x = __FIX10_6_MULT((size->y >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->x)));
			this->a.z = __FIX10_6_MULT((size->y >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->x)));
			this->a.y = 0;
		}
		else if(rotation->z)
		{
			this->a.x = __FIX10_6_MULT((size->y >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->z)));
			this->a.y = __FIX10_6_MULT((size->y >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->z)));
			this->a.z = 0;
		}

		this->normal = (Vector3D){__I_TO_FIX10_6(1), 0, 0};
	}
	else if(size->z)
	{
		this->a.x = 0;
		this->a.y = 0;
		this->a.z = size->z >> 1;

		if(rotation->x)
		{
			this->a.x = __FIX10_6_MULT((size->z >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->x)));
			this->a.y = 0;
			this->a.z = __FIX10_6_MULT((size->z >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->x)));
		}
		else if(rotation->y)
		{
			this->a.x = __FIX10_6_MULT((size->z >> 1), __FIX7_9_TO_FIX10_6(__COS(rotation->y)));
			this->a.y = __FIX10_6_MULT((size->z >> 1), __FIX7_9_TO_FIX10_6(__SIN(rotation->y)));
			this->a.z = 0;
		}

		this->normal = (Vector3D){0, __I_TO_FIX10_6(1), 0};
	}
	
	Base::position(this, position, rotation, scale, size);
}

static void LineField::project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max)
{
	// project this onto the current normal
	fix10_6 dotProduct = Vector3D::dotProduct(vector, center);

	*min = dotProduct - radius;
	*max = dotProduct + radius;

	if(*min > *max)
	{
		fix10_6 aux = *min;
		*min = *max;
		*max = aux;
	}
}

CollisionInformation LineField::testForCollision(Shape shape, Vector3D displacement, fix10_6 sizeIncrement)
{
/*	// save state
	Vector3D center = this->center;
	fix10_6 radius = this->radius;
	this->radius += sizeIncrement;

	// add displacement
	this->center.x += displacement.x;
	this->center.y += displacement.y;
	this->center.z += displacement.z;
*/
	// test for collision on displaced center
	CollisionInformation collisionInformation = CollisionHelper::checkIfOverlap(CollisionHelper::getInstance(), Shape::safeCast(this), shape);

	// restore state
//	this->center = center;
//	this->radius = radius;

	return collisionInformation;
}

Vector3D LineField::getPosition()
{
	return Vector3D::intermediate(this->a, this->b);
}

RightBox LineField::getSurroundingRightBox()
{
	return (RightBox)
	{
		this->a.x,
		this->a.y,
		this->a.z,

		this->b.x,
		this->b.y,
		this->b.z,
	};
}

// configure Polyhedron
void LineField::configureWireframe()
{
	if(this->wireframe)
	{
		return;
	}

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Line(this->a, this->b, Vector3D::scalarProduct(this->normal, this->normalLength), __COLOR_BRIGHT_RED));
}

// print debug data
void LineField::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "L:             " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::length(Vector3D::get(this->a, this->b))), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).x), x + 2, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).y), x + 8, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->a.x), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->b.x), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->a.y), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->b.y), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->a.z), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->a.z), x + 8, y++, NULL);
}
