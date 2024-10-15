/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OPTICS_H_
#define	OPTICS_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class Optics
///
/// Inherits from Object
///
/// Computes parallax.
/// @ingroup base
static class Optics : Object
{
	/// @publicsection

	/// Calculate parallax based on the z coordinate.
	/// @param z: 3D coordinate
	/// @return Parallax value (in pixels)
	static inline int16 calculateParallax(fixed_t z);
}

//=========================================================================================================
// CLASS'S STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------

#endif
