/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VECTOR_3D_H_
#define VECTOR_3D_H_

//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Camera.h>
#include <Math.h>
#include <Object.h>
#include <Optical.h>
#include <Optics.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Vector3D
///
/// Inherits from Object
///
/// Implements methods to operate on Vector3D structs.
/// @ingroup base-libgccvb
static class Vector3D : Object
{
	/// @publicsection

	/// Get a vector with all its members initialized to zero.
	/// @return Pixel vector with all its members initialized to zero
	static inline Vector3D zero();

	/// Get a unit vector on the provided axis.
	/// @param axis: Axis on which the vector should be (__X_AXIS, __Y_AXIS or __Z_AXIS)
	/// @return Unit vector on the provided axis
	static inline Vector3D unit(uint16 axis);

	/// Normalize the provided vector.
	/// @param vector: Vector to normalize
	/// @return Normalized vector
	static inline Vector3D normalize(Vector3D vector);

	/// Compute the difference vector between two provided vectors.
	/// @param from: Start vector
	/// @param to: End vector
	/// @return Difference vector between the provided vectors
	static inline Vector3D get(Vector3D from, Vector3D to);

	/// Compute the addition vector between other two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Addition vector between the provided vectors
	static inline Vector3D sum(Vector3D a, Vector3D b);

	/// Compute the difference vector between two provided vectors.
	/// @param a: End vector
	/// @param b: Start vector
	/// @return Difference vector between the provided vectors
	static inline Vector3D sub(Vector3D a, Vector3D b);

	/// Compute the intermediate vector between two provided vectors.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return Intermediate vector between the provided vectors
	static inline Vector3D intermediate(Vector3D a, Vector3D b);

	/// Compute the perpendicular vector to the X plane in the provided direction.
	/// @param a: Vector to compute the perpendicular of
	/// @param left: Direction of the perpendicular vector
	/// @return Perpendicular vector to the X plane
	static inline Vector3D perpendicularXPlane(Vector3D a, bool left);

	/// Compute the perpendicular vector to the Y plane in the provided direction.
	/// @param a: Vector to compute the perpendicular of
	/// @param left: Direction of the perpendicular vector
	/// @return Perpendicular vector to the Y plane
	static inline Vector3D perpendicularYPlane(Vector3D a, bool left);

	/// Compute the perpendicular vector to the Z plane in the provided direction.
	/// @param a: Vector to compute the perpendicular of
	/// @param left: Direction of the perpendicular vector
	/// @return Perpendicular vector to the Z plane
	static inline Vector3D perpendicularZPlane(Vector3D a, bool left);

	/// Scale the provided vector's components
	/// @param vector: Vector to scale
	/// @param scale: Scale to apply to the vector's components
	/// @return Scaled vector
	static inline Vector3D scale(Vector3D vector, Scale scale);

	/// Apply a scalar product over the vector's components
	/// @param rotation: Vector to scale
	/// @param scalar: Scalar to multiply
	/// @return Scaled vector
	static inline Vector3D scalarProduct(Vector3D vector, fixed_t scalar);

	/// Apply a scalar division over the vector's components
	/// @param rotation: Vector to scale
	/// @param scalar: Scalar divisor
	/// @return Scaled vector
	static inline Vector3D scalarDivision(Vector3D vector, fixed_t scalar);

	/// Compute the vector relative to the camera's position.
	/// @param vector: Vector to compute the relative vector of
	/// @return Vector relative to the camera's position
	static inline Vector3D getRelativeToCamera(Vector3D vector);

