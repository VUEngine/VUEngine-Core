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
	static inline int16 calculateParallax(fixed_t z);
}


/**
 * Calculate parallax based on the x and z coordinates
 *
 * @param x	X parameter for the calculation of the parallax displacement
 * @param z	Z parameter for the calculation of the parallax displacement
 * @return 	Parallax (in pixels)
 */
static inline int16 Optics::calculateParallax(fixed_t z)
{
	fixed_t divisor = (_optical->halfWidth << 1) + z;

	if(0 == divisor)
	{
		return 0;
	}
	
	ASSERT(0 <= _optical->baseDistance, "Optics::calculateParallax: baseDistance < 0");

	return __METERS_TO_PIXELS(__FIXED_EXT_DIV(__FIXED_EXT_MULT(((unsigned)_optical->baseDistance), z), divisor));
}

#endif
