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

#include <DirectDraw.h>
#include <Math.h>
#include <Optics.h>
#include <WireframeManager.h>

#include "Sphere.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Sphere::constructor(SpatialObject owner, SphereSpec* sphereSpec)
{
	// construct base object
	Base::constructor(owner, &sphereSpec->wireframeSpec);

	this->center = PixelVector::zero();

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

/**
 * Class destructor
 */
void Sphere::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Retrieve this->center
 *
 * @return 	Sphere's this->center
 */
PixelVector Sphere::getCenter()
{
	return this->center;
}

/**
 * Set this->center
 *
 * @param this->center New value
 */
void Sphere::setCenter(PixelVector center)
{
	this->center = center;
}

/**
 * Retrieve this->radius
 *
 * @return 	Sphere's this->radius
 */
fixed_t Sphere::getRadius()
{
	return this->radius;
}

/**
 * Set radius
 *
 * @param this->radius New value
 */
void Sphere::setRadius(fixed_t radius)
{
	this->radius = __ABS(radius);
}

/**
 * Set radius scale
 *
 * @param radiusScale Scale
 */
void Sphere::setRadiusScale(fixed_t radiusScale)
{
	this->scaledRadius = __METERS_TO_PIXELS(__FIXED_MULT(this->radius, radiusScale));
}

/**
 * Render
 */
bool Sphere::render()
{
	NM_ASSERT(NULL != this->transformation, "Sphere::render: NULL transformation");

	Vector3D relativePosition = Vector3D::rotate(Vector3D::sub(Vector3D::sum(this->transformation->position, this->displacement), _previousCameraPosition), _previousCameraInvertedRotation);
	this->center = Vector3D::projectToPixelVector(relativePosition, Optics::calculateParallax(relativePosition.z));

	Sphere::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return false;
	}

	this->scaledRadius = __METERS_TO_PIXELS(__FIXED_MULT(this->radius, Vector3D::getScale(relativePosition.z, false)));
	this->scaledRadius = __METERS_TO_PIXELS(__FIXED_MULT(this->radius, __1I_FIXED));

	return true;
}

/**
 * Draw
 */

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
bool Sphere::draw()
{
	NM_ASSERT(NULL != this->transformation, "Sphere::render: NULL transformation");

	bool drawn = false;

	drawn = DirectDraw::drawColorCircumference(this->center, this->scaledRadius, this->color, this->bufferIndex, this->interlaced);

	if(this->drawCenter)
	{
		DirectDraw::drawColorPoint(this->center.x, this->center.y, this->center.parallax, this->color);
	}

	this->bufferIndex = !this->bufferIndex;

	return drawn;
}
