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

#include <Camera.h>
#include <Math.h>
#include <Object.h>
#include <Optical.h>
#include <Optics.h>
#include <PixelVector.h>


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
	static inline Vector3D getFrom2D(Vector2D vector2D, fixed_t z);
	static inline Vector3D get(Vector3D from, Vector3D to);
	static inline Vector3D sum(Vector3D a, Vector3D b);
	static inline Vector3D sub(Vector3D a, Vector3D b);
	static inline Vector3D scale(Vector3D vector, Scale scale);
	static inline Vector3D perpedicular(Vector3D a, bool left);
	static inline Vector3D perpedicularXPlane(Vector3D a, bool left);
	static inline Vector3D perpedicularYPlane(Vector3D a, bool left);
	static inline Vector3D perpedicularZPlane(Vector3D a, bool left);
	static inline Vector3D intermediate(Vector3D a, Vector3D b);
	static inline fixed_ext_t dotProduct(Vector3D vectorA, Vector3D vectorB);
	static inline fix19_13 dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB);
	static inline Vector3D scalarProduct(Vector3D vector, fixed_t scalar);
	static inline Vector3D scalarDivision(Vector3D vector, fixed_t scalar);
	static inline Vector3D normalize(Vector3D vector);
	static inline Vector3D getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC);
	static inline fixed_t length(Vector3D vector);
	static inline fixed_ext_t squareLength(Vector3D vector);
	static inline fixed_t lengthProduct(Vector3D vectorA, Vector3D vectorB);
	static inline Vector3D getRelativeToCamera(Vector3D vector3D);
	static inline fixed_t getScale(fixed_t z, bool applyScalingMultiplier);
	static inline PixelVector projectToPixelVector(Vector3D vector3D, int16 parallax);
	static inline PixelVector transformToPixelVector(Vector3D vector);
	static inline Vector3D getFromPixelVector(PixelVector screenVector);
	static inline Vector3D getFromScreenPixelVector(ScreenPixelVector screenPixelVector);
	static inline bool isLeft(Vector3D a, Vector3D b, Vector3D p);
	static inline bool isRight(Vector3D a, Vector3D b, Vector3D p);
	static inline bool areEqual(Vector3D a, Vector3D b);
	static inline Vector3D projectOnto(Vector3D p, Vector3D a, Vector3D b);
	static inline Vector3D projectOntoHighPrecision(Vector3D p, Vector3D a, Vector3D b);
	static inline bool isValueInRange(fixed_t value, fixed_t limitA, fixed_t limitB);
	static inline bool isVectorInsideLine(Vector3D vector, Vector3D lineStart, Vector3D lineEnd);
	static inline Vector3D rotateXAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotateYAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotateZAxis(Vector3D vector, int16 degrees);
	static inline Vector3D rotate(Vector3D vector, Rotation rotation);
	static inline bool isVisible(Vector3D vector, PixelRightBox pixelRightBox, int16 padding);
	static void print(Vector3D vector, int32 x, int32 y);
	static void printRaw(Vector3D vector, int32 x, int32 y);
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
		__X_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Y_AXIS & axis ? __I_TO_FIXED(1) : 0,
		__Z_AXIS & axis ? __I_TO_FIXED(1) : 0
	};
}

static inline Vector3D Vector3D::getFrom2D(Vector2D vector2D, fixed_t z)
{
	return (Vector3D){vector2D.x, vector2D.y, z};
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

static inline Vector3D Vector3D::scale(Vector3D vector, Scale scale)
{
	return (Vector3D){__FIXED_EXT_MULT(vector.x, __FIX7_9_TO_FIXED(scale.x)), __FIXED_EXT_MULT(vector.y, __FIX7_9_TO_FIXED(scale.y)), __FIXED_EXT_MULT(vector.z, __FIX7_9_TO_FIXED(scale.z))};
}

static inline Vector3D Vector3D::perpedicular(Vector3D a, bool left)
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

static inline Vector3D Vector3D::perpedicularXPlane(Vector3D a, bool left)
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

static inline Vector3D Vector3D::perpedicularYPlane(Vector3D a, bool left)
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

static inline Vector3D Vector3D::perpedicularZPlane(Vector3D a, bool left)
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

static inline Vector3D Vector3D::intermediate(Vector3D a, Vector3D b)
{
	return (Vector3D)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}

static inline fixed_ext_t Vector3D::dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIXED_EXT_MULT(vectorA.x, vectorB.x) + __FIXED_EXT_MULT(vectorA.y, vectorB.y) + __FIXED_EXT_MULT(vectorA.z, vectorB.z);
}

static inline fix19_13 Vector3D::dotProductHighPrecision(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.x), __FIXED_TO_FIX19_13(vectorB.x)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.y), __FIXED_TO_FIX19_13(vectorB.y)) +
			__FIX19_13_MULT(__FIXED_TO_FIX19_13(vectorA.z), (vectorB.z));
}


static inline Vector3D Vector3D::scalarProduct(Vector3D vector, fixed_t scalar)
{
	return (Vector3D){__FIXED_MULT(vector.x, scalar), __FIXED_MULT(vector.y, scalar), __FIXED_MULT(vector.z, scalar)};
}

