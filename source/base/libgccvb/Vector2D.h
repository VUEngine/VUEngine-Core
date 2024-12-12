/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VECTOR_2D_H_
#define VECTOR_2D_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>
#include <Math.h>
#include <Optical.h>
#include <Optics.h>
#include <Camera.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Vector2D
///
/// Inherits from Object
///
/// Implements methods to operate on Vector2D structs.
/// @ingroup base-libgccvb
static class Vector2D : Object
{
	/// @publicsection

	/// Get a vector with all its members initialized to zero.
	/// @return Pixel vector with all its members initialized to zero
	static inline Vector2D zero();

	/// Get a unit vector on the provided axis.
	/// @param axis: Axis on which the vector should be (__X_AXIS, __Y_AXIS)
	/// @return Unit vector on the provided axis
	static inline Vector2D unit(uint16 axis);

	/// Normalize the provided vector.
	/// @param vector: Vector to normalize
	/// @return Normalized vector
	static inline Vector2D normalize(Vector2D vector);
	
	/// Compute the difference vector between two provided vectors.
	/// @param from: Start vector
	/// @param to: End vector
	/// @return Difference vector between the provided vectors
	static inline Vector2D get(Vector2D from, Vector2D to);

	/// Compute the addition vector between other two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Addition vector between the provided vectors
	static inline Vector2D sum(Vector2D a, Vector2D b);

	/// Compute the difference vector between two provided vectors.
	/// @param a: End vector
	/// @param b: Start vector
	/// @return Difference vector between the provided vectors
	static inline Vector2D sub(Vector2D a, Vector2D b);

	/// Compute the intermediate vector between two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Intermediate vector between the provided vectors
	static inline Vector2D intermediate(Vector2D a, Vector2D b);

	/// Compute the perpendicular vector towards the provided direction.
	/// @param a: Vector to compute the perpendicular of
	/// @param left: Direction of the perpendicular vector
	/// @return Perpendicular vector
	static inline Vector2D perpendicular(Vector2D a, bool left);

	/// @param vector: Vector to scale
	/// @param scale: Scale to apply to the vector's components
	/// @return Scaled vector
	static inline Vector2D scale(Vector2D vector, Scale scale);

	/// Apply a scalar product over the vector's components
	/// @param rotation: Vector to scale
	/// @param scalar: Scalar to multiply
	/// @return Scaled vector
	static inline Vector2D scalarProduct(Vector2D vector, fixed_t scalar);

	/// Apply a scalar division over the vector's components
	/// @param rotation: Vector to scale
	/// @param scalar: Scalar divisor
	/// @return Scaled vector
	static inline Vector2D scalarDivision(Vector2D vector, fixed_t scalar);

	/// Compute the vector relative to the camera's position.
	/// @param vector: Vector to compute the relative vector of
	/// @return Vector relative to the camera's position
	static inline Vector2D getRelativeToCamera(Vector2D vector);

	/// Project the point p onto the vector ab.
	/// @param p: Vector point to project
	/// @param a: First vector point of the vector onto which to project p
	/// @param b: Second vector point of the vector onto which to project p
	/// @return Projected vector point
	static inline Vector2D projectOnto(Vector2D p, Vector2D a, Vector2D b);

	/// Project the point p onto the vector ab with higher decima precision.
	/// @param p: Vector point to project
	/// @param a: First vector point of the vector onto which to project p
	/// @param b: Second vector point of the vector onto which to project p
	/// @return Projected vector point
	static inline Vector2D projectOntoHighPrecision(Vector2D p, Vector2D a, Vector2D b);

	/// Rotate the vector according to the provided degrees.
	/// @param vector: Vector to rotate
	/// @param degrees: Amount of degrees to rotate the vector
	/// @return Rotated vector
	static inline Vector2D rotate(Vector2D vector, int16 degrees);

	/// Reduce the provided 3D vector to 2D by discarding the Z coordinate
	/// @param vector3D: 3D vector to reduce
	/// @return Reduced vector
	static inline Vector2D getFromVector3D(Vector3D vector3D);

