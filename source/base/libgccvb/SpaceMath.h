/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef VECTOR_3D_H_
#define VECTOR_3D_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>


//---------------------------------------------------------------------------------------------------------
//											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

inline Vector3D Vector3D_get(Vector3D from, Vector3D to);
inline fix10_6 Vector3D_dotProduct(Vector3D vectorA, Vector3D vectorB);
inline Vector3D Vector3D_scalarProduct(Vector3D vector, fix10_6 scalar);
inline Vector3D Vector3D_normalize(Vector3D vector);
inline Vector3D Vector3D_getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC);
inline fix10_6 Vector3D_length(Vector3D vector);
inline fix19_13 Vector3D_squareLength(Vector3D vector);
inline fix10_6 Vector3D_lengthProduct(Vector3D vectorA, Vector3D vectorB);
inline Vector3D Vector3D_getRelativeToCamera(Vector3D vector3D);
inline PixelVector Vector3D_projectToPixelVector(Vector3D vector3D, s16 parallax);
inline Vector3D Vector3D_getFromPixelVector(PixelVector screenVector);
inline PixelVector PixelVector_getFromVector3D(Vector3D vector3D);
inline Size Size_getFromPixelSize(PixelSize pixelSize);


//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

inline Vector3D Vector3D_get(Vector3D from, Vector3D to)
{
	return (Vector3D){to.x - from.x, to.y - from.y, to.z - from.z};
}

inline fix10_6 Vector3D_dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX10_6_MULT(vectorA.x, vectorB.x) + __FIX10_6_MULT(vectorA.y, vectorB.y) + __FIX10_6_MULT(vectorA.z, vectorB.z);
}

inline Vector3D Vector3D_scalarProduct(Vector3D vector, fix10_6 scalar)
{
	return (Vector3D){__FIX10_6_MULT(vector.x, scalar), __FIX10_6_MULT(vector.y, scalar), __FIX10_6_MULT(vector.z, scalar)};
}

inline Vector3D Vector3D_normalize(Vector3D vector)
{
	fix10_6 length = Vector3D_length(vector);

	if(length)
	{
		return (Vector3D){__FIX10_6_DIV(vector.x, length), __FIX10_6_DIV(vector.y, length),__FIX10_6_DIV(vector.z, length)};
	}

	return (Vector3D){0, 0, 0};
}

inline Vector3D Vector3D_getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC)
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
		__FIX10_6_MULT(u.y, v.z) - __FIX10_6_MULT(u.z, v.y),
		__FIX10_6_MULT(u.z, v.x) - __FIX10_6_MULT(u.x, v.z),
		__FIX10_6_MULT(u.x, v.y) - __FIX10_6_MULT(u.y, v.x),
	};
}

inline fix10_6 Vector3D_length(Vector3D vector)
{
	fix10_6_ext lengthSquare = __FIX10_6_EXT_MULT(vector.x, vector.x) + __FIX10_6_EXT_MULT(vector.y, vector.y) + __FIX10_6_EXT_MULT(vector.z, vector.z);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_EXT_TO_F(lengthSquare)));
}

inline fix19_13 Vector3D_squareLength(Vector3D vector)
{
	return __FIX10_6_MULT(vector.x, vector.x) + __FIX10_6_MULT(vector.y, vector.y) + __FIX10_6_MULT(vector.z, vector.z);
}

inline fix10_6 Vector3D_lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fix10_6 lengthSquareA = __FIX10_6_MULT(vectorA.x, vectorA.x) + __FIX10_6_MULT(vectorA.y, vectorA.y) + __FIX10_6_MULT(vectorA.z, vectorA.z);
	fix10_6 lengthSquareB = __FIX10_6_MULT(vectorB.x, vectorB.x) + __FIX10_6_MULT(vectorB.y, vectorB.y) + __FIX10_6_MULT(vectorB.z, vectorB.z);

	fix10_6 product = __FIX10_6_MULT(lengthSquareA, lengthSquareB);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(product)));
}

inline Vector3D Vector3D_getRelativeToCamera(Vector3D vector3D)
{
	extern const Vector3D* _cameraPosition;

	vector3D.x -= _cameraPosition->x;
	vector3D.y -= _cameraPosition->y;
	vector3D.z -= _cameraPosition->z;

	return vector3D;
}

inline PixelVector Vector3D_projectToPixelVector(Vector3D vector3D, s16 parallax)
{
	extern const Optical* _optical;

	fix10_6_ext x = (fix10_6_ext)vector3D.x;
	fix10_6_ext y = (fix10_6_ext)vector3D.y;
	fix10_6_ext z = (fix10_6_ext)vector3D.z;

	PixelVector projection =
	{
		__METERS_TO_PIXELS(x - (__FIX10_6_EXT_MULT(x - _optical->horizontalViewPointCenter, z) >> _optical->maximumViewDistancePower)),
		__METERS_TO_PIXELS(y - (__FIX10_6_EXT_MULT(y - _optical->verticalViewPointCenter, z) >> _optical->maximumViewDistancePower)),
		z,
		parallax
	};

	return projection;
}

inline Vector3D Vector3D_getFromPixelVector(PixelVector screenVector)
{
	return (Vector3D)
	{
		__PIXELS_TO_METERS(screenVector.x),
		__PIXELS_TO_METERS(screenVector.y),
		__PIXELS_TO_METERS(screenVector.z)
	};
}

inline PixelVector PixelVector_getFromVector3D(Vector3D vector3D)
{
	return (PixelVector)
	{
		__METERS_TO_PIXELS(vector3D.x),
		__METERS_TO_PIXELS(vector3D.y),
		__METERS_TO_PIXELS(vector3D.z),
		0
	};
}

inline Size Size_getFromPixelSize(PixelSize pixelSize)
{
	return (Size)
	{
		__PIXELS_TO_METERS(pixelSize.x),
		__PIXELS_TO_METERS(pixelSize.y),
		__PIXELS_TO_METERS(pixelSize.z)
	};
}

inline PixelSize PixelSize_getFromSize(Size size)
{
	return (PixelSize)
	{
		__METERS_TO_PIXELS(size.x),
		__METERS_TO_PIXELS(size.y),
		__METERS_TO_PIXELS(size.z)
	};
}

inline Optical Optical_getFromPixelOptical(PixelOptical pixelOptical)
{
	return (Optical)
	{
		pixelOptical.maximumViewDistancePower,
		__PIXELS_TO_METERS(pixelOptical.distanceEyeScreen),
		__PIXELS_TO_METERS(pixelOptical.baseDistance),
		__PIXELS_TO_METERS(pixelOptical.horizontalViewPointCenter),
		__PIXELS_TO_METERS(pixelOptical.verticalViewPointCenter),
	};
}

#endif
