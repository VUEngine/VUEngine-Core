/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
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
void Sphere::constructor(Vector3D center, fix10_6 radius)
{
	// construct base object
	Base::constructor();

	this->center = center;
	this->radius = __ABS(radius);
}

/**
 * Class destructor
 */
void Sphere::destructor()
{
	Wireframe::hide(this);

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
 * Write to the frame buffers
 *
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Sphere::draw(bool calculateParallax)
{
	int color = __COLOR_BRIGHT_RED;

	Vector3D normalizedCenter3D = Vector3D::getRelativeToCamera(this->center);

	fix10_6 radiusSquare = __FIX10_6_MULT(this->radius, this->radius);

	Vector3D relativePoint3D =
	{
		// draw on XY plane
		-this->radius,
		0,
		this->center.z
	};

	for(; relativePoint3D.x < this->radius; relativePoint3D.x += __METERS_PER_PIXEL)
	{
		relativePoint3D.y = __F_TO_FIX10_6(Math::squareRoot(__FIX10_6_EXT_TO_F(radiusSquare - __FIX10_6_EXT_MULT(relativePoint3D.x, relativePoint3D.x))));
		Vector3D topTranslatedPoint3D = {normalizedCenter3D.x + relativePoint3D.x, normalizedCenter3D.y - relativePoint3D.y, normalizedCenter3D.z};
		Vector3D bottomTranslatedPoint3D = {normalizedCenter3D.x + relativePoint3D.x, normalizedCenter3D.y + relativePoint3D.y, normalizedCenter3D.z};

		s16 parallax = calculateParallax ? Optics::calculateParallax(relativePoint3D.x, relativePoint3D.z) : 0;
		PixelVector topPoint2D = Vector3D::projectToPixelVector(topTranslatedPoint3D, parallax);
		PixelVector bottomPoint2D = Vector3D::projectToPixelVector(bottomTranslatedPoint3D, parallax);

		DirectDraw::drawPoint(DirectDraw::getInstance(), topPoint2D, color);
		DirectDraw::drawPoint(DirectDraw::getInstance(), bottomPoint2D, color);
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
		PixelVector point2D = Vector3D::projectToPixelVector(translatedPoint3D);

		if(calculateParallax)
		{
			point2D.parallax = Optics::calculateParallax(point3D.x, point3D.z);
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
		PixelVector point2D = Vector3D::projectToPixelVector(translatedPoint3D, 0);

		if(calculateParallax)
		{
			point2D.parallax = Optics::calculateParallax(point3D.x, point3D.z);
		}

		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);

		point2D.y = -point3D.y + normalizedCenter.y;
		DirectDraw::drawPoint(DirectDraw::getInstance(), point2D, color);
	}
*/
}
