/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PIXEL_SIZE_H_
#define PIXEL_SIZE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class PixelSize
///
/// Inherits from Object
///
/// Implements methods to operate on PixelSize structs.
static class PixelSize : Object
{
	/// @publicsection

	/// Convert a Size struct to a PixelSize one.
	/// @param size: Size struct to convert
	/// @return PixelSize struct
	static inline PixelSize getFromSize(Size size);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline PixelSize PixelSize::getFromSize(Size size)
{
	return (PixelSize)
	{
		__METERS_TO_PIXELS(size.x),
		__METERS_TO_PIXELS(size.y),
		__METERS_TO_PIXELS(size.z)
	};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
