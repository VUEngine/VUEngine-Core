/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
void Ball::constructor(SpatialObject owner, const ShapeSpec* shapeSpec)
{
	Base::constructor(owner, shapeSpec);

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

static void Ball::project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max)
{
	// project this onto the current normal
	fixed_t dotProduct = Vector3D::dotProduct(vector, center);

	*min = dotProduct - radius;
	*max = dotProduct + radius;

	if(*min > *max)
	{
		fixed_t aux = *min;
		*min = *max;
		*max = aux;
	}
}

CollisionInformation Ball::testForCollision(Shape shape, Vector3D displacement, fixed_t sizeIncrement)
{
	// save state
	Vector3D center = this->center;
	fixed_t radius = this->radius;
	this->radius += sizeIncrement;

	// add displacement
	this->center.x += displacement.x;
	this->center.y += displacement.y;
	this->center.z += displacement.z;

	// test for collision on displaced center
	CollisionInformation collisionInformation = CollisionHelper::checkIfOverlap(Shape::safeCast(this), shape);

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
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Sphere(NULL));
	Wireframe::setup(this->wireframe, &this->center, NULL, NULL);
	Sphere::setRadius(this->wireframe, this->radius);
}

// print debug data
void Ball::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "R:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->radius), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x), x + 2, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y), x + 8, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.x + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.y + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->center.z + this->radius), x + 8, y++, NULL);
}
