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

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Math.h>
#include <Optical.h>
#include <Optics.h>
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Vector2D : Object
{
	/// @publicsection
	static inline Vector2D zero();
	static inline Vector2D unit(uint16 axis);
	static inline Vector2D getFrom3D(Vector3D vector3D);
	static inline Vector2D get(Vector2D from, Vector2D to);
	static inline Vector2D sum(Vector2D a, Vector2D b);
	static inline Vector2D sub(Vector2D a, Vector2D b);
	static inline Vector2D scale(Vector2D vector, Scale scale);
	static inline Vector2D perpedicular(Vector2D a, bool left);
	static inline Vector2D intermediate(Vector2D a, Vector2D b);
	static inline fixed_ext_t dotProduct(Vector2D vectorA, Vector2D vectorB);
	static inline fix19_13 dotProductHighPrecision(Vector2D vectorA, Vector2D vectorB);
	static inline Vector2D scalarProduct(Vector2D vector, fixed_t scalar);
	static inline Vector2D scalarDivision(Vector2D vector, fixed_t scalar);
	static inline Vector2D normalize(Vector2D vector);
	static inline fixed_t length(Vector2D vector);
	static inline fixed_ext_t squareLength(Vector2D vector);
	static inline fixed_t lengthProduct(Vector2D vectorA, Vector2D vectorB);
	static inline Vector2D getRelativeToCamera(Vector2D Vector2D);
	static inline fixed_t getScale(fixed_t z, bool applyScalingMultiplier);
	static inline Vector2D getFromPixelVector(PixelVector screenVector);
	static inline Vector2D getFromScreenPixelVector(ScreenPixelVector screenPixelVector);
	static inline bool isLeft(Vector2D a, Vector2D b, Vector2D p);
	static inline bool isRight(Vector2D a, Vector2D b, Vector2D p);
	static inline bool areEqual(Vector2D a, Vector2D b);
	static inline Vector2D projectOnto(Vector2D p, Vector2D a, Vector2D b);
	static inline Vector2D projectOntoHighPrecision(Vector2D p, Vector2D a, Vector2D b);
	static inline bool isValueInRange(fixed_t value, fixed_t limitA, fixed_t limitB);
	static inline bool isVectorInsideLine(Vector2D vector, Vector2D lineStart, Vector2D lineEnd);
	static inline Vector2D rotate(Vector2D vector, int16 degrees);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Vector2D Vector2D::zero()
{
	return (Vector2D){0, 0};
}

static inline Vector2D Vector2D::unit(uint16 axis)
{
	return (Vector2D)
	{
		__X_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Y_AXIS & axis ? __I_TO_FIXED(1) : 0
	};
}

static inline Vector2D Vector2D::getFrom3D(Vector3D vector3D)
{
	return (Vector2D){vector3D.x, vector3D.y};
}

static inline Vector2D Vector2D::get(Vector2D from, Vector2D to)
{
	return (Vector2D){to.x - from.x, to.y - from.y};
}

static inline Vector2D Vector2D::sum(Vector2D a, Vector2D b)
{
	return (Vector2D){a.x + b.x, a.y + b.y};
}

static inline Vector2D Vector2D::sub(Vector2D a, Vector2D b)
{
	return (Vector2D){a.x - b.x, a.y - b.y};
}

static inline Vector2D Vector2D::scale(Vector2D vector, Scale scale)
{
	return (Vector2D){__FIXED_EXT_MULT(vector.x, __FIX7_9_TO_FIXED(scale.x)), __FIXED_EXT_MULT(vector.y, __FIX7_9_TO_FIXED(scale.y))};
}

static inline Vector2D Vector2D::perpedicular(Vector2D a, bool left)
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

static inline Vector2D Vector2D::intermediate(Vector2D a, Vector2D b)
{
	return (Vector2D)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1
	};
}

static inline fixed_ext_t Vector2D::dotProduct(Vector2D vectorA, Vector2D vectorB)
{
	return __FIXED_EXT_MULT(vectorA.x, vectorB.x) + __FIXED_EXT_MULT(vectorA.y, vectorB.y);
}

