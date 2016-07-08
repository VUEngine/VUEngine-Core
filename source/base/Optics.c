/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Optics.h>
#include <VPUManager.h>


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
	if(WRLD_AFFINE != mapMode)
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