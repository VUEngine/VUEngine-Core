/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <DirectDraw.h>
#include <Math.h>
#include <Optics.h>
#include <WireframeManager.h>

#include "Sphere.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Sphere::constructor(GameObject owner, const SphereSpec* sphereSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &sphereSpec->wireframeSpec);

	this->position = PixelVector::zero();

	if(NULL == sphereSpec)
	{		
		this->radius = __PIXELS_TO_METERS(8);
		this->drawCenter = false;
	}
	else
	{
		this->radius = __ABS(sphereSpec->radius);
		this->drawCenter = sphereSpec->drawCenter;
	}

	this->scaledRadius = __METERS_TO_PIXELS(this->radius);
}
//---------------------------------------------------------------------------------------------------------
void Sphere::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Sphere::render(Vector3D relativePosition)
{
	NM_ASSERT(NULL != this->transformation, "Sphere::render: NULL transformation");

	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);

	this->position = PixelVector::projectVector3D(relativePosition, Optics::calculateParallax(relativePosition.z));
	this->scaledRadius = __METERS_TO_PIXELS(__FIXED_MULT(this->radius, Vector3D::getScale(relativePosition.z, false)));
}
//---------------------------------------------------------------------------------------------------------
bool Sphere::draw()
{
	NM_ASSERT(NULL != this->transformation, "Sphere::render: NULL transformation");

	bool drawn = false;

	drawn = DirectDraw::drawCircle(this->position, this->scaledRadius, this->color, this->bufferIndex, this->interlaced);

	if(this->drawCenter)
	{
		DirectDraw::drawPoint(this->position, this->color, this->bufferIndex, this->interlaced);
	}

	this->bufferIndex = !this->bufferIndex;

	return drawn;
}
//---------------------------------------------------------------------------------------------------------
void Sphere::setRadius(fixed_t radius)
{
	this->radius = __ABS(radius);
}
//---------------------------------------------------------------------------------------------------------
fixed_t Sphere::getRadius()
{
	return this->radius;
}
//---------------------------------------------------------------------------------------------------------
