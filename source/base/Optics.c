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

#include <Optics.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// declare global pointers
extern const Optical* _optical;
extern const VBVec3D* _screenPosition;


//---------------------------------------------------------------------------------------------------------
//												3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Calculate parallax based on the x and z coordinates
 *
 * @memberof		Optics
 * @public
 *
 * @param x			X parameter for the calculation of the parallax displacement
 * @param x			Z parameter for the calculation of the parallax displacement
 */
inline int Optics_calculateParallax(fix19_13 x, fix19_13 z)
{
	fix19_13 leftEyePoint, rightEyePoint;

	fix19_13 leftEyeGx, rightEyeGx;

	// set map position and parallax
	leftEyePoint = _optical->horizontalViewPointCenter - (_optical->baseDistance >> 1);

	rightEyePoint = _optical->horizontalViewPointCenter + (_optical->baseDistance >> 1);

	leftEyeGx = x - FIX19_13_DIV(FIX19_13_MULT((x - leftEyePoint) , (z)) , (_optical->distanceEyeScreen + z));
	rightEyeGx = x + FIX19_13_DIV(FIX19_13_MULT((rightEyePoint - x) , (z)) , (_optical->distanceEyeScreen + z));

	return FIX19_13TOI(rightEyeGx - leftEyeGx) / __PARALLAX_CORRECTION_FACTOR;
}

/**
 * Calculate the squared length of a given vector
 *
 * @memberof			Optics
 * @public
 *
 * @param vector1
 * @param vector2
 *
 * @return 				Squared length of the result vector
 */
inline int Optics_lengthSquared3D(VBVec3D vector1, VBVec3D vector2)
{
	return  FIX19_13TOI(FIX19_13_MULT((vector1.x - vector2.x), (vector1.x - vector2.x)) +
			FIX19_13_MULT((vector1.y - vector2.y), (vector1.y - vector2.y))+
			FIX19_13_MULT((vector1.z - vector2.z), (vector1.z - vector2.z)));
}
