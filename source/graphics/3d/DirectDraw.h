/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef DIRECT_DRAW_H_
#define DIRECT_DRAW_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __FRAME_BUFFERS_SIZE								0x6000


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class DirectDraw
///
/// Inherits from Object
///
/// Implements various draw rutines that manipulate the frame buffers.
/// @ingroup graphics-3d
singleton class DirectDraw : Object
{
	/// @protectedsection

	/// Number of pixels drawn during the current cycle
	uint16 drawnPixelsCounter;

	/// Maximum number of pixels to draw during each cycle
	uint16 maximumPixelsToDraw;

	/// @publicsection

	/// @publicsection
	/// Method to retrieve the singleton instance
	/// @return DirectDraw singleton
	static DirectDraw getInstance();

	static bool drawColorPoint(PixelVector point, int32 color);
	static bool drawColorPointInterlaced(PixelVector point, int32 color, uint8 bufferIndex);
	static bool drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, uint8 bufferIndex, bool interlaced);
	static bool drawColorCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static bool drawColorCircumference(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static bool drawSolidRhumbus(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static bool drawColorCross(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);
	static bool drawColorX(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);

	/// Reset the manager's state
	void reset();

	/// Prepare the manager to start drawing to the frame buffers.
	void preparteToDraw();
	
	/// Configure the frustum where drawing is allowed.
	/// @param frustum: 3D boundary when drawing is allowed
	void setFrustum(CameraFrustum frustum);

	/// Retrieve the frustum where drawing is allowed.
	/// @return 3D boundary when drawing is allowed
	CameraFrustum getFrustum();
	
	/// Print the manager's current status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int16 x, int16 y);
}


#endif
