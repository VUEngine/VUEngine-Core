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

#include <CollisionHelper.h>
#include <DebugConfig.h>
#include <DebugUtilities.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Printing.h>
#include <Sphere.h>

#include "Ball.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Ball::constructor(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderBallIndex;
	this->radius = 0;

	Ball::computeRadius(this);
}

// class's destructor
void Ball::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Ball::computeRadius()
{
	Size size = Size::getFromPixelSize(((ColliderSpec*)this->componentSpec)->pixelSize);

	this->radius = size.z >> 1;

	if(size.x > size.y)
	{
		if(size.x > size.z)
		{
			this->radius = size.x >> 1;
		}
	}
	else if(size.y > size.z)
	{
		this->radius = size.y >> 1;
	}
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

void Ball::testForCollision(Collider collider, fixed_t sizeIncrement, CollisionInformation* collisionInformation)
{
	// save state
	fixed_t radius = this->radius;
	this->radius += sizeIncrement;

	CollisionHelper::checkIfOverlap(Collider::safeCast(this), collider, collisionInformation);

	// restore state
	this->radius = radius;
}

Vector3D Ball::getPosition()
{
	return *this->position;
}

// configure Polyhedron
void Ball::configureWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Sphere(this->owner, NULL));

	if(!isDeleted(this->wireframe))
	{		
		Sphere::setRadius(this->wireframe, this->radius);
	}
}

// print debug data
void Ball::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "R:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->radius), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->x), x + 2, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->y), x + 8, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->x - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->x + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->y - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->y + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->z - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->position->z + this->radius), x + 8, y++, NULL);
}
