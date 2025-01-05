/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <Optics.h>
#include <Printing.h>
#include <Sphere.h>

#include "Ball.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Ball::project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max)
{
	// Project this onto the current normal
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Ball::constructor(Entity owner, const ColliderSpec* colliderSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderBallIndex;
	this->radius = 0;

	Ball::computeRadius(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Ball::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Ball::resize(fixed_t sizeDelta __attribute__((unused)))
{
	this->radius += sizeDelta;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Ball::configureWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	// Create a wireframe
	this->wireframe = Wireframe::safeCast(new Sphere(this->owner, NULL));

	if(!isDeleted(this->wireframe))
	{		
		if(NULL != this->componentSpec)
		{
			Sphere::setDisplacement(this->wireframe, Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement));
		}
		
		Sphere::setRadius(this->wireframe, this->radius);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Ball::print(int32 x, int32 y)
{
	Base::print(this, x, y);
	
	Printing::text(Printing::getInstance(), "R:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->radius), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.x), x + 2, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.y), x + 8, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.x - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.x + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.y - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.y + this->radius), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.z - this->radius), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->transformation->position.z + this->radius), x + 8, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Ball::computeRadius()
{
	NM_ASSERT(NULL != this->componentSpec, "Ball::computeRadius: NULL componentSpec");

	if(NULL == this->componentSpec)
	{
		return;
	}

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
