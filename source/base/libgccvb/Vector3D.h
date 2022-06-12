/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VECTOR_3D_H_
#define VECTOR_3D_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>
#include <Printing.h>
#include <Optical.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Vector3D : Object
{
	/// @publicsection
	static inline Vector3D zero();
	static inline Vector3D unit(uint16 axis);
	static inline Vector3D get(Vector3D from, Vector3D to);
	static inline Vector3D sum(Vector3D a, Vector3D b);
	static inline Vector3D sub(Vector3D a, Vector3D b);
	static inline Vector3D perpedicular(Vector3D a, bool left);
	static inline Vector3D intermediate(Vector3D a, Vector3D b);
	static inline fix10_6_ext dotProduct(Vector3D vectorA, Vector3D vectorB);
	static inline fix19_13 dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB);
	static inline Vector3D scalarProduct(Vector3D vector, fix10_6 scalar);
	static inline Vector3D scalarDivision(Vector3D vector, fix10_6 scalar);
	static inline Vector3D normalize(Vector3D vector);
	static inline Vector3D getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC);
	static inline fix10_6 length(Vector3D vector);
	static inline fix10_6_ext squareLength(Vector3D vector);
	static inline fix10_6 lengthProduct(Vector3D vectorA, Vector3D vectorB);
	static inline Vector3D getRelativeToCamera(Vector3D vector3D);
	static inline fix10_6 getScale(fix10_6 z);
	static inline PixelVector projectToPixelVector(Vector3D vector3D, int16 parallax);
	static inline Vector3D getFromPixelVector(PixelVector screenVector);
	static inline Vector3D getFromScreenPixelVector(ScreenPixelVector screenPixelVector);
	static inline bool isLeft(Vector3D a, Vector3D b, Vector3D p);
	static inline bool isRight(Vector3D a, Vector3D b, Vector3D p);
	static inline bool areEqual(Vector3D a, Vector3D b);
	static inline Vector3D projectOnto(Vector3D p, Vector3D a, Vector3D b);
	static inline Vector3D projectOntoHighPrecision(Vector3D p, Vector3D a, Vector3D b);
	static inline bool isValueInRange(fix10_6 value, fix10_6 limitA, fix10_6 limitB);
	static inline bool isVectorInsideLine(Vector3D vector, Vector3D lineStart, Vector3D lineEnd);
	static inline Vector3D rotateXAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotateYAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotateZAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotate(Vector3D vector, Rotation rotation);
	static inline void print(Vector3D vector, int32 x, int32 y);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Vector3D Vector3D::zero()
{
	return (Vector3D){0, 0, 0};
}

static inline Vector3D Vector3D::unit(uint16 axis)
{
	return (Vector3D)
	{
		__X_AXIS & axis ? __I_TO_FIX10_6(1) : 0,
		__Y_AXIS & axis ? __I_TO_FIX10_6(1) : 0,
		__Z_AXIS & axis ? __I_TO_FIX10_6(1) : 0
	};
}

static inline Vector3D Vector3D::get(Vector3D from, Vector3D to)
{
	return (Vector3D){to.x - from.x, to.y - from.y, to.z - from.z};
}

static inline Vector3D Vector3D::sum(Vector3D a, Vector3D b)
{
	return (Vector3D){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vector3D Vector3D::sub(Vector3D a, Vector3D b)
{
	return (Vector3D){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vector3D Vector3D::perpedicular(Vector3D a, bool left)
{
	if(left)
	{
		fix10_6 aux = a.x;
		a.x = -a.y;
		a.y = aux;
	}
	else
	{
		fix10_6 aux = a.x;
		a.x = a.y;
		a.y = -aux;
	}

	return a;
}

static inline Vector3D Vector3D::intermediate(Vector3D a, Vector3D b)
{
	return (Vector3D)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}

static inline fix10_6_ext Vector3D::dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX10_6_EXT_MULT(vectorA.x, vectorB.x) + __FIX10_6_EXT_MULT(vectorA.y, vectorB.y) + __FIX10_6_EXT_MULT(vectorA.z, vectorB.z);
}

static inline fix19_13 Vector3D::dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX19_13_MULT(__FIX10_6_TO_FIX19_13(vectorA.x), __FIX10_6_TO_FIX19_13(vectorB.x)) +
			__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(vectorA.y), __FIX10_6_TO_FIX19_13(vectorB.y)) +
			__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(vectorA.z), (vectorB.z));
}


static inline Vector3D Vector3D::scalarProduct(Vector3D vector, fix10_6 scalar)
{
	return (Vector3D){__FIX10_6_MULT(vector.x, scalar), __FIX10_6_MULT(vector.y, scalar), __FIX10_6_MULT(vector.z, scalar)};
}

static inline Vector3D Vector3D::scalarDivision(Vector3D vector, fix10_6 scalar)
{
	if(0 != scalar)
	{
		return (Vector3D){__FIX10_6_DIV(vector.x, scalar), __FIX10_6_DIV(vector.y, scalar), __FIX10_6_DIV(vector.z, scalar)};
	}

	return Vector3D::zero();
}

