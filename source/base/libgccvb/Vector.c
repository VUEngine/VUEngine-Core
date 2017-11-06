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

#include <Vector.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
//											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

VBVec3D Vector_get(VBVec3D from, VBVec3D to)
{
	return (VBVec3D){to.x - from.x, to.y - from.y, to.z - from.z};
}

fix19_13 Vector_dotProduct(VBVec3D vectorA, VBVec3D vectorB)
{
	return __FIX19_13_MULT(vectorA.x, vectorB.x) + __FIX19_13_MULT(vectorA.y, vectorB.y) + __FIX19_13_MULT(vectorA.z, vectorB.z);
}

VBVec3D Vector_scalarProduct(VBVec3D vector, fix19_13 scalar)
{
	return (VBVec3D){__FIX19_13_MULT(vector.x, scalar), __FIX19_13_MULT(vector.y, scalar), __FIX19_13_MULT(vector.z, scalar)};
}

VBVec3D Vector_normalize(VBVec3D vector)
{
//	long magnitudeSquare = __FIX19_13_TO_I(vector.x) * __FIX19_13_TO_I(vector.x) + __FIX19_13_TO_I(vector.y) * __FIX19_13_TO_I(vector.y) + __FIX19_13_TO_I(vector.z) * __FIX19_13_TO_I(vector.z);
//	fix19_13 magnitude = __F_TO_FIX19_13(Math_squareRoot(magnitudeSquare));

	fix19_13 magnitudeSquare = __FIX19_13_MULT(vector.x, vector.x) + __FIX19_13_MULT(vector.y, vector.y) + __FIX19_13_MULT(vector.z, vector.z);
	fix19_13 magnitude = __F_TO_FIX19_13(Math_squareRoot(__FIX19_13_TO_F(magnitudeSquare)));
	return (VBVec3D){__FIX19_13_DIV(vector.x, magnitude), __FIX19_13_DIV(vector.y, magnitude),__FIX19_13_DIV(vector.z, magnitude)};
}

VBVec3D Vector_getPlaneNormal(VBVec3D vectorA, VBVec3D vectorB, VBVec3D vectorC)
{
	VBVec3D u =
	{
		vectorB.x - vectorA.x,
		vectorB.y - vectorA.y,
		vectorB.z - vectorA.z,
	};

	VBVec3D v =
	{
		vectorC.x - vectorA.x,
		vectorC.y - vectorA.y,
		vectorC.z - vectorA.z,
	};

	return (VBVec3D)
	{
		__FIX19_13_MULT(u.y, v.z) - __FIX19_13_MULT(u.z, v.y),
		__FIX19_13_MULT(u.z, v.x) - __FIX19_13_MULT(u.x, v.z),
		__FIX19_13_MULT(u.x, v.y) - __FIX19_13_MULT(u.y, v.x),
	};
}
