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

	this->center = sphereSpec->center;
	this->radius = __ABS(sphereSpec->radius);
	this->normalizedCenter3D = Vector3D::zero();
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
 * Retrieve center
 *
 * @return 	Sphere's center
 */
Vector3D Sphere::getCenter()
{
	return this->center;
}

/**
 * Set center
 *
 * @param center New value
 */
void Sphere::setCenter(Vector3D center)
{
	this->center = center;
}

/**
 * Retrieve radius
 *
 * @return 	Sphere's radius
 */
fix10_6 Sphere::getRadius()
{
	return this->radius;
}

/**
 * Set radius
 *
 * @param radius New value
 */
void Sphere::setRadius(fix10_6 radius)
{
	this->radius = __ABS(radius);
}

/**
 * Render
 */
void Sphere::render()
{
	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;
	Vector3D position = NULL != this->position ? *this->position : this->center;

	Vector3D relativePosition = Vector3D::sub(position, _previousCameraPosition);
	Sphere::setupRenderingMode(this, Vector3D::squareLength(relativePosition));

	this->normalizedCenter3D = Vector3D::rotate(Vector3D::getRelativeToCamera(this->center), *_cameraInvertedRotation);
}

/**
 * Draw
 */

/**
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Sphere::draw(bool calculateParallax)
{
	fix10_6 radiusSquare = __FIX10_6_MULT(this->radius, this->radius);

	Vector3D relativePoint3D =
	{
		// draw on XY plane
		-this->radius,
		0,
		this->normalizedCenter3D.z
	};

	for(; relativePoint3D.x < this->radius; relativePoint3D.x += __METERS_PER_PIXEL)
	{
		relativePoint3D.y = __F_TO_FIX10_6(Math::squareRoot(__FIX10_6_EXT_TO_F(radiusSquare - __FIX10_6_EXT_MULT(relativePoint3D.x, relativePoint3D.x))));
		Vector3D topTranslatedPoint3D = {this->normalizedCenter3D.x + relativePoint3D.x, this->normalizedCenter3D.y - relativePoint3D.y, this->normalizedCenter3D.z};
		Vector3D bottomTranslatedPoint3D = {this->normalizedCenter3D.x + relativePoint3D.x, this->normalizedCenter3D.y + relativePoint3D.y, this->normalizedCenter3D.z};

		int16 parallax = calculateParallax ? Optics::calculateParallax(relativePoint3D.z) : 0;
		PixelVector topPoint2D = Vector3D::projectToPixelVector(topTranslatedPoint3D, parallax);
		PixelVector bottomPoint2D = Vector3D::projectToPixelVector(bottomTranslatedPoint3D, parallax);

		DirectDraw::drawPoint(DirectDraw::getInstance(), topPoint2D, this->color);
		DirectDraw::drawPoint(DirectDraw::getInstance(), bottomPoint2D, this->color);
	}
/*
	// draw on YZ plane
	point3D.x = 0;
	point3D.y = 0;
	point3D.z = -this->radius;

	for(; point3D.z < this->radius; point3D.z += __I_TO_FIX10_6(1))
	{
		point3D.y = __F_TO_FIX10_6(Math::squareRoot(__FIX10_6_TO_F(radiusSquare - __FIX10_6_MULT(point3D.x, point3D.x) - __FIX10_6_MULT(point3D.z, point3D.z))));

		Vector3D translatedPoint3D = {point3D.x + normalizedCenter.x, point3D.y + normalizedCenter.y, point3D.z + normalizedCenter.z};
		PixelVector point2D = PixelVector::project(translatedPoint3D);

		if(calculateParallax)
		{
			point2D.parallax = Optics::calculateParallax(point3D.z);
		}

		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);

		point2D.y = -point3D.y + normalizedCenter.y;
		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);
	}

	// draw on XZ plane
	point3D.x = -this->radius;
	point3D.y = 0;
	point3D.z = 0;

	for(; point3D.x < this->radius; point3D.x += __I_TO_FIX10_6(1))
	{
		point3D.z = __F_TO_FIX10_6(Math::squareRoot(__FIX10_6_TO_F(radiusSquare - __FIX10_6_MULT(point3D.x, point3D.x) - __FIX10_6_MULT(point3D.z, point3D.z))));

		Vector3D translatedPoint3D = {point3D.x + normalizedCenter.x, point3D.y + normalizedCenter.y, point3D.z + normalizedCenter.z};
		PixelVector point2D = PixelVector::project(translatedPoint3D, 0);

		if(calculateParallax)
		{
			point2D.parallax = Optics::calculateParallax(point3D.z);
		}

		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);

		point2D.y = -point3D.y + normalizedCenter.y;
		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);
	}
*/
}
