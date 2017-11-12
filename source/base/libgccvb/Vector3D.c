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

fix19_13 Vector3D_dotProduct(Vector3D vectorA, Vector3D vectorB)
{
	return __FIX19_13_MULT(vectorA.x, vectorB.x) + __FIX19_13_MULT(vectorA.y, vectorB.y) + __FIX19_13_MULT(vectorA.z, vectorB.z);
}

Vector3D Vector3D_scalarProduct(Vector3D vector, fix19_13 scalar)
{
	return (Vector3D){__FIX19_13_MULT(vector.x, scalar), __FIX19_13_MULT(vector.y, scalar), __FIX19_13_MULT(vector.z, scalar)};
}

Vector3D Vector3D_normalize(Vector3D vector)
{
	fix19_13 length = Vector3D_length(vector);

	if(length)
	{
		return (Vector3D){__FIX19_13_DIV(vector.x, length), __FIX19_13_DIV(vector.y, length),__FIX19_13_DIV(vector.z, length)};
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
		__FIX19_13_MULT(u.y, v.z) - __FIX19_13_MULT(u.z, v.y),
		__FIX19_13_MULT(u.z, v.x) - __FIX19_13_MULT(u.x, v.z),
		__FIX19_13_MULT(u.x, v.y) - __FIX19_13_MULT(u.y, v.x),
	};
}

fix19_13 Vector3D_length(Vector3D vector)
{
	fix51_13 lengthSquare = __FIX51_13_MULT(vector.x, vector.x) + __FIX51_13_MULT(vector.y, vector.y) + __FIX51_13_MULT(vector.z, vector.z);

	return __F_TO_FIX19_13(Math_squareRoot(__FIX51_13_TO_F(lengthSquare)));
}

fix19_13 Vector3D_lengthProduct(Vector3D vectorA, Vector3D vectorB)
{
	fix51_13 lengthSquareA = __FIX51_13_MULT(vectorA.x, vectorA.x) + __FIX51_13_MULT(vectorA.y, vectorA.y) + __FIX51_13_MULT(vectorA.z, vectorA.z);
	fix51_13 lengthSquareB = __FIX51_13_MULT(vectorB.x, vectorB.x) + __FIX51_13_MULT(vectorB.y, vectorB.y) + __FIX51_13_MULT(vectorB.z, vectorB.z);

	fix51_13 product = __FIX51_13_MULT(lengthSquareA, lengthSquareB);

	return __F_TO_FIX19_13(Math_squareRoot(__FIX51_13_TO_F(product)));
}