	/// Compute the normal to the plane defined by the provided vectors.
	/// @param vectorA: Common vector
	/// @param vectorB: Second vector
	/// @param vectorC: Third vector
	/// @return Normal vector to the plane defined by AB and AC
	static inline Vector3D getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC);

	/// Project the point p onto the vector ab.
	/// @param p: Vector point to project
	/// @param a: First vector point of the vector onto which to project p
	/// @param b: Second vector point of the vector onto which to project p
	/// @return Projected vector point
	static inline Vector3D projectOnto(Vector3D p, Vector3D a, Vector3D b);

	/// Project the point p onto the vector ab with higher decima precision.
	/// @param p: Vector point to project
	/// @param a: First vector point of the vector onto which to project p
	/// @param b: Second vector point of the vector onto which to project p
	/// @return Projected vector point
	static inline Vector3D projectOntoHighPrecision(Vector3D p, Vector3D a, Vector3D b);

	/// Rotate the vector according to the provided rotation.
	/// @param vector: Vector to rotate
	/// @param rotation: Rotation to apply to the vector
	/// @return Rotated vector
	static inline Vector3D rotate(Vector3D vector, Rotation rotation);

	/// Rotate the vector around the X axis.
	/// @param vector: Vector to rotate
	/// @param degrees: Amount of degrees to rotate the vector
	/// @return Rotated vector
	static inline Vector3D rotateXAxis(Vector3D vector, int16 degrees);

	/// Rotate the vector around the Y axis.
	/// @param vector: Vector to rotate
	/// @param degrees: Amount of degrees to rotate the vector
	/// @return Rotated vector
	static inline Vector3D rotateYAxis(Vector3D vector, int16 degrees);

	/// Rotate the vector around the Z axis.
	/// @param vector: Vector to rotate
	/// @param degrees: Amount of degrees to rotate the vector
	/// @return Rotated vector
	static inline Vector3D rotateZAxis(Vector3D vector, int16 degrees);

	/// Extend the provided 2D vector to 3D.
	/// @param vector2D: 2D vector to extend
	/// @param z: Z coordinate for the extended vector
	/// @return Extended vector
	static inline Vector3D getFromVector2D(Vector2D vector2D, fixed_t z);

	/// Transform the provided vector in pixel coordinates into a normal 3D vector.
	/// @param pixelVector: Vector to transform
	/// @return 3D vector
	static inline Vector3D getFromPixelVector(PixelVector pixelVector);

	/// Transform the provided vector in screen coordinates into a normal 3D vector.
	/// @param screenPixelVector: Vector to transform
	/// @return 3D vector
	static inline Vector3D getFromScreenPixelVector(ScreenPixelVector screenPixelVector);

	/// Compute the length of the provided vector.
	/// @param vector: Vector to compute the length of
	/// @return Length of the provided vector
	static inline fixed_t length(Vector3D vector);

	/// Compute the squared length of the provided vector.
	/// @param vector: Vector to compute the square length of
	/// @return Square length of the provided vector
	static inline fixed_ext_t squareLength(Vector3D vector);

	/// Compute the product of the lengths of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Product of the lengths of the provided vectors
	static inline fixed_t lengthProduct(Vector3D vectorA, Vector3D vectorB);

	/// Compute the dot product of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Dot product of the provided vectors
	static inline fixed_ext_t dotProduct(Vector3D vectorA, Vector3D vectorB);

	/// Compute with higher decimal precision the dot product of the provided vectors.
	/// @param vectorA: First vector
	/// @param vectorB: Second vector
	/// @return Dot product of the provided vectors
	static inline fix19_13 dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB);

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
	static inline bool isLeft(Vector3D a, Vector3D b, Vector3D p);
	
	/// Test if the vector point b is to the right of the vector ab.
	/// @param a: First vector
	/// @param b: Second vector
	/// @param p: Vector point to test
	/// @return True if p is to the right of ab; false otherwise
	static inline bool isRight(Vector3D a, Vector3D b, Vector3D p);
	
	/// Test if two vectors are equal.
	/// @param a: First vector
	/// @param b: Second vector
	/// @return True if all the components of both vectors are equal; false otherwise
	static inline bool areEqual(Vector3D a, Vector3D b);
	
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
	static inline bool isVectorInsideLine(Vector3D p, Vector3D a, Vector3D b);

	/// Print the vector's components.
	/// @param vector: Vector to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(Vector3D vector, int32 x, int32 y);

	/// Print the vector's components without converting the underlying data type.
	/// @param vector: Vector to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void printRaw(Vector3D vector, int32 x, int32 y);
}

