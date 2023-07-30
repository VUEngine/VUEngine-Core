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
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __FRAME_BUFFERS_SIZE								0x6000


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-3d
singleton class DirectDraw : Object
{
	uint16 totalDrawPixels;
	uint16 maximuDrawPixels;

	/// @publicsection
	static DirectDraw getInstance();
	static void writeToFrameBuffers();
	static void drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, uint8 bufferIndex, bool interlaced);
	static void drawColorCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static void drawColorCircumference(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static void drawColorPoint(int16 x, int16 y, int16 parallax, int32 color);
	static void drawColorPointInterlaced(int16 x, int16 y, int16 parallax, int32 color, uint8 bufferIndex);
	static void drawSolidRhumbus(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced);
	static void drawColorCross(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);
	static void drawColorX(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced);
	static bool isPointInsideFrustum(PixelVector point);

	void reset();
	void startDrawing();
	void drawLine(PixelVector fromPoint, PixelVector toPoint, int32 color);
	void drawPoint(PixelVector point, int32 color);
	void setFrustum(CameraFrustum frustum);
	CameraFrustum getFrustum();
}


#endif
