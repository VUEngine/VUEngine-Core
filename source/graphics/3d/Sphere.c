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

#include <Sphere.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VIPManager.h>
#include <WireframeManager.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Sphere::constructor(SphereSpec* sphereSpec)
{
	// construct base object
	Base::constructor(&sphereSpec->wireframeSpec);

	this->center = PixelVector::zero();
	this->radius = __ABS(sphereSpec->radius);
	this->scaledRadius = this->radius;
	this->drawCenter = sphereSpec->drawCenter;
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
void Sphere::render()
{
	NM_ASSERT(NULL != this->position, "Sphere::render: NULL position");

	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;

	Vector3D relativePosition = Vector3D::sub(*this->position, _previousCameraPosition);
	Sphere::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return;
	}

	relativePosition = Vector3D::rotate(relativePosition, _previousCameraInvertedRotation);
	this->center = Vector3D::projectToPixelVector(relativePosition, Optics::calculateParallax(relativePosition.z));
	this->scaledRadius = __METERS_TO_PIXELS(__FIXED_MULT(this->radius, Vector3D::getScale(relativePosition.z, false)));
}

/**
 * Draw
 */

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Sphere::draw()
{
	NM_ASSERT(NULL != this->position, "Sphere::render: NULL position");

	DirectDraw::drawColorCircumference(this->center, this->scaledRadius, this->color, this->bufferIndex, this->interlaced);

	if(this->drawCenter)
	{
		DirectDraw::drawColorPoint(this->center.x, this->center.y, this->center.parallax, this->color);
	}

	this->bufferIndex = !this->bufferIndex;
}
