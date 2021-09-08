/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
//												EXTERNALS
//---------------------------------------------------------------------------------------------------------

extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//											3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
static class Optics : Object
{
	/// @publicsection
	static inline int16 calculateParallax(fix10_6 x, fix10_6 z);
}


/**
 * Calculate parallax based on the x and z coordinates
 *
 * @param x	X parameter for the calculation of the parallax displacement
 * @param z	Z parameter for the calculation of the parallax displacement
 * @return 	Parallax (in pixels)
 */
static inline int16 Optics::calculateParallax(fix10_6 x, fix10_6 z)
{
	if(0 == z)
	{
		return 0;
	}
	
	fix10_6 leftEyePoint, rightEyePoint;
	fix10_6 leftEyeGx, rightEyeGx;

	ASSERT(0 <= _optical->baseDistance, "Optics::calculateParallax: baseDistance < 0");

	// set map position and parallax
	leftEyePoint = _optical->horizontalViewPointCenter - ((unsigned)_optical->baseDistance);
	rightEyePoint = _optical->horizontalViewPointCenter + ((unsigned)_optical->baseDistance);
	leftEyeGx = x + __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(leftEyePoint, z) , (_optical->distanceEyeScreen + z));
	rightEyeGx = x + __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(rightEyePoint, z) , (_optical->distanceEyeScreen + z));

	return __METERS_TO_PIXELS((rightEyeGx - leftEyeGx) / 2);
}

#endif