	/// Transform the provided vector in pixel coordinates into a normal 2D vector.
	/// @param pixelVector: Vector to transform
	/// @return 2D vector
	static inline Vector2D getFromPixelVector(PixelVector screenVector);

	/// Transform the provided vector in screen coordinates into a normal 2D vector.
	/// @param screenPixelVector: Vector to transform
	/// @return 2D vector
	static inline Vector2D getFromScreenPixelVector(ScreenPixelVector screenPixelVector);

	/// Compute the length of the provided vector.
	/// @param vector: Vector to compute the length of
	/// @return Length of the provided vector
	static inline fixed_t length(Vector2D vector);

	/// Compute the squared length of the provided vector.
	/// @param vector: Vector to compute the square length of
	/// @return Square length of the provided vector
	static inline fixed_ext_t squareLength(Vector2D vector);

	/// Compute the product of the lengths of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Product of the lengths of the provided vectors
	static inline fixed_t lengthProduct(Vector2D vectorA, Vector2D vectorB);

	/// Compute the dot product of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Dot product of the provided vectors
	static inline fixed_ext_t dotProduct(Vector2D vectorA, Vector2D vectorB);

	/// Compute with higher decimal precision the dot product of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Dot product of the provided vectors
	static inline fix19_13 dotProductHighPrecision(Vector2D vectorA, Vector2D vectorB);

	/// Compute the scale factor for the provided z coordinate.
	/// @param z: Z coordinate
	/// @param applyScalingMultiplier: Flag to determine if the optics' scaling multiplier is applied to the result
	/// @return Computed scale
	static inline fixed_t getScale(fixed_t z, bool applyScalingMultiplier);

	/// Test if the vector point b is to the left of the vector ab.
	/// @param a: First vector
	/// @param b: Second vector
	/// @param p: Vector point to test
	/// @return True if p is to the left of ab; false otherwise
	static inline bool isLeft(Vector2D a, Vector2D b, Vector2D p);
	
	/// Test if the vector point b is to the right of the vector ab.
	/// @param a: First vector
	/// @param b: Second vector
	/// @param p: Vector point to test
	/// @return True if p is to the right of ab; false otherwise
	static inline bool isRight(Vector2D a, Vector2D b, Vector2D p);
	
	/// Test if two vectors are equal.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return True if all the components of both vectors are equal; false otherwise
	static inline bool areEqual(Vector2D a, Vector2D b);
	
	/// Test if a number is within a range.
	/// @param value: Number to test
	/// @param limitA: First limit of the range
	/// @param limitB: Second limit of the range
	/// @return True if all the provided number is within the provided limits
	static inline bool isValueInRange(fixed_t number, fixed_t limitA, fixed_t limitB);

	/// Test if vector point p is within the vector ab
	/// @param p: Vector point to test
	/// @param a: First vector point of the vector onto which to test p
	/// @param b: Second vector point of the vector onto which to test p
	/// @return True if p lies in ab; false otherwise
	static inline bool isVectorInsideLine(Vector2D p, Vector2D a, Vector2D b);

	/// Print the vector's components.
	/// @param vector: Vector to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(Vector2D vector, int32 x, int32 y);

	/// Print the vector's components without converting the underlying data type.
	/// @param vector: Vector to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void printRaw(Vector2D vector, int32 x, int32 y);
}