static inline Vector3D Vector3D::scalarDivision(Vector3D vector, fixed_t scalar)
{
	if(0 == scalar)
	{
		return Vector3D::zero();
	}

	return (Vector3D){__FIXED_DIV(vector.x, scalar), __FIXED_DIV(vector.y, scalar), __FIXED_DIV(vector.z, scalar)};
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
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.y, v.z) - __FIXED_EXT_MULT(u.z, v.y)),
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.z, v.x) - __FIXED_EXT_MULT(u.x, v.z)),
		__FIXED_EXT_TO_FIXED(__FIXED_EXT_MULT(u.x, v.y) - __FIXED_EXT_MULT(u.y, v.x)),
	};
}

static inline fixed_t Vector3D::length(Vector3D vector)
{
	fixed_ext_t lengthSquare = __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y) + __FIXED_EXT_MULT(vector.z, vector.z);

	return Math_squareRootFixed(lengthSquare);
}

static inline fixed_ext_t Vector3D::squareLength(Vector3D vector)
{
	return __FIXED_EXT_MULT(vector.x, vector.x) + __FIXED_EXT_MULT(vector.y, vector.y) + __FIXED_EXT_MULT(vector.z, vector.z);
}

static inline fixed_t Vector3D::lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fixed_ext_t lengthSquareA = __FIXED_EXT_MULT(vectorA.x, vectorA.x) + __FIXED_EXT_MULT(vectorA.y, vectorA.y) + __FIXED_EXT_MULT(vectorA.z, vectorA.z);
	fixed_ext_t lengthSquareB = __FIXED_EXT_MULT(vectorB.x, vectorB.x) + __FIXED_EXT_MULT(vectorB.y, vectorB.y) + __FIXED_EXT_MULT(vectorB.z, vectorB.z);

	fixed_ext_t product = __FIXED_EXT_MULT(lengthSquareA, lengthSquareB);

	return Math_squareRootFixed(product);
}

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

static inline Vector3D Vector3D::getRelativeToCamera(Vector3D vector3D)
{
	vector3D.x -= _cameraPosition->x;
	vector3D.y -= _cameraPosition->y;
	vector3D.z -= _cameraPosition->z;

	return vector3D;
}

static inline PixelVector Vector3D::projectToPixelVector(Vector3D vector3D, int16 parallax)
{
	fixed_ext_t x = (fixed_ext_t)(vector3D.x);
	fixed_ext_t y = (fixed_ext_t)(vector3D.y);
	fixed_ext_t z = (fixed_ext_t)(vector3D.z);

#ifdef __LEGACY_COORDINATE_PROJECTION
	if(0 != z)
	{
		x -= (__FIXED_EXT_MULT(x - _optical->horizontalViewPointCenter, z) >> _optical->maximumXViewDistancePower);	
		y -= (__FIXED_EXT_MULT(y - _optical->verticalViewPointCenter, z) >> _optical->maximumYViewDistancePower);	
/*
		fixed_ext_t factor = __FIXED_EXT_DIV(__FIXED_EXT_MULT(_optical->halfWidth, _optical->aspectRatioXfov) << __PROJECTION_PRECISION_INCREMENT, z + _optical->cameraNearPlane);

		x = (__FIXED_EXT_MULT(x - _optical->horizontalViewPointCenter, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->horizontalViewPointCenter;	
		y = (__FIXED_EXT_MULT(y - _optical->verticalViewPointCenter, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->verticalViewPointCenter;
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

		x = (__FIXED_EXT_MULT(x, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->horizontalViewPointCenter;	
		y = (__FIXED_EXT_MULT(y, factor) >> __PROJECTION_PRECISION_INCREMENT) + _optical->verticalViewPointCenter;
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

static inline PixelVector Vector3D::transformToPixelVector(Vector3D vector)
{
	vector = Vector3D::rotate(Vector3D::sub(vector, *_cameraPosition), *_cameraInvertedRotation);
	
	return Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
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
		__PIXELS_TO_METERS(screenPixelVector.z)
	};
}

static inline bool Vector3D::isLeft(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 < (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector3D::isRight(Vector3D a, Vector3D b, Vector3D p)
{
	return 0 > (__FIXED_EXT_MULT((b.x - a.x), (p.y - a.y)) - __FIXED_EXT_MULT((b.y - a.y), (p.x - a.x)));
}

static inline bool Vector3D::areEqual(Vector3D a, Vector3D b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

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

static inline bool Vector3D::isValueInRange(fixed_t value, fixed_t limitA, fixed_t limitB)
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

static inline bool Vector3D::isVisible(Vector3D vector, PixelRightBox pixelRightBox, int16 padding)
{
	extern const CameraFrustum* _cameraFrustum;
	vector = Vector3D::rotate(Vector3D::getRelativeToCamera(vector), *_cameraInvertedRotation);
	PixelVector pixelVector = Vector3D::projectToPixelVector(vector, 0);

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


#endif
