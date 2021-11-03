/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SIZE_H_
#define SIZE_H_

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
static class Size : Object
{
	/// @publicsection
	static inline Size getFromPixelSize(PixelSize pixelSize);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Size Size::getFromPixelSize(PixelSize pixelSize)
{
	return (Size)
	{
		__PIXELS_TO_METERS(pixelSize.x),
		__PIXELS_TO_METERS(pixelSize.y),
		__PIXELS_TO_METERS(pixelSize.z)
	};
}


#endif
