/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PIXEL_SIZE_H_
#define PIXEL_SIZE_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class PixelSize : ListenerObject
{
	/// @publicsection
	static inline PixelSize getFromSize(Size size);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline PixelSize PixelSize::getFromSize(Size size)
{
	return (PixelSize)
	{
		__METERS_TO_PIXELS(size.x),
		__METERS_TO_PIXELS(size.y),
		__METERS_TO_PIXELS(size.z)
	};
}


#endif
