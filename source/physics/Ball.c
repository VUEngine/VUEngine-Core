/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal 'me engine for the Nintendo Virtual Boy
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Ball.h>
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
void Ball::constructor(SpatialObject owner)
{
	Base::constructor(owner);

	this->center = Vector3D::zero();
	this->radius = 0;
}

// class's destructor
void Ball::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Ball::position(const Vector3D* position, const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size)
{
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

	Ball::updateRightBox(this);

	Base::position(this, position, rotation, scale, size);
}

void Ball::updateRightBox()
{
	this->rightBox.x0 = this->center.x - this->radius;
	this->rightBox.y0 = this->center.y - this->radius;
	this->rightBox.z0 = this->center.z - this->radius;

	this->rightBox.x1 = this->center.x + this->radius;
	this->rightBox.y1 = this->center.y + this->radius;
	this->rightBox.z1 = this->center.z + this->radius;
}

/**
 * Set position
 *
 * @param position				Vector3d*
 */
void Ball::setPosition(const Vector3D* position)
{
	this->center = *position;

	Ball::updateRightBox(this);
}

static void Ball::project(Vector3D center, fix10_6 radius, Vector3D vector, fix10_6* min, fix10_6* max)
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

CollisionInformation Ball::testForCollision(Shape shape, Vector3D displacement, fix10_6 sizeIncrement)
{
	// save state
	Vector3D center = this->center;
	fix10_6 radius = this->radius;
	this->radius += sizeIncrement;

	// add displacement
	this->center.x += displacement.x;
	this->center.y += displacement.y;
	this->center.z += displacement.z;

	// test for collision on displaced center
	CollisionInformation collisionInformation = CollisionHelper::checkIfOverlap(CollisionHelper::getInstance(), Shape::safeCast(this), shape);

	// restore state
	this->center = center;
	this->radius = radius;

	return collisionInformation;
}

Vector3D Ball::getPosition()
{
	return this->center;
}

// configure Polyhedron
void Ball::configureWireframe()
{
	if(this->wireframe)
	{
		Sphere::setCenter(this->wireframe, this->center);
		return;
	}

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Sphere(this->center, this->radius, __COLOR_BRIGHT_RED));
}

// print debug data
void Ball::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "R:             " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->radius), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x), x + 2, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y), x + 8, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z + this->radius), x + 8, y++, NULL);
}
