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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Vector3D.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
//											FUNCTIONS
//---------------------------------------------------------------------------------------------------------


Vector3D Vector3D_get(Vector3D from, Vector3D to)
{
	return (Vector3D){to.x - from.x, to.y - from.y, to.z - from.z};
}

fix10_6 Vector3D_dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX10_6_MULT(vectorA.x, vectorB.x) + __FIX10_6_MULT(vectorA.y, vectorB.y) + __FIX10_6_MULT(vectorA.z, vectorB.z);
}

Vector3D Vector3D_scalarProduct(Vector3D vector, fix10_6 scalar)
{
	return (Vector3D){__FIX10_6_MULT(vector.x, scalar), __FIX10_6_MULT(vector.y, scalar), __FIX10_6_MULT(vector.z, scalar)};
}

Vector3D Vector3D_normalize(Vector3D vector)
{
	fix10_6 length = Vector3D_length(vector);

	if(length)
	{
		return (Vector3D){__FIX10_6_DIV(vector.x, length), __FIX10_6_DIV(vector.y, length),__FIX10_6_DIV(vector.z, length)};
	}

	return (Vector3D){0, 0, 0};
}

Vector3D Vector3D_getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC)
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

fix10_6 Vector3D_length(Vector3D vector)
{
	fix10_6 lengthSquare = __FIX10_6_MULT(vector.x, vector.x) + __FIX10_6_MULT(vector.y, vector.y) + __FIX10_6_MULT(vector.z, vector.z);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(lengthSquare)));
}

fix19_13 Vector3D_squareLength(Vector3D vector)
{
	return __FIX10_6_MULT(vector.x, vector.x) + __FIX10_6_MULT(vector.y, vector.y) + __FIX10_6_MULT(vector.z, vector.z);
}

fix10_6 Vector3D_lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fix10_6 lengthSquareA = __FIX10_6_MULT(vectorA.x, vectorA.x) + __FIX10_6_MULT(vectorA.y, vectorA.y) + __FIX10_6_MULT(vectorA.z, vectorA.z);
	fix10_6 lengthSquareB = __FIX10_6_MULT(vectorB.x, vectorB.x) + __FIX10_6_MULT(vectorB.y, vectorB.y) + __FIX10_6_MULT(vectorB.z, vectorB.z);

	fix10_6 product = __FIX10_6_MULT(lengthSquareA, lengthSquareB);

	return __F_TO_FIX10_6(Math_squareRoot(__FIX10_6_TO_F(product)));
}


Vector3D Vector3D_getRelativeToCamera(Vector3D vector3D)
{
	extern const Vector3D* _cameraPosition;

	vector3D.x -= _cameraPosition->x;
	vector3D.y -= _cameraPosition->y;
	vector3D.z -= _cameraPosition->z;

	return vector3D;
}

Vector2D Vector3D_projectToVector2D(Vector3D vector3D, fix10_6 parallax)
{
	extern const Optical* _optical;

	vector3D.x <<= 4;
	vector3D.y <<= 4;
	vector3D.z <<= 4;

	Vector2D projection =
	{
		vector3D.x + (__FIX10_6_MULT(_optical->horizontalViewPointCenter -  vector3D.x, vector3D.z) >> _optical->maximumViewDistancePower),
		vector3D.y - (__FIX10_6_MULT(vector3D.y - _optical->verticalViewPointCenter, vector3D.z) >> _optical->maximumViewDistancePower),
		vector3D.z,
		parallax
	};

	return projection;
}
