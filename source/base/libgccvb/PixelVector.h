/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PIXEL_VECTOR_H_
#define PIXEL_VECTOR_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>
#include <Camera.h>
#include <Vector3D.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class PixelVector
///
/// Inherits from Object
///
/// Implements methods to operate on PixelVector structs.
/// @ingroup base-libgccvb
static class PixelVector : Object
{
	/// @publicsection

	/// Get a vector with all its members initialized to zero.
	/// @return Pixel vector with all its members initialized to zero
	static inline PixelVector zero();

	/// Get the difference vector between two provided vectors.
	/// @param from: Start vector
	/// @param to: End vector
	/// @return Difference vector between the provided vectors
	static inline PixelVector get(PixelVector from, PixelVector to);

	/// Compute the addition vector between other two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Addition vector between the provided vectors
	static inline PixelVector sum(PixelVector a, PixelVector b);

	/// Compute the difference vector between two provided vectors.
	/// @param a: End vector
	/// @param b: Start vector
	/// @return Difference vector between the provided vectors
	static inline PixelVector sub(PixelVector a, PixelVector b);

	/// Compute the intermediate vector between two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Intermediate vector between the provided vectors
	static inline PixelVector intermediate(PixelVector a, PixelVector b);

	/// Transform the provided vector in screen coordinates into a normal pixel vector.
	/// @param screenPixelVector: Vector to transform
	/// @param parallax: Parallax component for the resulting vector
	/// @return Vector in pixel units
	static inline PixelVector getFromScreenPixelVector(ScreenPixelVector screenPixelVector, int16 parallax);

	/// Transform the provided 2D vector in meter coordinates into a normal pixel vector.
	/// @param vector2D: 2D vector to transform
	/// @param parallax: Parallax component for the resulting vector
	/// @return Vector in pixel units
	static inline PixelVector getFromVector2D(Vector2D vector2D, int16 parallax);

	/// Transform the provided 2D vector in meter coordinates into a normal pixel vector.
	/// @param vector3D: 3D vector to transform
	/// @param parallax: Parallax component for the resulting vector
	/// @return Vector in pixel units
	static inline PixelVector getFromVector3D(Vector3D vector3D, int16 parallax);

	/// Compute the vector relative to the camera's position.
	/// @param vector: Vector to compute the relative vector of
	/// @return Vector relative to the camera's position
	static inline PixelVector getRelativeToCamera(PixelVector vector);

	/// Project the 3D vector to a vector in pixel units.
	/// @param vector: 3D vector to project
	/// @param parallax: Parallax component for the resulting vector
	/// @return Projected vector in pixel units
	static inline PixelVector projectVector3D(Vector3D vector3D, int16 parallax);

	/// Transform to camera space and project the 3D vector to a vector in pixel units.
	/// @param vector: 3D vector to transform
	/// @param parallax: Parallax component for the resulting vector
	/// @return Projected vector in pixel units
	static inline PixelVector transformVector3D(Vector3D vector);

	/// Compute the length of the provided vector.
	/// @param vector: Vector to compute the length of
	/// @return Length of the provided vector
	static inline fixed_t length(PixelVector vector);

	/// Compute the squared length of the provided vector.
	/// @param vector: Vector to compute the square length of
	/// @return Square length of the provided vector
	static inline uint32 squareLength(PixelVector vector);

	/// Test if the provided vector is within the provided right box
	/// @param vector: Vector to test
	/// @param pixelRightBox: Bounding box
	/// @param padding: Padding to be added to the bounding box
	/// @return True if the part of the vector is inside the bounding box; false otherwise
	static inline bool isVisible(PixelVector vector, PixelRightBox pixelRightBox, int16 padding);

	/// Test if the provided 3D vector is within the provided right box
	/// @param vector: 3D vector to test
	/// @param pixelRightBox: Bounding box
	/// @param padding: Padding to be added to the bounding box
	/// @return True if the part of the vector is inside the bounding box; false otherwise
	static inline bool isVector3DVisible(Vector3D vector, PixelRightBox pixelRightBox, int16 padding);
	
	/// Print the vector's components.
	/// @param vector: Vector to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(PixelVector vector, int32 x, int32 y);
}