static inline fix19_13 Vector2D::dotProductHighPrecision(Vector2D vectorA, Vector2D vectorB)
{
	return __FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.x), __FIXED_TO_FIX19_13(vectorB.x)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.y), __FIXED_TO_FIX19_13(vectorB.y));
}


static inline Vector2D Vector2D::scalarProduct(Vector2D vector, fixed_t scalar)
{
	return (Vector2D){__FIXED_MULT(vector.x, scalar), __FIXED_MULT(vector.y, scalar)};
}

static inline Vector2D Vector2D::scalarDivision(Vector2D vector, fixed_t scalar)
{
	if(0 == scalar)
	{
		return Vector2D::zero();
	}

	return (Vector2D){__FIXED_DIV(vector.x, scalar), __FIXED_DIV(vector.y, scalar)};
}

static inline Vector2D Vector2D::normalize(Vector2D vector)
{
	return Vector2D::scalarDivision(vector, Vector2D::length(vector));
}

static inline fixed_t Vector2D::length(Vector2D vector)
{
	fixed_ext_t lengthSquare = __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y);

	return Math_squareRootFixed(lengthSquare);
}

static inline fixed_ext_t Vector2D::squareLength(Vector2D vector)
{
	return __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y);
}

static inline fixed_t Vector2D::lengthProduct(Vector2D vectorA, Vector2D vectorB)
{
	fixed_ext_t lengthSquareA = __FIXED_EXT_MULT(vectorA.x, vectorA.x) + __FIXED_EXT_MULT(vectorA.y, vectorA.y);
	fixed_ext_t lengthSquareB = __FIXED_EXT_MULT(vectorB.x, vectorB.x) + __FIXED_EXT_MULT(vectorB.y, vectorB.y);
	
	fixed_ext_t product = __FIXED_EXT_MULT(lengthSquareA, lengthSquareB);

	return Math_squareRootFixed(product);
}

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
		projectedWidth >>= __PROJECTION_PRECISION_INCREMENT;
	}

	return __FIXED_EXT_DIV(projectedWidth, _optical->halfWidth);
}

static inline Vector2D Vector2D::getRelativeToCamera(Vector2D Vector2D)
{
	Vector2D.x -= _cameraPosition->x;
	Vector2D.y -= _cameraPosition->y;

	return Vector2D;
}

static inline Vector2D Vector2D::getFromPixelVector(PixelVector pixelVector)
{
	return (Vector2D)
	{
		__PIXELS_TO_METERS(pixelVector.x),
		__PIXELS_TO_METERS(pixelVector.y)
	};
}

static inline Vector2D Vector2D::getFromScreenPixelVector(ScreenPixelVector screenPixelVector)
{
	return (Vector2D)
	{
		__PIXELS_TO_METERS(screenPixelVector.x),
		__PIXELS_TO_METERS(screenPixelVector.y)
	};
}

static inline bool Vector2D::isLeft(Vector2D a, Vector2D b, Vector2D p)
{
	return 0 < (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector2D::isRight(Vector2D a, Vector2D b, Vector2D p)
{
	return 0 > (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector2D::areEqual(Vector2D a, Vector2D b)
{
	return a.x == b.x && a.y == b.y;
}

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

static inline bool Vector2D::isValueInRange(fixed_t value, fixed_t limitA, fixed_t limitB)
{
	if(limitA < limitB)
	{
		return (unsigned)(value - limitA) <= (unsigned)(limitB - limitA);
	}

	return (unsigned)(value - limitB) <= (unsigned)(limitA - limitB);
}

static inline bool Vector2D::isVectorInsideLine(Vector2D vector, Vector2D lineStart, Vector2D lineEnd)
{
	return (Vector2D::isValueInRange(vector.x, lineStart.x, lineEnd.x)
		&&
		Vector2D::isValueInRange(vector.y, lineStart.y, lineEnd.y)
	);
}

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

#endif
