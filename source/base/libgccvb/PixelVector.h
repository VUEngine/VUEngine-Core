/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PIXEL_VECTOR_H_
#define PIXEL_VECTOR_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class PixelVector : Object
{
	/// @publicsection
	static inline PixelVector zero();
	static inline PixelVector get(PixelVector from, PixelVector to);
	static inline PixelVector sum(PixelVector a, PixelVector b);
	static inline PixelVector sub(PixelVector a, PixelVector b);
	static inline PixelVector intermediate(PixelVector a, PixelVector b);
	static inline PixelVector getFromScreenPixelVector(ScreenPixelVector screenPixelVector, int16 parallax);
	static inline PixelVector getFromVector3D(Vector3D vector3D, int16 parallax);
	static inline uint32 squareLength(PixelVector vector);
	static inline fixed_t length(PixelVector vector);
	static inline PixelVector getRelativeToCamera(PixelVector vector);
	static inline PixelVector project(Vector3D vector3D, int16 parallax);
	static inline PixelVector getProjectionDisplacementHighPrecision(Vector3D vector3D, int16 parallax);
	static inline PixelVector projectHighPrecision(Vector3D vector3D, int16 parallax);
	static void print(PixelVector vector, int32 x, int32 y);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline PixelVector PixelVector::zero()
{
	return (PixelVector){0, 0, 0, 0};
}

static inline PixelVector PixelVector::get(PixelVector from, PixelVector to)
{
	return (PixelVector){to.x - from.x, to.y - from.y, to.z - from.z, to.parallax - from.parallax};
}

static inline PixelVector PixelVector::sum(PixelVector a, PixelVector b)
{
	return (PixelVector){a.x + b.x, a.y + b.y, a.z + b.z, a.parallax + b.parallax};
}

static inline PixelVector PixelVector::sub(PixelVector a, PixelVector b)
{
	return (PixelVector){a.x - b.x, a.y - b.y, a.z - b.z, a.parallax - b.parallax};
}

static inline PixelVector PixelVector::intermediate(PixelVector a, PixelVector b)
{
	return (PixelVector)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1,
		(a.parallax + b.parallax) >> 1
	};
}

static inline PixelVector PixelVector::getFromScreenPixelVector(ScreenPixelVector screenPixelVector, int16 parallax)
{
	return (PixelVector)
	{
		screenPixelVector.x,
		screenPixelVector.y,
		screenPixelVector.z + screenPixelVector.zDisplacement,
		parallax
	};
}

static inline PixelVector PixelVector::getFromVector3D(Vector3D vector3D, int16 parallax)
{
	return (PixelVector)
	{
		__METERS_TO_PIXELS(vector3D.x),
		__METERS_TO_PIXELS(vector3D.y),
		__METERS_TO_PIXELS(vector3D.z),
		parallax
	};
}

static inline uint32 PixelVector::squareLength(PixelVector vector)
{
	return ((uint32)vector.x) * ((uint32)vector.x) + ((uint32)vector.y) * ((uint32)vector.y) + ((uint32)vector.z) * ((uint32)vector.z);
}

static inline fixed_t PixelVector::length(PixelVector vector)
{
	return __F_TO_FIXED(Math_squareRoot(PixelVector::squareLength(vector)));
}

static inline PixelVector PixelVector::getRelativeToCamera(PixelVector vector)
{
	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	vector.x -= cameraPosition.x;
	vector.y -= cameraPosition.y;
	vector.z -= cameraPosition.z;

	return vector;
}

static inline PixelVector PixelVector::project(Vector3D vector3D, int16 parallax)
{
	vector3D.x -= (__FIXED_MULT(vector3D.x - _optical->horizontalViewPointCenter, vector3D.z) >> _optical->maximumXViewDistancePower);	
	vector3D.y -= (__FIXED_MULT(vector3D.y - _optical->verticalViewPointCenter, vector3D.z) >> _optical->maximumYViewDistancePower);	
	
	PixelVector projection =
	{
		__METERS_TO_PIXELS(vector3D.x),
		__METERS_TO_PIXELS(vector3D.y),
		__METERS_TO_PIXELS(vector3D.z),
		parallax
	};

	return projection;
}

static inline PixelVector PixelVector::getProjectionDisplacementHighPrecision(Vector3D vector3D, int16 parallax)
{	
	PixelVector projection =
	{
		-__METERS_TO_PIXELS(__FIXED_EXT_MULT((fixed_ext_t)vector3D.x - _optical->horizontalViewPointCenter, (fixed_ext_t)vector3D.z) >> (_optical->maximumXViewDistancePower + __PIXELS_PER_METER_2_POWER)),
		-__METERS_TO_PIXELS(__FIXED_EXT_MULT((fixed_ext_t)vector3D.y - _optical->verticalViewPointCenter, (fixed_ext_t)vector3D.z) >> (_optical->maximumYViewDistancePower + __PIXELS_PER_METER_2_POWER)),
		0,
		parallax
	};

	return projection;
}

static inline PixelVector PixelVector::projectHighPrecision(Vector3D vector3D, int16 parallax)
{
	PixelVector displacement = PixelVector::getProjectionDisplacementHighPrecision(vector3D, 0);
	
	PixelVector pixelVector =
	{
		__METERS_TO_PIXELS(vector3D.x),
		__METERS_TO_PIXELS(vector3D.y),
		__METERS_TO_PIXELS(vector3D.z),
		parallax
	};

	return PixelVector::sum(pixelVector, displacement);
}


#endif
