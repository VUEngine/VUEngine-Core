/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SIZE_H_
#define SIZE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Math.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Size
///
/// Inherits from Object
///
/// Implements methods to operate on Size structs.
static class Size : Object
{
	/// @publicsection

	/// Convert a PixelSize struct to a Size one.
	/// @param pixelSize: PixelSize struct to convert
	/// @return Size struct
	static inline Size getFromPixelSize(PixelSize pixelSize);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Size Size::getFromPixelSize(PixelSize pixelSize)
{
	return (Size)
	{
		__PIXELS_TO_METERS(pixelSize.x),
		__PIXELS_TO_METERS(pixelSize.y),
		__PIXELS_TO_METERS(pixelSize.z)
	};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


#endif
