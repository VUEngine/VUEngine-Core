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

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>


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
	static inline PixelVector getFromScreenPixelVector(ScreenPixelVector screenPixelVector, int16 parallax);
	static inline PixelVector getFromVector3D(Vector3D vector3D, int16 parallax);
	static inline uint32 squareLength(PixelVector vector);
	static inline fix10_6 length(PixelVector vector);
	static inline PixelVector getRelativeToCamera(PixelVector vector);
	static inline PixelVector project(Vector3D vector3D, int16 parallax);
	static inline PixelVector projectHighPrecision(Vector3D vector3D, int16 parallax);
	static inline void print(PixelVector vector, int32 x, int32 y);
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

static inline fix10_6 PixelVector::length(PixelVector vector)
{
	return __F_TO_FIX10_6(Math_squareRoot(PixelVector::squareLength(vector)));
}

static inline PixelVector PixelVector::getRelativeToCamera(PixelVector vector)
{
	extern const Vector3D* _cameraPosition;
	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	vector.x -= cameraPosition.x;
	vector.y -= cameraPosition.y;
	vector.z -= cameraPosition.z;

	return vector;
}

static inline PixelVector PixelVector::project(Vector3D vector3D, int16 parallax)
{
	extern const Optical* _optical;

	vector3D.x -= (__FIX10_6_MULT(vector3D.x - _optical->horizontalViewPointCenter, vector3D.z) >> _optical->maximumXViewDistancePower);	
	vector3D.y -= (__FIX10_6_MULT(vector3D.y - _optical->verticalViewPointCenter, vector3D.z) >> _optical->maximumYViewDistancePower);	
	
	PixelVector projection =
	{
		__METERS_TO_PIXELS(vector3D.x),
		__METERS_TO_PIXELS(vector3D.y),
		__METERS_TO_PIXELS(vector3D.z),
		parallax
	};

	return projection;
}

static inline PixelVector PixelVector::projectHighPrecision(Vector3D vector3D, int16 parallax)
{
	extern const Optical* _optical;

	fix10_6_ext x = (fix10_6_ext)(vector3D.x);
	fix10_6_ext y = (fix10_6_ext)(vector3D.y);
	fix10_6_ext z = (fix10_6_ext)(vector3D.z);

	x -= (__FIX10_6_EXT_MULT(x - _optical->horizontalViewPointCenter, z) >> _optical->maximumXViewDistancePower);	
	y -= (__FIX10_6_EXT_MULT(y - _optical->verticalViewPointCenter, z) >> _optical->maximumYViewDistancePower);	
	
	PixelVector projection =
	{
		__METERS_TO_PIXELS(x),
		__METERS_TO_PIXELS(y),
		__METERS_TO_PIXELS(z),
		parallax
	};

	return projection;
}

static inline void PixelVector::print(PixelVector vector, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);
	PRINT_TEXT("p:    ", x, y + 3);

	PRINT_INT(vector.x, x + 2, y);
	PRINT_INT(vector.y, x + 2, y + 1);
	PRINT_INT(vector.z, x + 2, y + 2);
	PRINT_INT(vector.parallax, x + 2, y + 3);
}


#endif