static inline Vector3D Vector3D::normalize(Vector3D vector)
{
	return Vector3D::scalarDivision(vector, Vector3D::length(vector));
}

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
		__FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(u.y, v.z) - __FIX10_6_EXT_MULT(u.z, v.y)),
		__FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(u.z, v.x) - __FIX10_6_EXT_MULT(u.x, v.z)),
		__FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(u.x, v.y) - __FIX10_6_EXT_MULT(u.y, v.x)),
	};
}

static inline fix10_6 Vector3D::length(Vector3D vector)
{
	fix10_6_ext lengthSquare = __FIX10_6_EXT_MULT(vector.x, vector.x) + __FIX10_6_EXT_MULT(vector.y, vector.y) + __FIX10_6_EXT_MULT(vector.z, vector.z);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_EXT_TO_F(lengthSquare)));
}

static inline fix10_6_ext Vector3D::squareLength(Vector3D vector)
{
	return __FIX10_6_EXT_MULT(vector.x, vector.x) + __FIX10_6_EXT_MULT(vector.y, vector.y) + __FIX10_6_EXT_MULT(vector.z, vector.z);
}

static inline fix10_6 Vector3D::lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fix10_6_ext lengthSquareA = __FIX10_6_EXT_MULT(vectorA.x, vectorA.x) + __FIX10_6_EXT_MULT(vectorA.y, vectorA.y) + __FIX10_6_EXT_MULT(vectorA.z, vectorA.z);
	fix10_6_ext lengthSquareB = __FIX10_6_EXT_MULT(vectorB.x, vectorB.x) + __FIX10_6_EXT_MULT(vectorB.y, vectorB.y) + __FIX10_6_EXT_MULT(vectorB.z, vectorB.z);

	fix10_6_ext product = __FIX10_6_EXT_MULT(lengthSquareA, lengthSquareB);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_EXT_TO_F(product)));
}

static inline fix10_6 Vector3D::getScale(fix10_6 z)
{
	if(0 == _optical->halfWidth || 0 == z + _optical->cameraNearPlane)
	{
		return __1I_FIX10_6;
	}

	fix10_6_ext projectedWidth = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(_optical->halfWidth, _optical->scalingMultiplier), z + _optical->cameraNearPlane) >> __PROJECTION_PRECISION_INCREMENT;

	return __FIX10_6_EXT_DIV(projectedWidth, _optical->halfWidth);
}

static inline Vector3D Vector3D::getRelativeToCamera(Vector3D vector3D)
{
	extern const Vector3D* _cameraPosition;

	vector3D.x -= _cameraPosition->x;
	vector3D.y -= _cameraPosition->y;
	vector3D.z -= _cameraPosition->z;

	return vector3D;
}

