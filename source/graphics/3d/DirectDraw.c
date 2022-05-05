/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DirectDraw.h>
#include <Box.h>
#include <Polyhedron.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

extern uint32* _currentDrawingFrameBufferSet;
#ifdef __PROFILE_DIRECT_DRAWING
DirectDraw _directDraw = NULL;
#endif

#define __DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME		4000

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			DirectDraw::getInstance()
 * @public
 * @return		DirectDraw instance
 */


/**
 * Class constructor
 *
 * @private
 */
void DirectDraw::constructor()
{
	Base::constructor();

	this->totalDrawPixels = 0;

#ifdef __PROFILE_DIRECT_DRAWING
	_directDraw = this;
#endif
}

/**
 * Class destructor
 */
void DirectDraw::destructor()
{
	// allow a new construct
	Base::destructor();
}

uint32 pixelCount = 0;
/**
 * Reset
 */
void DirectDraw::reset()
{
	pixelCount = this->totalDrawPixels; 

	// dummy, don't remove
	this->totalDrawPixels = 0;
}

/**
 * Draws a pixel on the screen.
 * This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
 * instead.
 *
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 * @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
 */
static void DirectDraw::drawPixel(uint32 buffer, uint16 x, uint16 y, int32 color)
{
	// a pointer to the buffer
	//int32* pointer = (int32*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	pointer += ((x << 6) + (y >> 2));

	// draw the pixel
	*pointer |= (color << ((y & 3) << 1));
}

/**
 * Draws a non black pixel on the screen.
 * This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
 * instead.
 *
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 * @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
 */
#ifndef __DIRECT_DRAW_INTERLACED
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, uint16 x, uint16 y, uint16 parallax, int32 color, uint8 bufferIndex __attribute__((unused)))
{
	if(__SCREEN_HEIGHT <= (unsigned)y)
	{
		return;
	}

	if(__SCREEN_WIDTH <= (unsigned)(x - parallax) || __SCREEN_WIDTH <= (unsigned)(x + parallax))
	{
		return;
	}

	uint16 yHelper = y >> 2;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	leftBuffer  += (((x - parallax) << 6) + yHelper);
	rightBuffer += (((x + parallax) << 6) + yHelper);

	uint8 pixel = color << ((y & 3) << 1);

	// draw the pixel
	*leftBuffer  |= pixel;
	*rightBuffer |= pixel;

	_directDraw->totalDrawPixels += 2;
}
#else
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, uint16 x, uint16 y, uint16 parallax, int32 color, uint8 bufferIndex)
{
	if(__SCREEN_HEIGHT <= (unsigned)y)
	{
		return;
	}

	if(__SCREEN_WIDTH <= (unsigned)(x - parallax) || __SCREEN_WIDTH <= (unsigned)(x + parallax))
	{
		return;
	}

	uint16 yHelper = y >> 2;

	BYTE* buffer = 0;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	if(0 == bufferIndex)
	{
		buffer = leftBuffer + (((x - parallax) << 6) + yHelper);
	}
	else
	{
		buffer = rightBuffer + (((x + parallax) << 6) + yHelper);
	}

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	uint8 pixel = color << ((y & 3) << 1);

	// draw the pixel
	*buffer |= pixel;

	_directDraw->totalDrawPixels++;
}
#endif

/**
 * Draws a black pixel on the screen.
 * We have a separate function for black pixes since nulling bits requires a slightly different way than
 * simply writing 1's. Adding an if clause instead to the putPixel method instead would be too heavy on
 * the processor when used inside a loop due to the branching.
 *
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
static void DirectDraw::drawBlackPixel(uint32 buffer, uint16 x, uint16 y)
{
	// a pointer to the buffer
	//int32* pointer = (int32*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	pointer += ((x << 6) + (y >> 2));

	// draw the pixel
	*pointer &= ~(0b11 << ((y & 3) << 1));
}

/**
 * Draws a black pixel on the screen.
 *
 * @private
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
static void DirectDraw::drawBlackPixelWrapper(uint32 buffer, uint16 x, uint16 y, int32 color __attribute__ ((unused)))
{
	DirectDraw::drawBlackPixel(buffer, x, y);
}

/**
 * Draws a point on screen in the given color
 *
 * @param point 	Point to draw
 * @param color		The color to draw (0-3)
 */
