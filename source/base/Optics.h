/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
