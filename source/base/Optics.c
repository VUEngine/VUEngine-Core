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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Optics.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// declare global pointers
extern const Optical* _optical;
extern const VBVec3D* _screenPosition;


//---------------------------------------------------------------------------------------------------------
// 												3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

// calculate the parallax
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

// calculate the size of a given magnitude, being it a 8 pixel multiple
inline int Optics_calculateRealSize(int magnitude, u16 mapMode, fix7_9 scale)
{
	if(__WORLD_AFFINE != mapMode)
	{
		return  FIX19_13_ROUNDTOI(FIX19_13_DIV(ITOFIX19_13(magnitude), FIX7_9TOFIX19_13(scale)));
	}

	return magnitude;
}

// determine the squared length of a given vector
inline int Optics_lengthSquared3D(VBVec3D vect1, VBVec3D vect2)
{
	return  FIX19_13TOI(FIX19_13_MULT((vect1.x - vect2.x), (vect1.x - vect2.x)) +
			FIX19_13_MULT((vect1.y - vect2.y), (vect1.y - vect2.y))+
			FIX19_13_MULT((vect1.z - vect2.z), (vect1.z - vect2.z)));
}
