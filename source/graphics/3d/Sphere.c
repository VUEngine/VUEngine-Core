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
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Sphere
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(Sphere, Wireframe);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
static void Sphere_constructor(Sphere this, Vector3D center, fix10_6 radius);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(Sphere, Vector3D center, fix10_6 radius)
__CLASS_NEW_END(Sphere, center, radius);

/**
 * Class constructor
 *
 * @memberof	Sphere
 * @private
 *
 * @param this	Function scope
 */
static void Sphere_constructor(Sphere this, Vector3D center, fix10_6 radius)
{
	ASSERT(this, "Sphere::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Wireframe);

	this->center = center;
	this->radius = __ABS(radius);
}

/**
 * Class destructor
 *
 * @memberof	Sphere
 * @public
 *
 * @param this	Function scope
 */
void Sphere_destructor(Sphere this)
{
	ASSERT(this, "Sphere::destructor: null this");

	Wireframe_hide(__SAFE_CAST(Wireframe, this));

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Retrieve center
 *
 * @memberof					Sphere
 * @public
 *
 * @param this					Function scope
 * @return 						Sphere's center
 */
Vector3D Sphere_getCenter(Sphere this)
{
	ASSERT(this, "Sphere::getCenter: null this");

	return this->center;
}

/**
 * Set center
 *
 * @memberof					Sphere
 * @public
 *
 * @param this					Function scope
 * @param center 				New value
 */
void Sphere_setCenter(Sphere this, Vector3D center)
{
	ASSERT(this, "Sphere::destructor: null this");

	this->center = center;
}

/**
 * Retrieve radius
 *
 * @memberof					Sphere
 * @public
 *
 * @param this					Function scope
 * @return 						Sphere's radius
 */
fix10_6 Sphere_getRadius(Sphere this)
{
	ASSERT(this, "Sphere::destructor: null this");

	return this->radius;
}

/**
 * Set radius
 *
 * @memberof					Sphere
 * @public
 *
 * @param this					Function scope
 * @param radius 				New value
 */
void Sphere_setRadius(Sphere this, fix10_6 radius)
{
	ASSERT(this, "Sphere::destructor: null this");

	this->radius = __ABS(radius);
}

/**
 * Write to the frame buffers
 *
 * @memberof					Sphere
 * @public
 *
 * @param this					Function scope
 * @param this					Function scope
 * @param calculateParallax		Tru to compute the parallax displacement for each pixel
 */
void Sphere_draw(Sphere this, bool calculateParallax)
{
	ASSERT(this, "Sphere::draw: null this");

	int color = __COLOR_BRIGHT_RED;

	Vector3D normalizedCenter = this->center;
	normalizedCenter = Vector3D_getRelativeToCamera(normalizedCenter);
	PixelVector normalizedCenter2D = Vector3D_projectToPixelVector(normalizedCenter, 0);

	fix10_6 radiusSquare = __FIX10_6_MULT(this->radius, this->radius);

	Vector3D point3D;

	// draw on XY plane
	point3D.x = -this->radius;
	point3D.y = 0;
	point3D.z = 0;

	for(; point3D.x < this->radius; point3D.x += __METERS_PER_PIXEL)
	{
		point3D.y = __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(radiusSquare - __FIX10_6_MULT(point3D.x, point3D.x) - __FIX10_6_MULT(point3D.z, point3D.z))));

		Vector3D translatedPoint3D = {point3D.x, point3D.y, point3D.z};
		PixelVector relativePoint2D = Vector3D_projectToPixelVector(translatedPoint3D, 0);

		if(calculateParallax)
		{
			relativePoint2D.parallax = Optics_calculateParallax(point3D.x, point3D.z);
		}

		{
			PixelVector absolutePoint2D =
			{
				normalizedCenter2D.x + relativePoint2D.x,
				normalizedCenter2D.y + relativePoint2D.y,
				normalizedCenter2D.z + relativePoint2D.z,
				relativePoint2D.parallax,
			};

			DirectDraw_drawPoint(DirectDraw_getInstance(), absolutePoint2D, color);
		}

		{
			PixelVector absolutePoint2D =
			{
				normalizedCenter2D.x + relativePoint2D.x,
				normalizedCenter2D.y - relativePoint2D.y,
				normalizedCenter2D.z + relativePoint2D.z,
				relativePoint2D.parallax,
			};

			DirectDraw_drawPoint(DirectDraw_getInstance(), absolutePoint2D, color);
		}
	}
/*
	// draw on YZ plane
	point3D.x = 0;
	point3D.y = 0;
	point3D.z = -this->radius;

	for(; point3D.z < this->radius; point3D.z += __I_TO_FIX10_6(1))
	{
		point3D.y = __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(radiusSquare - __FIX10_6_MULT(point3D.x, point3D.x) - __FIX10_6_MULT(point3D.z, point3D.z))));

		Vector3D translatedPoint3D = {point3D.x + normalizedCenter.x, point3D.y + normalizedCenter.y, point3D.z + normalizedCenter.z};
		PixelVector point2D = Vector3D_projectToPixelVector(translatedPoint3D);

		if(calculateParallax)
		{
			point2D.parallax = Optics_calculateParallax(point3D.x, point3D.z);
		}

		DirectDraw_drawPoint(DirectDraw_getInstance(), point2D, color);

		point2D.y = -point3D.y + normalizedCenter.y;
		DirectDraw_drawPoint(DirectDraw_getInstance(), point2D, color);
	}

	// draw on XZ plane
	point3D.x = -this->radius;
	point3D.y = 0;
	point3D.z = 0;

	for(; point3D.x < this->radius; point3D.x += __I_TO_FIX10_6(1))
	{
		point3D.z = __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(radiusSquare - __FIX10_6_MULT(point3D.x, point3D.x) - __FIX10_6_MULT(point3D.z, point3D.z))));

		Vector3D translatedPoint3D = {point3D.x + normalizedCenter.x, point3D.y + normalizedCenter.y, point3D.z + normalizedCenter.z};
		PixelVector point2D = Vector3D_projectToPixelVector(translatedPoint3D, 0);

		if(calculateParallax)
		{
			point2D.parallax = Optics_calculateParallax(point3D.x, point3D.z);
		}

		DirectDraw_drawPoint(DirectDraw_getInstance(), point2D, color);

		point2D.y = -point3D.y + normalizedCenter.y;
		DirectDraw_drawPoint(DirectDraw_getInstance(), point2D, color);
	}
*/
}