//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::zero()
{
	return (PixelVector){0, 0, 0, 0};
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::get(PixelVector from, PixelVector to)
{
	return (PixelVector){to.x - from.x, to.y - from.y, to.z - from.z, to.parallax - from.parallax};
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::sum(PixelVector a, PixelVector b)
{
	return (PixelVector){a.x + b.x, a.y + b.y, a.z + b.z, a.parallax + b.parallax};
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::sub(PixelVector a, PixelVector b)
{
	return (PixelVector){a.x - b.x, a.y - b.y, a.z - b.z, a.parallax - b.parallax};
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::getFromScreenPixelVector(ScreenPixelVector screenPixelVector, int16 parallax)
{
	return (PixelVector)
	{
		screenPixelVector.x,
		screenPixelVector.y,
		screenPixelVector.z,
		parallax
	};
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::getFromVector2D(Vector2D vector2D, int16 parallax)
{
	return (PixelVector)
	{
		__METERS_TO_PIXELS(vector2D.x),
		__METERS_TO_PIXELS(vector2D.y),
		0,
		parallax
	};
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::getRelativeToCamera(PixelVector vector)
{
	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	vector.x -= cameraPosition.x;
	vector.y -= cameraPosition.y;
	vector.z -= cameraPosition.z;

	return vector;
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::projectVector3D(Vector3D vector3D, int16 parallax)
{
	fixed_ext_t x = (fixed_ext_t)(vector3D.x);
	fixed_ext_t y = (fixed_ext_t)(vector3D.y);
	fixed_ext_t z = (fixed_ext_t)(vector3D.z);

#ifdef __LEGACY_COORDINATE_PROJECTION
	if(0 != z)
	{
		x -= (__FIXED_EXT_MULT(x - _optical->horizontalViewPointCenter, z) >> _optical->maximumXViewDistancePower);	
		y -= (__FIXED_EXT_MULT(y - _optical->verticalViewPointCenter, z) >> _optical->maximumYViewDistancePower);	
	}
#else
	if(0 == z + _optical->cameraNearPlane)
	{
		x = 0 > x ? -(1 << 15) : (1 << 15);	
		y = 0 > y ? -(1 << 15) : (1 << 15);	
	}
	else
	{
		/*
		// Mathematically correct version
		// but produces distorted results on the y axis
		// x = x * aspect ratio * fov
		x = __FIXED_EXT_MULT(x, _optical->aspectRatioXfov);
		// y = y * fov
		// since fov = 1 because it is assumed and angle of 90, there is no
		// need to make the computation
		y = __FIXED_EXT_MULT(y, _optical->fov);
		// z = z * (far + near) / (far - near) + (2 * far * near) / (near - far)
		// since the near plane will always be 0, there is no need to perform 
		// this product		
		z = __FIXED_EXT_MULT(z, _optical->farRatio1Near) + _optical->farRatio2Near;

		x = __FIXED_EXT_DIV(__FIXED_EXT_MULT(x, _optical->halfHeight), z) + _optical->horizontalViewPointCenter;	
		y = __FIXED_EXT_DIV(__FIXED_EXT_MULT(y, _optical->halfWidth), z) + _optical->verticalViewPointCenter;
		*/

		// Fast and produces the expected result
		// x = x * aspect ratio * fov

		// to reduce from 4 products and 2 divisions to 3 products, 1 division and 3 bit shifts
		fixed_ext_t factor = __FIXED_EXT_DIV(_optical->projectionMultiplierHelper, z + _optical->cameraNearPlane);

		x = (__FIXED_EXT_MULT(x, factor)) + _optical->horizontalViewPointCenter;	
		y = (__FIXED_EXT_MULT(y, factor)) + _optical->verticalViewPointCenter;
	}
#endif

	PixelVector projection =
	{
		__METERS_TO_PIXELS(x),
		__METERS_TO_PIXELS(y),
		__METERS_TO_PIXELS(z),
		parallax
	};

	return projection;
}
//---------------------------------------------------------------------------------------------------------
static inline PixelVector PixelVector::transformVector3D(Vector3D vector)
{
	vector = Vector3D::rotate(Vector3D::sub(vector, *_cameraPosition), *_cameraInvertedRotation);
	
	return PixelVector::projectVector3D(vector, Optics::calculateParallax(vector.z));
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t PixelVector::length(PixelVector vector)
{
	return __F_TO_FIXED(Math_squareRoot(PixelVector::squareLength(vector)));
}
//---------------------------------------------------------------------------------------------------------
static inline uint32 PixelVector::squareLength(PixelVector vector)
{
	return ((uint32)vector.x) * ((uint32)vector.x) + ((uint32)vector.y) * ((uint32)vector.y) + ((uint32)vector.z) * ((uint32)vector.z);
}
//---------------------------------------------------------------------------------------------------------
static inline bool PixelVector::isVisible(PixelVector vector, PixelRightBox pixelRightBox, int16 padding)
{
	extern const CameraFrustum* _cameraFrustum;

#ifndef __LEGACY_COORDINATE_PROJECTION
	vector = PixelVector::sum(vector, (PixelVector){__HALF_SCREEN_WIDTH, __HALF_SCREEN_HEIGHT, 0, 0});
#endif

	if(vector.x + pixelRightBox.x0 > _cameraFrustum->x1 + padding || vector.x + pixelRightBox.x1 < _cameraFrustum->x0 - padding)
	{
		return false;
	}

	// check y visibility
	if(vector.y + pixelRightBox.y0 > _cameraFrustum->y1 + padding || vector.y + pixelRightBox.y1 < _cameraFrustum->y0 - padding)
	{
		return false;
	}

	// check z visibility
	if(vector.z + pixelRightBox.z0 > _cameraFrustum->z1 + padding || vector.z + pixelRightBox.z1 < _cameraFrustum->z0 - padding)
	{
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------
static inline bool PixelVector::isVector3DVisible(Vector3D vector, PixelRightBox pixelRightBox, int16 padding)
{
	extern const CameraFrustum* _cameraFrustum;
	vector = Vector3D::rotate(Vector3D::getRelativeToCamera(vector), *_cameraInvertedRotation);
	PixelVector pixelVector = PixelVector::projectVector3D(vector, 0);

	if(pixelVector.x + pixelRightBox.x0 > _cameraFrustum->x1 + padding || pixelVector.x + pixelRightBox.x1 < _cameraFrustum->x0 - padding)
	{
		return false;
	}

	// check y visibility
	if(pixelVector.y + pixelRightBox.y0 > _cameraFrustum->y1 + padding || pixelVector.y + pixelRightBox.y1 < _cameraFrustum->y0 - padding)
	{
		return false;
	}

	// check z visibility
	if(pixelVector.z + pixelRightBox.z0 > _cameraFrustum->z1 + padding || pixelVector.z + pixelRightBox.z1 < _cameraFrustum->z0 - padding)
	{
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------


#endif
