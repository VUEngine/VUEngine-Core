/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define DirectDraw_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\

/**
 * @class	DirectDraw
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(DirectDraw, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void DirectDraw_constructor(DirectDraw this);

extern u32* _currentDrawingFrameBufferSet;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			DirectDraw_getInstance()
 * @memberof	DirectDraw
 * @public
 *
 * @return		DirectDraw instance
 */
__SINGLETON(DirectDraw);

/**
 * Class constructor
 *
 * @memberof	DirectDraw
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) DirectDraw_constructor(DirectDraw this)
{
	ASSERT(this, "DirectDraw::constructor: null this");

	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	DirectDraw
 * @public
 *
 * @param this	Function scope
 */
void DirectDraw_destructor(DirectDraw this)
{
	ASSERT(this, "DirectDraw::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Draws a pixel on the screen.
 * This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
 * instead.
 *
 * @brief			Draws a pixel on the screen
 * @memberof		DirectDraw
 * @public
 *
 * @param this		Function scope
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 * @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
 */
void DirectDraw_drawPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, u16 x, u16 y, int color)
{
	ASSERT(this, "DirectDraw::drawPixel: null this");

	// a pointer to the buffer
	//int* pointer = (int*)buffer;
	BYTE* pointer = (BYTE*)buffer;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	pointer += ((x << 6) + (y >> 2));

	// draw the pixel
	*pointer |= (color << ((y & 3) << 1));
}

/**
 * Draws a black pixel on the screen.
 * We have a separate function for black pixes since nulling bits requires a slightly different way than
 * simply writing 1's. Adding an if clause instead to the putPixel method instead would be too heavy on
 * the processor when used inside a loop due to the branching.
 *
 * @brief			Draws a black pixel on the screen
 * @memberof		DirectDraw
 * @public
 *
 * @param this		Function scope
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
void DirectDraw_drawBlackPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, u16 x, u16 y)
{
	ASSERT(this, "DirectDraw::drawBlackPixel: null this");

	// a pointer to the buffer
	//int* pointer = (int*)buffer;
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
 * @brief			Draws a black pixel on the screen
 * @memberof		DirectDraw
 * @private
 *
 * @param this		Function scope
 * @param buffer	Buffer base address
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
static void DirectDraw_drawBlackPixelWrapper(DirectDraw this __attribute__ ((unused)), u32 buffer, u16 x, u16 y, int color)
{
	ASSERT(this, "DirectDraw::drawBlackPixel: null this");

	DirectDraw_drawBlackPixel(this, buffer, x, y);
}


/**
 * Draws a point between two given 2D points
 *
 * @memberof		DirectDraw
 * @public
 *
 * @param this		Function scope
 * @param point 	Point to draw
 * @param color		The color to draw (0-3)
 */
void DirectDraw_drawPoint(DirectDraw this, PixelVector point, int color)
{
	u32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	u32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

	if((unsigned)(point.x - point.parallax - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0)
		&&
		(unsigned)(point.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)
	)
	{
		DirectDraw_drawPixel(this, leftBuffer, (u16)(point.x - point.parallax), (u16)point.y, color);
	}
	if((unsigned)(point.x + point.parallax - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0)
		&&
		(unsigned)(point.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)
	)
	{
		DirectDraw_drawPixel(this, rightBuffer, (u16)(point.x + point.parallax), (u16)point.y, color);
	}
}

/**
 * Draws a line between two given 2D points
 *
 * @memberof		DirectDraw
 * @public
 *
 * @param this		Function scope
 * @param fromPoint Point 1
 * @param toPoint	Point 2
 * @param color		The color to draw (0-3)
 */
void DirectDraw_drawLine(DirectDraw this, PixelVector fromPoint, PixelVector toPoint, int color)
{
	ASSERT(this, "DirectDraw::drawLine: null this");

	u32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	u32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

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

	void (*drawPixelMethod)(DirectDraw this __attribute__ ((unused)), u32 buffer, u16 x, u16 y, int color) = DirectDraw_drawPixel;
	// duplicating code here since it is much lighter on the cpu

	if(color == __COLOR_BLACK)
	{
		drawPixelMethod = DirectDraw_drawBlackPixelWrapper;
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
				drawPixelMethod(this, leftBuffer, (u16)__FIX19_13_TO_I(fromPointX - parallax), (u16)__FIX19_13_TO_I(fromPointY), color);
			}

			if((unsigned)(fromPointX + parallax - __I_TO_FIX19_13(_cameraFrustum->x0)) < (unsigned)(__I_TO_FIX19_13(_cameraFrustum->x1) - __I_TO_FIX19_13(_cameraFrustum->x0)))
			{
				drawPixelMethod(this, rightBuffer, (u16)__FIX19_13_TO_I(fromPointX + parallax), (u16)__FIX19_13_TO_I(fromPointY), color);
			}
		}

		fromPointX += stepX;
		fromPointY += stepY;
		auxParallax += parallaxStep;
	}
}