static inline PixelVector Vector3D::projectToPixelVector(Vector3D vector3D, int16 parallax)
{
	extern const Vector3D* _cameraPosition;
	extern const Optical* _optical;

	fix10_6_ext x = (fix10_6_ext)(vector3D.x);
	fix10_6_ext y = (fix10_6_ext)(vector3D.y);
	fix10_6_ext z = (fix10_6_ext)(vector3D.z);

#ifdef __LEGACY_COORDINATE_PROJECTION
	if(0 != z)
	{
		x -= (__FIX10_6_EXT_MULT(x - _optical->horizontalViewPointCenter, z) >> _optical->maximumXViewDistancePower);	
		y -= (__FIX10_6_EXT_MULT(y - _optical->verticalViewPointCenter, z) >> _optical->maximumYViewDistancePower);	
/*
		fix10_6_ext factor = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(_optical->halfWidth, _optical->aspectRatioXfov) << __PROJECTION_PRECISION_INCREMENT, z + _optical->cameraNearPlane);

		x = (__FIX10_6_EXT_MULT(x - _optical->horizontalViewPointCenter, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->horizontalViewPointCenter;	
		y = (__FIX10_6_EXT_MULT(y - _optical->verticalViewPointCenter, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->verticalViewPointCenter;
*/
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
		x = __FIX10_6_EXT_MULT(x, _optical->aspectRatioXfov);
		// y = y * fov
		// since fov = 1 because it is assumed and angle of 90, there is no
		// need to make the computation
		y = __FIX10_6_EXT_MULT(y, _optical->fov);
		// z = z * (far + near) / (far - near) + (2 * far * near) / (near - far)
		// since the near plane will always be 0, there is no need to perform 
		// this product		
		z = __FIX10_6_EXT_MULT(z, _optical->farRatio1Near) + _optical->farRatio2Near;

		x = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(x, _optical->halfHeight), z) + _optical->horizontalViewPointCenter;	
		y = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(y, _optical->halfWidth), z) + _optical->verticalViewPointCenter;
		*/

		// Fast and produces the expected result
		// x = x * aspect ratio * fov

		// to reduce from 4 products and 2 divisions to 3 products, 1 division and 3 bit shifts
		fix10_6_ext factor = __FIX10_6_EXT_DIV(_optical->projectionMultiplierHelper, z + _optical->cameraNearPlane);

		x = (__FIX10_6_EXT_MULT(x, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->horizontalViewPointCenter;	
		y = (__FIX10_6_EXT_MULT(y, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->verticalViewPointCenter;
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


static inline Vector3D Vector3D::getFromPixelVector(PixelVector pixelVector)
{
	return (Vector3D)
	{
		__PIXELS_TO_METERS(pixelVector.x),
		__PIXELS_TO_METERS(pixelVector.y),
		__PIXELS_TO_METERS(pixelVector.z)
	};
}

static inline Vector3D Vector3D::getFromScreenPixelVector(ScreenPixelVector screenPixelVector)
{
	return (Vector3D)
	{
		__PIXELS_TO_METERS(screenPixelVector.x),
		__PIXELS_TO_METERS(screenPixelVector.y),
		__PIXELS_TO_METERS(screenPixelVector.z + screenPixelVector.zDisplacement)
	};
}

static inline bool Vector3D::isLeft(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 < (__FIX10_6_MULT((b.x - a.x), (p.y - a.y)) - __FIX10_6_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector3D::isRight(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 > (__FIX10_6_MULT((b.x - a.x), (p.y - a.y)) - __FIX10_6_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector3D::areEqual(Vector3D a, Vector3D b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline Vector3D Vector3D::projectOnto(Vector3D p, Vector3D a, Vector3D b)
{
	Vector3D ap = Vector3D::get(a, p);
	Vector3D ab = Vector3D::get(a, b);
	fix10_6_ext dotApAb = Vector3D::dotProduct(ap, ab);
	fix10_6_ext dotAbAb = Vector3D::dotProduct(ab, ab);

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
		a.x + __FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(__FIX10_6_TO_FIX10_6_EXT(ab.x), __FIX10_6_EXT_DIV(dotApAb, dotAbAb))),
		a.y + __FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(__FIX10_6_TO_FIX10_6_EXT(ab.y), __FIX10_6_EXT_DIV(dotApAb, dotAbAb))),
		a.z + __FIX10_6_EXT_TO_FIX10_6(__FIX10_6_EXT_MULT(__FIX10_6_TO_FIX10_6_EXT(ab.z), __FIX10_6_EXT_DIV(dotApAb, dotAbAb))),
	};

	return projection;
}

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
		a.x + __FIX19_13_TO_FIX10_6(__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(ab.x), __FIX19_13_DIV(dotApAb, dotAbAb))),
		a.y + __FIX19_13_TO_FIX10_6(__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(ab.y), __FIX19_13_DIV(dotApAb, dotAbAb))),
		a.z + __FIX19_13_TO_FIX10_6(__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(ab.z), __FIX19_13_DIV(dotApAb, dotAbAb))),
	};

	return projection;
}

static inline bool Vector3D::isValueInRange(fix10_6 value, fix10_6 limitA, fix10_6 limitB)
{
	if(limitA < limitB)
	{
		return (unsigned)(value - limitA) <= (unsigned)(limitB - limitA);
	}

	return (unsigned)(value - limitB) <= (unsigned)(limitA - limitB);
}

static inline bool Vector3D::isVectorInsideLine(Vector3D vector, Vector3D lineStart, Vector3D lineEnd)
{
	return (Vector3D::isValueInRange(vector.x, lineStart.x, lineEnd.x)
		&&
		Vector3D::isValueInRange(vector.y, lineStart.y, lineEnd.y)
		&&
		Vector3D::isValueInRange(vector.z, lineStart.z, lineEnd.z)
	);
}

static inline Vector3D Vector3D::rotateXAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			vector.x,
			__FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__COS(degrees))) - __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__SIN(degrees))),
			__FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__COS(degrees)))
		};
}

static inline Vector3D Vector3D::rotateYAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__COS(degrees))) - __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__SIN(degrees))),
			vector.y,
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__COS(degrees)))
		};
}

static inline Vector3D Vector3D::rotateZAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__COS(degrees))) - __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__SIN(degrees))),
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__COS(degrees))),
			vector.z
		};
}

static inline Vector3D Vector3D::rotate(Vector3D vector, Rotation rotation)
{
	Vector3D result = vector;

	if(0 != rotation.x)
	{
		result = Vector3D::rotateXAxis(result, __FIX10_6_TO_I(rotation.x + __05F_FIX10_6));
	}

	if(0 != rotation.y)
	{
		result = Vector3D::rotateYAxis(result, __FIX10_6_TO_I(rotation.y + __05F_FIX10_6));
	}

	if(0 != rotation.z)
	{
		result = Vector3D::rotateZAxis(result, __FIX10_6_TO_I(rotation.z + __05F_FIX10_6));
	}

	return result;
}

static inline void Vector3D::print(Vector3D vector, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_FLOAT(__FIX10_6_TO_F(vector.x), x + 2, y);
	PRINT_FLOAT(__FIX10_6_TO_F(vector.y), x + 2, y + 1);
	PRINT_FLOAT(__FIX10_6_TO_F(vector.z), x + 2, y + 2);
}


#endif
