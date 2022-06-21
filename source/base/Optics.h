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
static class Optics : ListenerObject
{
	/// @publicsection
	static inline int16 calculateParallax(fix10_6 z);
}


/**
 * Calculate parallax based on the x and z coordinates
 *
 * @param x	X parameter for the calculation of the parallax displacement
 * @param z	Z parameter for the calculation of the parallax displacement
 * @return 	Parallax (in pixels)
 */
static inline int16 Optics::calculateParallax(fix10_6 z)
{
	if(0 == (_optical->halfWidth << 1) + z)
	{
		return 0;
	}
	
	ASSERT(0 <= _optical->baseDistance, "Optics::calculateParallax: baseDistance < 0");

	return __METERS_TO_PIXELS(__FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(((unsigned)_optical->baseDistance) << 1, z), (_optical->halfWidth << 1) + z) >> 1);
}

#endif