//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::zero()
{
	return (Vector3D){0, 0, 0};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::unit(uint16 axis)
{
	return (Vector3D)
	{
		__X_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Y_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Z_AXIS & axis ? __I_TO_FIXED(1) : 0
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::normalize(Vector3D vector)
{
	return Vector3D::scalarDivision(vector, Vector3D::length(vector));
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::get(Vector3D from, Vector3D to)
{
	return (Vector3D){to.x - from.x, to.y - from.y, to.z - from.z};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::sum(Vector3D a, Vector3D b)
{
	return (Vector3D){a.x + b.x, a.y + b.y, a.z + b.z};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::sub(Vector3D a, Vector3D b)
{
	return (Vector3D){a.x - b.x, a.y - b.y, a.z - b.z};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::intermediate(Vector3D a, Vector3D b)
{
	return (Vector3D)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::perpendicularXPlane(Vector3D a, bool left)
{
	if(left)
	{
		fixed_t aux = a.y;
		a.y = -a.z;
		a.z = aux;
	}
	else
	{
		fixed_t aux = a.y;
		a.y = a.z;
		a.z = -aux;
	}

	return a;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::perpendicularYPlane(Vector3D a, bool left)
{
	if(left)
	{
		fixed_t aux = a.x;
		a.x = -a.z;
		a.z = aux;
	}
	else
	{
		fixed_t aux = a.x;
		a.x = a.z;
		a.z = -aux;
	}

	return a;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::perpendicularZPlane(Vector3D a, bool left)
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
static inline Vector3D Vector3D::scale(Vector3D vector, Scale scale)
{
	return (Vector3D){__FIXED_EXT_MULT(vector.x, __FIX7_9_TO_FIXED(scale.x)), __FIXED_EXT_MULT(vector.y, __FIX7_9_TO_FIXED(scale.y)), __FIXED_EXT_MULT(vector.z, __FIX7_9_TO_FIXED(scale.z))};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::scalarProduct(Vector3D vector, fixed_t scalar)
{
	return (Vector3D){__FIXED_MULT(vector.x, scalar), __FIXED_MULT(vector.y, scalar), __FIXED_MULT(vector.z, scalar)};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::scalarDivision(Vector3D vector, fixed_t scalar)
{
	if(0 == scalar)
	{
		return Vector3D::zero();
	}

	return (Vector3D){__FIXED_DIV(vector.x, scalar), __FIXED_DIV(vector.y, scalar), __FIXED_DIV(vector.z, scalar)};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::getRelativeToCamera(Vector3D vector)
{
	vector.x -= _cameraPosition->x;
	vector.y -= _cameraPosition->y;
	vector.z -= _cameraPosition->z;

	return vector;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC)
{
	Vector3D u =
	{
		vectorB.x - vectorA.x,
		vectorB.y - vectorA.y,
		vectorB.z - vectorA.z,
	};

	Vector3D v =
	{
		vectorC.x - vectorA.x,
		vectorC.y - vectorA.y,
		vectorC.z - vectorA.z,
	};

	return (Vector3D)
	{
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.y, v.z) - __FIXED_EXT_MULT(u.z, v.y)),
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.z, v.x) - __FIXED_EXT_MULT(u.x, v.z)),
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.x, v.y) - __FIXED_EXT_MULT(u.y, v.x)),
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::projectOnto(Vector3D p, Vector3D a, Vector3D b)
{
	Vector3D ap = Vector3D::get(a, p);
	Vector3D ab = Vector3D::get(a, b);
	fixed_ext_t dotApAb = Vector3D::dotProduct(ap, ab);
	fixed_ext_t dotAbAb = Vector3D::dotProduct(ab, ab);

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

		if(a.z == b.z)
		{
			p.z = a.z;
		}

		return p;
	}

	Vector3D projection =
	{
		a.x + __FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(__FIXED_TO_FIXED_EXT(ab.x), __FIXED_EXT_DIV(dotApAb, dotAbAb))),
		a.y + __FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(__FIXED_TO_FIXED_EXT(ab.y), __FIXED_EXT_DIV(dotApAb, dotAbAb))),
		a.z + __FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(__FIXED_TO_FIXED_EXT(ab.z), __FIXED_EXT_DIV(dotApAb, dotAbAb))),
	};

	return projection;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::projectOntoHighPrecision(Vector3D p, Vector3D a, Vector3D b)
{
	Vector3D ap = Vector3D::get(a, p);
	Vector3D ab = Vector3D::get(a, b);
	fix19_13 dotApAb = Vector3D::dotProductHighPrecision(ap, ab);
	fix19_13 dotAbAb = Vector3D::dotProductHighPrecision(ab, ab);

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

		if(a.z == b.z)
		{
			p.z = a.z;
		}

		return p;
	}

	Vector3D projection =
	{
		a.x + __FIX19_13_TO_FIXED(__FIX19_13_MULT(__FIXED_TO_FIX19_13(ab.x), __FIX19_13_DIV(dotApAb, dotAbAb))),
		a.y + __FIX19_13_TO_FIXED(__FIX19_13_MULT(__FIXED_TO_FIX19_13(ab.y), __FIX19_13_DIV(dotApAb, dotAbAb))),
		a.z + __FIX19_13_TO_FIXED(__FIX19_13_MULT(__FIXED_TO_FIX19_13(ab.z), __FIX19_13_DIV(dotApAb, dotAbAb))),
	};

	return projection;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::rotate(Vector3D vector, Rotation rotation)
{
	Vector3D result = vector;

	if(0 != rotation.x)
	{
		result = Vector3D::rotateXAxis(result, __FIXED_TO_I(rotation.x + __05F_FIXED));
	}

	if(0 != rotation.y)
	{
		result = Vector3D::rotateYAxis(result, __FIXED_TO_I(rotation.y + __05F_FIXED));
	}

	if(0 != rotation.z)
	{
		result = Vector3D::rotateZAxis(result, __FIXED_TO_I(rotation.z + __05F_FIXED));
	}

	return result;
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::rotateXAxis(Vector3D vector, int16 degrees)
{
	fix7_9_ext cos = __FIX7_9_TO_FIX7_9_EXT(__COS(degrees));
	fix7_9_ext sin = __FIX7_9_TO_FIX7_9_EXT(__SIN(degrees));

	fix7_9_ext y = __FIXED_TO_FIX7_9_EXT(vector.y);
	fix7_9_ext z = __FIXED_TO_FIX7_9_EXT(vector.z);

	return (Vector3D) 
		{
			vector.x,
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(y , cos) - __FIX7_9_EXT_MULT(z , sin)),
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(y , sin) + __FIX7_9_EXT_MULT(z , cos))
		};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::rotateYAxis(Vector3D vector, int16 degrees)
{
	fix7_9_ext cos = __FIX7_9_TO_FIX7_9_EXT(__COS(degrees));
	fix7_9_ext sin = __FIX7_9_TO_FIX7_9_EXT(__SIN(degrees));

	fix7_9_ext x = __FIXED_TO_FIX7_9_EXT(vector.x);
	fix7_9_ext z = __FIXED_TO_FIX7_9_EXT(vector.z);

	return (Vector3D) 
		{
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(x , cos) + __FIX7_9_EXT_MULT(z , sin)),
			vector.y,
			__FIX7_9_EXT_TO_FIXED(-__FIX7_9_EXT_MULT(x , sin) + __FIX7_9_EXT_MULT(z , cos))
		};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::rotateZAxis(Vector3D vector, int16 degrees)
{
	fix7_9_ext cos = __FIX7_9_TO_FIX7_9_EXT(__COS(degrees));
	fix7_9_ext sin = __FIX7_9_TO_FIX7_9_EXT(__SIN(degrees));

	fix7_9_ext x = __FIXED_TO_FIX7_9_EXT(vector.x);
	fix7_9_ext y = __FIXED_TO_FIX7_9_EXT(vector.y);

	return (Vector3D) 
		{
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(x , cos) - __FIX7_9_EXT_MULT(y , sin)),
			__FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_MULT(x , sin) + __FIX7_9_EXT_MULT(y , cos)),
			vector.z
		};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::getFromVector2D(Vector2D vector2D, fixed_t z)
{
	return (Vector3D){vector2D.x, vector2D.y, z};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::getFromPixelVector(PixelVector pixelVector)
{
	return (Vector3D)
	{
		__PIXELS_TO_METERS(pixelVector.x),
		__PIXELS_TO_METERS(pixelVector.y),
		__PIXELS_TO_METERS(pixelVector.z)
	};
}
//---------------------------------------------------------------------------------------------------------
static inline Vector3D Vector3D::getFromScreenPixelVector(ScreenPixelVector screenPixelVector)
{
	return (Vector3D)
	{
		__PIXELS_TO_METERS(screenPixelVector.x),
		__PIXELS_TO_METERS(screenPixelVector.y),
		__PIXELS_TO_METERS(screenPixelVector.z)
	};
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector3D::length(Vector3D vector)
{
	fixed_ext_t lengthSquare = __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y) + __FIXED_EXT_MULT(vector.z, vector.z);

	return Math_squareRootFixed(lengthSquare);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_ext_t Vector3D::squareLength(Vector3D vector)
{
	return __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y) + __FIXED_EXT_MULT(vector.z, vector.z);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector3D::lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fixed_ext_t lengthSquareA = __FIXED_EXT_MULT(vectorA.x, vectorA.x) + __FIXED_EXT_MULT(vectorA.y, vectorA.y) + __FIXED_EXT_MULT(vectorA.z, vectorA.z);
	fixed_ext_t lengthSquareB = __FIXED_EXT_MULT(vectorB.x, vectorB.x) + __FIXED_EXT_MULT(vectorB.y, vectorB.y) + __FIXED_EXT_MULT(vectorB.z, vectorB.z);

	fixed_ext_t product = __FIXED_EXT_MULT(lengthSquareA, lengthSquareB);

	return Math_squareRootFixed(product);
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_ext_t Vector3D::dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIXED_EXT_MULT(vectorA.x, vectorB.x) + __FIXED_EXT_MULT(vectorA.y, vectorB.y) + __FIXED_EXT_MULT(vectorA.z, vectorB.z);
}
//---------------------------------------------------------------------------------------------------------
static inline fix19_13 Vector3D::dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.x), __FIXED_TO_FIX19_13(vectorB.x)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.y), __FIXED_TO_FIX19_13(vectorB.y)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.z), (vectorB.z));
}
//---------------------------------------------------------------------------------------------------------
static inline fixed_t Vector3D::getScale(fixed_t z, bool applyScalingMultiplier)
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
		projectedWidth >>= __PROJECTION_PRECISION_INCREMENT;
	}

	return __FIXED_EXT_DIV(projectedWidth, _optical->halfWidth);
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector3D::isLeft(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 < (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector3D::isRight(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 > (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector3D::areEqual(Vector3D a, Vector3D b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector3D::isValueInRange(fixed_t number, fixed_t limitA, fixed_t limitB)
{
	if(limitA < limitB)
	{
		return (unsigned)(number - limitA) <= (unsigned)(limitB - limitA);
	}

	return (unsigned)(number - limitB) <= (unsigned)(limitA - limitB);
}
//---------------------------------------------------------------------------------------------------------
static inline bool Vector3D::isVectorInsideLine(Vector3D vector, Vector3D a, Vector3D b)
{
	return (Vector3D::isValueInRange(vector.x, a.x, b.x)
		&&
		Vector3D::isValueInRange(vector.y, a.y, b.y)
		&&
		Vector3D::isValueInRange(vector.z, a.z, b.z)
	);
}
//---------------------------------------------------------------------------------------------------------


#endif