//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::zero()
{
	return (Vector2D){0, 0};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::unit(uint16 axis)
{
	return (Vector2D)
	{
		__X_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Y_AXIS & axis ? __I_TO_FIXED(1) : 0
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::normalize(Vector2D vector)
{
	return Vector2D::scalarDivision(vector, Vector2D::length(vector));
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::get(Vector2D from, Vector2D to)
{
	return (Vector2D){to.x - from.x, to.y - from.y};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::sum(Vector2D a, Vector2D b)
{
	return (Vector2D){a.x + b.x, a.y + b.y};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::sub(Vector2D a, Vector2D b)
{
	return (Vector2D){a.x - b.x, a.y - b.y};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::intermediate(Vector2D a, Vector2D b)
{
	return (Vector2D)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::perpendicular(Vector2D a, bool left)
{
	if(left)
	{
		fixed_t aux = a.x;
		a.x = -a.y;
		a.y = aux;
	}
	else
	{
		fixed_t aux = a.x;
		a.x = a.y;
		a.y = -aux;
	}

	return a;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::scale(Vector2D vector, Scale scale)
{
	return (Vector2D){__FIXED_EXT_MULT(vector.x, __FIX7_9_TO_FIXED(scale.x)), __FIXED_EXT_MULT(vector.y, __FIX7_9_TO_FIXED(scale.y))};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::scalarProduct(Vector2D vector, fixed_t scalar)
{
	return (Vector2D){__FIXED_MULT(vector.x, scalar), __FIXED_MULT(vector.y, scalar)};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::scalarDivision(Vector2D vector, fixed_t scalar)
{
	if(0 == scalar)
	{
		return Vector2D::zero();
	}

	return (Vector2D){__FIXED_DIV(vector.x, scalar), __FIXED_DIV(vector.y, scalar)};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::getRelativeToCamera(Vector2D vector)
{
	vector.x -= _cameraPosition->x;
	vector.y -= _cameraPosition->y;

	return vector;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::projectOnto(Vector2D p, Vector2D a, Vector2D b)
{
	Vector2D ap = Vector2D::get(a, p);
	Vector2D ab = Vector2D::get(a, b);
	fixed_ext_t dotApAb = Vector2D::dotProduct(ap, ab);
	fixed_ext_t dotAbAb = Vector2D::dotProduct(ab, ab);

	if(!dotAbAb)
	{
		if(a.x == b.x)
		{
			p.x = a.x;
		}

		if(a.y == b.y)
		{
			p.y = a.y;
		}

		return p;
	}

	Vector2D projection =
	{
		a.x + __FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(__FIXED_TO_FIXED_EXT(ab.x), __FIXED_EXT_DIV(dotApAb, dotAbAb))),
		a.y + __FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(__FIXED_TO_FIXED_EXT(ab.y), __FIXED_EXT_DIV(dotApAb, dotAbAb)))
	};

	return projection;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::projectOntoHighPrecision(Vector2D p, Vector2D a, Vector2D b)
{
	Vector2D ap = Vector2D::get(a, p);
	Vector2D ab = Vector2D::get(a, b);
	fix19_13 dotApAb = Vector2D::dotProductHighPrecision(ap, ab);
	fix19_13 dotAbAb = Vector2D::dotProductHighPrecision(ab, ab);

	if(!dotAbAb)
	{
		if(a.x == b.x)
		{
			p.x = a.x;
		}

		if(a.y == b.y)
		{
			p.y = a.y;
		}

		return p;
	}

	Vector2D projection =
	{
		a.x + __FIX19_13_TO_FIXED(__FIX19_13_MULT(__FIXED_TO_FIX19_13(ab.x), __FIX19_13_DIV(dotApAb, dotAbAb))),
		a.y + __FIX19_13_TO_FIXED(__FIX19_13_MULT(__FIXED_TO_FIX19_13(ab.y), __FIX19_13_DIV(dotApAb, dotAbAb)))
	};

	return projection;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::rotate(Vector2D vector, int16 degrees)
{
	Vector2D result = vector;

	if(0 != degrees)
	{
		fix7_9_ext cos = __FIX7_9_TO_FIX7_9_EXT(__COS(degrees));
		fix7_9_ext sin = __FIX7_9_TO_FIX7_9_EXT(__SIN(degrees));

		fix7_9_ext x = __FIXED_TO_FIX7_9_EXT(vector.x);
		fix7_9_ext y = __FIXED_TO_FIX7_9_EXT(vector.y);

		result = (Vector2D) 
		{
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(x , cos) - __FIX7_9_EXT_MULT(y , sin)),
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(x , sin) + __FIX7_9_EXT_MULT(y , cos)),
		};
	}

	return result;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::getFromVector3D(Vector3D vector3D)
{
	return (Vector2D){vector3D.x, vector3D.y};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::getFromPixelVector(PixelVector pixelVector)
{
	return (Vector2D)
	{
		__PIXELS_TO_METERS(pixelVector.x),
		__PIXELS_TO_METERS(pixelVector.y)
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector2D Vector2D::getFromScreenPixelVector(ScreenPixelVector screenPixelVector)
{
	return (Vector2D)
	{
		__PIXELS_TO_METERS(screenPixelVector.x),
		__PIXELS_TO_METERS(screenPixelVector.y)
	};
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector2D::length(Vector2D vector)
{
	fixed_ext_t lengthSquare = __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y);

	return Math_squareRootFixed(lengthSquare);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_ext_t Vector2D::squareLength(Vector2D vector)
{
	return __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector2D::lengthProduct(Vector2D vectorA, Vector2D vectorB)
{
	fixed_ext_t lengthSquareA = __FIXED_EXT_MULT(vectorA.x, vectorA.x) + __FIXED_EXT_MULT(vectorA.y, vectorA.y);
	fixed_ext_t lengthSquareB = __FIXED_EXT_MULT(vectorB.x, vectorB.x) + __FIXED_EXT_MULT(vectorB.y, vectorB.y);
	
	fixed_ext_t product = __FIXED_EXT_MULT(lengthSquareA, lengthSquareB);

	return Math_squareRootFixed(product);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_ext_t Vector2D::dotProduct(Vector2D vectorA, Vector2D vectorB)
{
	return __FIXED_EXT_MULT(vectorA.x, vectorB.x) + __FIXED_EXT_MULT(vectorA.y, vectorB.y);
}
//---------------------------------------------------------------------------------------------------------
static inline fix19_13 Vector2D::dotProductHighPrecision(Vector2D vectorA, Vector2D vectorB)
{
	return __FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.x), __FIXED_TO_FIX19_13(vectorB.x)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.y), __FIXED_TO_FIX19_13(vectorB.y));
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector2D::getScale(fixed_t z, bool applyScalingMultiplier)
{
	if(0 == _optical->halfWidth)
	{
		return __1I_FIXED;
	}

	fixed_ext_t projectedWidth = 0;

	if(applyScalingMultiplier)
	{
		if(0 == z + _optical->halfWidth)
		{
			return __1I_FIXED;
		}

		projectedWidth = __FIXED_EXT_DIV(__FIXED_EXT_MULT(_optical->halfWidth, _optical->scalingMultiplier), z + _optical->halfWidth);
	}
	else
	{
		if(0 == z + _optical->cameraNearPlane)
		{
			return __1I_FIXED;
		}

		projectedWidth = __FIXED_EXT_DIV(__FIXED_EXT_MULT(_optical->halfWidth, _optical->projectionMultiplierHelper), z + _optical->cameraNearPlane);
	}

	return __FIXED_EXT_DIV(projectedWidth, _optical->halfWidth);
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector2D::isLeft(Vector2D a, Vector2D b, Vector2D p)
{
	return 0 < (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector2D::isRight(Vector2D a, Vector2D b, Vector2D p)
{
	return 0 > (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector2D::areEqual(Vector2D a, Vector2D b)
{
	return a.x == b.x && a.y == b.y;
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector2D::isValueInRange(fixed_t value, fixed_t limitA, fixed_t limitB)
{
	if(limitA < limitB)
	{
		return (unsigned)(value - limitA) <= (unsigned)(limitB - limitA);
	}

	return (unsigned)(value - limitB) <= (unsigned)(limitA - limitB);
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector2D::isVectorInsideLine(Vector2D vector, Vector2D lineStart, Vector2D lineEnd)
{
	return (Vector2D::isValueInRange(vector.x, lineStart.x, lineEnd.x)
		&&
		Vector2D::isValueInRange(vector.y, lineStart.y, lineEnd.y)
	);
}
//---------------------------------------------------------------------------------------------------------


#endif
