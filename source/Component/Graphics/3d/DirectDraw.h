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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __FRAME_BUFFERS_SIZE								0x6000

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class DirectDraw
///
/// Inherits from Object
///
/// Implements various draw rutines that manipulate the frame buffers.
singleton class DirectDraw : ListenerObject
{
	/// @protectedsection

	/// Number of pixels drawn during the current cycle
	uint16 drawnPixelsCounter;

	/// Maximum number of pixels to draw during each cycle
	uint16 maximumPixelsToDraw;

	/// @publicsection

	/// Draw a single point.
	/// @param point: Screen coordinate where to draw the point
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawPoint(PixelVector point, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw a line.
	/// @param fromPoint: Line's starting point
	/// @param toPoint: Line's ending point
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawLine(PixelVector fromPoint, PixelVector toPoint, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw a circle.
	/// @param center: Circle's center
	/// @param radius: Circle's radius
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw an X.
	/// @param center: X's center
	/// @param length: X's arms length
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawX(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw a cross.
	/// @param center: Cross' center
	/// @param length: Cross' arms length
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawCross(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw a solid circle.
	/// @param center: Circle's center
	/// @param radius: Circle's radius
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawSolidCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);

	/// Draw a solid rhombus.
	/// @param center: Rhombus' center
	/// @param radius: Rhombus' radius
	/// @param color: Color of the point to draw (only non black)
	/// @param bufferIndex: Buffer set index for interlaced drawing
	/// @param interlaced: If true, the drawing is interlaced
	/// @return True if a pixel was written to the frame buffers
	static bool drawSolidRhumbus(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);

	/// Reset the manager's state
	void reset();

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
