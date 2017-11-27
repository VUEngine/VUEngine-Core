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

#ifndef OPTICS_H_
#define	OPTICS_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// these improve performance in the real machine

//---------------------------------------------------------------------------------------------------------
//												EXTERNALS
//---------------------------------------------------------------------------------------------------------

extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//											3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Calculate parallax based on the x and z coordinates
 *
 * @fn				Optics_calculateParallax()
 * @public
 *
 * @param x			X parameter for the calculation of the parallax displacement
 * @param x			Z parameter for the calculation of the parallax displacement
 */
inline int Optics_calculateParallax(fix19_13 x, fix19_13 z)
{
	fix19_13 leftEyePoint, rightEyePoint;
	fix19_13 leftEyeGx, rightEyeGx;

	ASSERT(0 <= _optical->baseDistance, "Optics::calculateParallax: baseDistance < 0");

	// set map position and parallax
	leftEyePoint = _optical->horizontalViewPointCenter - ((unsigned)_optical->baseDistance >> 1);
	rightEyePoint = _optical->horizontalViewPointCenter + ((unsigned)_optical->baseDistance >> 1);

	leftEyeGx = x - __FIX19_13_DIV(__FIX19_13_MULT((x - leftEyePoint) , (z)) , (_optical->distanceEyeScreen + z));
	rightEyeGx = x + __FIX19_13_DIV(__FIX19_13_MULT((rightEyePoint - x) , (z)) , (_optical->distanceEyeScreen + z));

	return __FIX19_13_TO_I(rightEyeGx - leftEyeGx) / __PARALLAX_CORRECTION_FACTOR;
}


#endif
