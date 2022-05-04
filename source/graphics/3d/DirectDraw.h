/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef DIRECT_DRAW_H_
#define DIRECT_DRAW_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-3d
singleton class DirectDraw : Object
{
	/// @publicsection
	static DirectDraw getInstance();

	void reset();
	void drawLine(PixelVector fromPoint, PixelVector toPoint, int32 color);
	void drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color);
	void drawPoint(PixelVector point, int32 color);
}


#endif
