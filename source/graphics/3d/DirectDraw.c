/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Cuboid.h>
#include <Polyhedron.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


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
 * @param x			Screen x coordinate
 * @param y			Screen y coordinate
 * @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
 */
void DirectDraw_drawPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, int x, int y, int color)
{
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
 * @param x			Screen x coordinate
 * @param y			Screen y coordinate
 */
void DirectDraw_drawBlackPixel(DirectDraw this __attribute__ ((unused)), u32 buffer, int x, int y)
{
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
void DirectDraw_drawLine(DirectDraw this, VBVec2D fromPoint, VBVec2D toPoint, int color)
{
	u32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	u32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

	fromPoint.x = FIX19_13TOI(fromPoint.x);
	fromPoint.y = FIX19_13TOI(fromPoint.y);

	toPoint.x = FIX19_13TOI(toPoint.x);
	toPoint.y = FIX19_13TOI(toPoint.y);

	int dx = toPoint.x - fromPoint.x;
	int dy = toPoint.y - fromPoint.y;

	int stepX = 0, stepY = 0;

	int parallax = fromPoint.parallax;

	if(dy < 0)
	{
		dy = -dy;
		stepY = -1;
	}
	else
	{
		stepY = 1;
	}

	if(dx < 0)
	{
		dx = -dx;
		stepX = -1;
	}
	else
	{
		stepX = 1;
	}

	dy <<= 1;
	dx <<= 1;

	// duplicating code here since it is much lighter on the cpu
	if(color == __COLOR_BLACK)
	{
		if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			&&
			((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
		{
			DirectDraw_drawBlackPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y);
			DirectDraw_drawBlackPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y);
		}

		if(dx > dy)
		{
			int fraction = dy - (dx >> 1);

			int parallaxStep = (dx >> 1) ? ((toPoint.parallax - fromPoint.parallax) / __ABS(dx >> 1)) : 0;

			while(fromPoint.x != toPoint.x)
			{
				if(fraction >= 0)
				{
					fromPoint.y += stepY;
					fraction -= dx;
				}

				fromPoint.x += stepX;
				fraction += dy;

				parallax += parallaxStep;

				if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
					&&
					((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
				{
					DirectDraw_drawBlackPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y);
					DirectDraw_drawBlackPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y);
				}
			}
		}
		else
		{
			int fraction = dx - (dy >> 1);

			int parallaxStep = (dy >> 1) ? ((toPoint.parallax - fromPoint.parallax) / __ABS(dy >> 1)) : 0;

			while(fromPoint.y != toPoint.y)
			{
				if(fraction >= 0)
				{
					fromPoint.x += stepX;
					fraction -= dy;
				}

				fromPoint.y += stepY;
				fraction += dx;

				parallax += parallaxStep;

				if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
					&&
					((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
				{
					DirectDraw_drawBlackPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y);
					DirectDraw_drawBlackPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y);
				}
			}
		}

	}
	else
	{
		if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			&&
			((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
		{
			DirectDraw_drawPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y, color);
			DirectDraw_drawPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y, color);
		}

		if(dx > dy)
		{
			int fraction = dy - (dx >> 1);

			int parallaxStep = (dx >> 1) ? ((toPoint.parallax - fromPoint.parallax) / __ABS(dx >> 1)) : 0;

			while(fromPoint.x != toPoint.x)
			{
				if(fraction >= 0)
				{
					fromPoint.y += stepY;
					fraction -= dx;
				}

				fromPoint.x += stepX;
				fraction += dy;

				parallax += parallaxStep;

				if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
					&&
					((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
				{
					DirectDraw_drawPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y, color);
					DirectDraw_drawPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y, color);
				}
			}
		}
		else
		{
			int fraction = dx - (dy >> 1);

			int parallaxStep = (dy >> 1) ? ((toPoint.parallax - fromPoint.parallax) / __ABS(dy >> 1)) : 0;

			while(fromPoint.y != toPoint.y)
			{
				if(fraction >= 0)
				{
					fromPoint.x += stepX;
					fraction -= dy;
				}

				fromPoint.y += stepY;
				fraction += dx;

				parallax += parallaxStep;

				if(((unsigned)(fromPoint.x - _cameraFrustum->x0) < (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
					&&
					((unsigned)(fromPoint.y - _cameraFrustum->y0) < (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0)))
				{
					DirectDraw_drawPixel(this, leftBuffer, fromPoint.x - parallax, fromPoint.y, color);
					DirectDraw_drawPixel(this, rightBuffer, fromPoint.x + parallax, fromPoint.y, color);
				}
			}
		}
	}
}