void DirectDraw::drawPoint(PixelVector point, int32 color)
{
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

	if((unsigned)(point.x - point.parallax - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0)
		&&
		(unsigned)(point.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)
	)
	{
		if(color == __COLOR_BLACK)
		{
			DirectDraw::drawBlackPixel(leftBuffer, (uint16)(point.x - point.parallax), (uint16)point.y);
		}
		else
		{
			DirectDraw::drawPixel(leftBuffer, (uint16)(point.x - point.parallax), (uint16)point.y, color);
		}
	}
	if((unsigned)(point.x + point.parallax - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0)
		&&
		(unsigned)(point.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)
	)
	{
		if(color == __COLOR_BLACK)
		{
			DirectDraw::drawBlackPixel(rightBuffer, (uint16)(point.x + point.parallax), (uint16)point.y);
		}
		else
		{
			DirectDraw::drawPixel(rightBuffer, (uint16)(point.x + point.parallax), (uint16)point.y, color);
		}
	}
}

/**
 * Draws a line between two given 2D points
 *
 * @param fromPoint Point 1
 * @param toPoint	Point 2
 * @param color		The color to draw (0-3)
 */
void DirectDraw::drawLine(PixelVector fromPoint, PixelVector toPoint, int32 color)
{
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

	fix19_13 fromPointX = __I_TO_FIX19_13(fromPoint.x);
	fix19_13 fromPointY = __I_TO_FIX19_13(fromPoint.y);

	fix19_13 toPointX = __I_TO_FIX19_13(toPoint.x);
	fix19_13 toPointY = __I_TO_FIX19_13(toPoint.y);

	fix19_13 dx = __ABS(toPointX - fromPointX);
	fix19_13 dy = __ABS(toPointY - fromPointY);

	if(0 == dx && 0 == dy)
	{
		return;
	}

	fix19_13 stepX = dx ? __I_TO_FIX19_13(1) : 0;
	fix19_13 stepY = dy ? __I_TO_FIX19_13(1) : 0;
	fix19_13 parallax = __I_TO_FIX19_13(fromPoint.parallax);
	fix19_13 parallaxDelta = __I_TO_FIX19_13(toPoint.parallax - fromPoint.parallax);

	void (*drawPixelMethod)(uint32 buffer, uint16 x, uint16 y, int32 color) = DirectDraw::drawPixel;
	// duplicating code here since it is much lighter on the cpu

	if(color == __COLOR_BLACK)
	{
		drawPixelMethod = DirectDraw::drawBlackPixelWrapper;
	}

	fix19_13* fromCoordinate = NULL;
	fix19_13* toCoordinate = NULL;
	fix19_13 parallaxStep = 0;

	if(dy == dx || dy < dx || 0 == dy)
	{
		fromCoordinate = &fromPointX;
		toCoordinate = &toPointX;

		stepX = __I_TO_FIX19_13(1);
		stepY = __FIX10_6_TO_FIX19_13(__FIX10_6_DIV(__FIX19_13_TO_FIX10_6(dy), __FIX19_13_TO_FIX10_6(dx)));

		if(toPointX < fromPointX)
		{
			fix19_13 aux = toPointX;
			toPointX = fromPointX;
			fromPointX = aux;

			aux = toPointY;
			toPointY = fromPointY;
			fromPointY = aux;
		}

		if(toPointY < fromPointY)
		{
			stepY = -stepY;
		}

		parallaxStep = __FIX10_6_TO_FIX19_13(__FIX10_6_DIV(__FIX19_13_TO_FIX10_6(parallaxDelta), __FIX19_13_TO_FIX10_6(dx)));
	}
	else if(dx < dy || 0 == dx)
	{
		fromCoordinate = &fromPointY;
		toCoordinate = &toPointY;

		// make sure that no software based divisions is introduced
		stepX = __FIX10_6_TO_FIX19_13(__FIX10_6_DIV(__FIX19_13_TO_FIX10_6(dx), __FIX19_13_TO_FIX10_6(dy)));
		stepY = __I_TO_FIX19_13(1);

		if(toPointY < fromPointY)
		{
			fix19_13 aux = toPointX;
			toPointX = fromPointX;
			fromPointX = aux;

			aux = toPointY;
			toPointY = fromPointY;
			fromPointY = aux;
		}

		if(toPointX < fromPointX)
		{
			stepX = -stepX;
		}

		parallaxStep = __FIX10_6_TO_FIX19_13(__FIX10_6_DIV(__FIX19_13_TO_FIX10_6(parallaxDelta), __FIX19_13_TO_FIX10_6(dy)));
	}

	fix19_13 auxParallax = parallax;

	while(*fromCoordinate <= *toCoordinate)
	{
		parallax = auxParallax;

		if((unsigned)(fromPointY - __I_TO_FIX19_13(_cameraFrustum->y0)) < (unsigned)(__I_TO_FIX19_13(_cameraFrustum->y1) - __I_TO_FIX19_13(_cameraFrustum->y0)))
		{
			if((unsigned)(fromPointX - parallax - __I_TO_FIX19_13(_cameraFrustum->x0)) < (unsigned)(__I_TO_FIX19_13(_cameraFrustum->x1) - __I_TO_FIX19_13(_cameraFrustum->x0)))
			{
				drawPixelMethod(leftBuffer, (uint16)__FIX19_13_TO_I(fromPointX - parallax), (uint16)__FIX19_13_TO_I(fromPointY), color);
			}

			if((unsigned)(fromPointX + parallax - __I_TO_FIX19_13(_cameraFrustum->x0)) < (unsigned)(__I_TO_FIX19_13(_cameraFrustum->x1) - __I_TO_FIX19_13(_cameraFrustum->x0)))
			{
				drawPixelMethod(rightBuffer, (uint16)__FIX19_13_TO_I(fromPointX + parallax), (uint16)__FIX19_13_TO_I(fromPointY), color);
			}
		}

		fromPointX += stepX;
		fromPointY += stepY;
		auxParallax += parallaxStep;
	}
}

