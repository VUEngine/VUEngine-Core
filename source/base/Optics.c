/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Optics.h>


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

	fix19_13 leftEjeGx, rightEjeGx;

	// set map position and parallax
	leftEyePoint = _optical->horizontalViewPointCenter - (_optical->baseDistance >> 1);

	rightEyePoint = _optical->horizontalViewPointCenter + (_optical->baseDistance >> 1);

	leftEjeGx = x - FIX19_13_DIV(FIX19_13_MULT((x - leftEyePoint) , (z)) , (_optical->distanceEyeScreen + z));
	rightEjeGx = x + FIX19_13_DIV(FIX19_13_MULT((rightEyePoint - x) , (z)) , (_optical->distanceEyeScreen + z));

	return FIX19_13TOI(rightEjeGx - leftEjeGx) / __PARALLAX_CORRECTION_FACTOR;
}

// calculate the size of a given magnitud, being it a 8 pixel multiple
inline u16 Optics_calculateRealSize(u16 magnitude, u16 mapMode, fix7_9 scale)
{
	if(WRLD_AFFINE != mapMode)
	{
		return  FIX19_13_ROUNDTOI(FIX19_13_DIV(ITOFIX19_13((int)magnitude), FIX7_9TOFIX19_13(scale)));
	}

	return magnitude;
}


//determine if a point is visible
inline bool Optics_isVisible(VBVec3D position3D, u16 width, u16 height, int parallax, int pad)
{
	int lowLimit = 0 - parallax - pad;
	int highLimit = __SCREEN_WIDTH + parallax + pad;

	VBVec2D position2D;

	//normalize position
	__OPTICS_NORMALIZE(position3D);

	//project the position to 2d space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	width >>= 1;
	height >>= 1;

	// check x visibility
	if(FIX19_13TOI(position2D.x) + width < lowLimit || FIX19_13TOI(position2D.x) - width > highLimit)
	{
		return false;
	}

	lowLimit = - pad;
	highLimit = __SCREEN_HEIGHT + pad;

	// check y visibility
	if(FIX19_13TOI(position2D.y) + height < lowLimit || FIX19_13TOI(position2D.y) - height > highLimit)
	{
		return false;
	}

	return true;
}

// determine the squared length of a given vector
inline int Optics_lengthSquared3D(VBVec3D vect1, VBVec3D vect2)
{
	return  FIX19_13TOI(FIX19_13_MULT((vect1.x - vect2.x), (vect1.x - vect2.x)) +
			FIX19_13_MULT((vect1.y - vect2.y), (vect1.y - vect2.y))+
			FIX19_13_MULT((vect1.z - vect2.z), (vect1.z - vect2.z)));
}