static PixelVector DirectDraw::clampPixelVector(PixelVector vector)
{
	if(-__FIX10_6_MAXIMUM_VALUE_TO_I > vector.x)
	{
		vector.x = -__FIX10_6_MAXIMUM_VALUE_TO_I;
	}
	else if(__FIX10_6_MAXIMUM_VALUE_TO_I < vector.x)
	{
		vector.x = __FIX10_6_MAXIMUM_VALUE_TO_I;
	}

	if(-__FIX10_6_MAXIMUM_VALUE_TO_I > vector.y)
	{
		vector.y = -__FIX10_6_MAXIMUM_VALUE_TO_I;
	}
	else if(__FIX10_6_MAXIMUM_VALUE_TO_I < vector.y)
	{
		vector.y = __FIX10_6_MAXIMUM_VALUE_TO_I;
	}

	return vector;
}

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, int32 clampLimit, uint8 bufferIndex)
{
	if(__DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME < _directDraw->totalDrawPixels)
	{
		return;
	}

	if(0 == clampLimit || __FIX10_6_MAXIMUM_VALUE_TO_I < clampLimit)
	{
		fromPoint = DirectDraw::clampPixelVector(fromPoint);
		toPoint = DirectDraw::clampPixelVector(toPoint);
	}

	uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

	fix10_6 fromPointX = __I_TO_FIX10_6(fromPoint.x);
	fix10_6 fromPointY = __I_TO_FIX10_6(fromPoint.y);

	fix10_6 toPointX = __I_TO_FIX10_6(toPoint.x);
	fix10_6 toPointY = __I_TO_FIX10_6(toPoint.y);

	fix10_6 dx = toPointX - fromPointX;
	fix10_6 dy = toPointY - fromPointY;

	if(0 == dx && 0 == dy)
	{
		return;
	}

	fix10_6 dp = __I_TO_FIX10_6(toPoint.parallax - fromPoint.parallax);

	fix10_6 dxABS = __ABS(dx);
	fix10_6 dyABS = __ABS(dy);

	fix10_6 stepX = toPointX < fromPointX ? - __I_TO_FIX10_6(1) : __I_TO_FIX10_6(1);
	fix10_6 stepY = toPointY < fromPointY ? - __I_TO_FIX10_6(1) : __I_TO_FIX10_6(1);
	fix10_6 parallaxStep = 0;

	fix10_6 parallax = __I_TO_FIX10_6(fromPoint.parallax >> 1);

	if(dyABS == dxABS || dyABS < dxABS || 0 == dy)
	{
		stepY = __FIX10_6_DIV(dy, dxABS);
		parallaxStep = __FIX10_6_DIV(dp, dxABS) >> 1;		

	CACHE_ENABLE;

		while(toPointX != fromPointX)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, (uint16)__FIX10_6_TO_I(fromPointX), (uint16)__FIX10_6_TO_I(fromPointY), __FIX10_6_TO_I(parallax), color, bufferIndex);
			fromPointX += stepX;
			fromPointY += stepY;
			parallax += parallaxStep;
#ifdef __DIRECT_DRAW_INTERLACED
			bufferIndex = !bufferIndex;
#endif
		}
	CACHE_DISABLE;
	}
	else if(dxABS < dyABS || 0 == dx)
	{
		// make sure that no software based divisions is introduced
		stepX = __FIX10_6_DIV(dx, dyABS);
		parallaxStep = __FIX10_6_DIV(dp, dyABS) >> 1;

	CACHE_ENABLE;

		while(toPointY != fromPointY)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, (uint16)__FIX10_6_TO_I(fromPointX), (uint16)__FIX10_6_TO_I(fromPointY), __FIX10_6_TO_I(parallax), color, bufferIndex);
			fromPointX += stepX;
			fromPointY += stepY;
			parallax += parallaxStep;
#ifdef __DIRECT_DRAW_INTERLACED
			bufferIndex = !bufferIndex;
#endif
		}
	CACHE_DISABLE;
	}

}


