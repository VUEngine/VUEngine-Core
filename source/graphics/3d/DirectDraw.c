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
DirectDraw _directDraw = NULL;

#define	__DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS				10000
#define __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_OVERHEAD		100
#define __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY		1
#define __FRAME_BUFFER_SIDE_BIT_INDEX						16
#define __FRAME_BUFFER_SIDE_BIT								__RIGHT_FRAME_BUFFER_0
#define __FLIP_FRAME_BUFFER_SIDE_BIT(a)						a ^= __FRAME_BUFFER_SIDE_BIT
#define __FRAME_BUFFERS_SIZE								0x6000

enum DirectDrawLineShrinkingResult
{
	kDirectDrawLineShrinkingInvalid = 0,
	kDirectDrawLineShrinkingNone,
	kDirectDrawLineShrinkingSafe,
	kDirectDrawLineShrinkingUnsafe
};

static CameraFrustum _frustum;
static RightBox _frustumFixedPoint;

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

	DirectDraw::setFrustum(this, (CameraFrustum)
	{
		0, 0, 0, __SCREEN_WIDTH - 1, __SCREEN_HEIGHT - 1, 8191
	});

	this->totalDrawPixels = 0;
	this->maximuDrawPixels = 0;

	VIPManager::addEventListener(VIPManager::getInstance(), Object::safeCast(this), (EventListener)DirectDraw::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);

	_directDraw = this;
}

/**
 * Class destructor
 */
void DirectDraw::destructor()
{
	// allow a new construct
	Base::destructor();
}

void DirectDraw::onVIPManagerGAMESTARTDuringXPEND(Object eventFirer __attribute__ ((unused)))
{
	this->maximuDrawPixels = this->totalDrawPixels - __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_OVERHEAD;
}

/**
 * Reset
 */
void DirectDraw::reset()
{
	this->maximuDrawPixels = __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS;
}

/**
 * Reset
 */
void DirectDraw::startDrawing()
{
#ifdef __PROFILE_DIRECT_DRAWING
	static int counter = 0;

	if(__TARGET_FPS <= counter++)
	{
		PRINT_TEXT("Total pixels:       ", 1, 27);
		PRINT_INT(this->totalDrawPixels, 14, 27);
		counter = 0;
	}
#endif

	this->totalDrawPixels = 0;

	_directDraw->maximuDrawPixels += __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY;
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
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax, int32 color)
{
	uint16 yHelper = y >> 2;
	uint8 pixel = color << ((y & 3) << 1);

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = (((x - parallax) << 6) + yHelper);

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		leftBuffer[displacement] |= pixel;
	}

	displacement = (((x + parallax) << 6) + yHelper);

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		rightBuffer[displacement] |= pixel;
	}

	_directDraw->totalDrawPixels++;
}

static void DirectDraw::drawColorPixelUnsafe(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax, int32 color)
{
	uint16 yHelper = y >> 2;
	uint8 pixel = color << ((y & 3) << 1);

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	leftBuffer += (((x - parallax) << 6) + yHelper);
	rightBuffer += (((x + parallax) << 6) + yHelper);

	*leftBuffer |= pixel;
	*rightBuffer |= pixel;

	_directDraw->totalDrawPixels++;
}

#else
static void DirectDraw::drawColorPixel(BYTE* buffer, BYTE* dummyBuffer __attribute__((unused)), int16 x, int16 y, int16 parallax, int32 color)
{
	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = (((x - parallax) << 6) + (y >> 2));

	if((unsigned)__FRAME_BUFFERS_SIZE <= (unsigned)displacement)
	{
		return;
	}

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	buffer[displacement] |= color << ((y & 3) << 1);

	_directDraw->totalDrawPixels++;
}

static void DirectDraw::drawColorPixelUnsafe(BYTE* buffer, BYTE* dummyBuffer __attribute__((unused)), int16 x, int16 y, int16 parallax, int32 color)
{
	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	buffer += (((x - parallax) << 6) + (y >> 2));

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	*buffer |= color << ((y & 3) << 1);

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

	if((unsigned)(point.x - point.parallax - _frustum.x0) < (unsigned)(_frustum.x1 - _frustum.x0)
		&&
		(unsigned)(point.y - _frustum.y0) < (unsigned)(_frustum.y1 - _frustum.y0)
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
	if((unsigned)(point.x + point.parallax - _frustum.x0) < (unsigned)(_frustum.x1 - _frustum.x0)
		&&
		(unsigned)(point.y - _frustum.y0) < (unsigned)(_frustum.y1 - _frustum.y0)
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

		if((unsigned)(fromPointY - __I_TO_FIX19_13(_frustum.y0)) < (unsigned)(__I_TO_FIX19_13(_frustum.y1) - __I_TO_FIX19_13(_frustum.y0)))
		{
			if((unsigned)(fromPointX - parallax - __I_TO_FIX19_13(_frustum.x0)) < (unsigned)(__I_TO_FIX19_13(_frustum.x1) - __I_TO_FIX19_13(_frustum.x0)))
			{
				drawPixelMethod(leftBuffer, (uint16)__FIX19_13_TO_I(fromPointX - parallax), (uint16)__FIX19_13_TO_I(fromPointY), color);
			}

			if((unsigned)(fromPointX + parallax - __I_TO_FIX19_13(_frustum.x0)) < (unsigned)(__I_TO_FIX19_13(_frustum.x1) - __I_TO_FIX19_13(_frustum.x0)))
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

static uint32 DirectDraw::shrinkLineToScreenSpace(fix10_6* x0, fix10_6* y0, fix10_6* parallax0, fix10_6 dx, fix10_6 dy, fix10_6 dParallax, fix10_6 x1, fix10_6 y1, fix10_6 parallax1)
{
	fix10_6 x = *x0;
	fix10_6 y = *y0;
	fix10_6 parallax = *parallax0;

	fix10_6 width = _frustumFixedPoint.x1 - _frustumFixedPoint.x0 + __I_TO_FIX10_6(1);
	fix10_6 height = _frustumFixedPoint.y1 - _frustumFixedPoint.y0 + __I_TO_FIX10_6(1);

	if((unsigned)width < (unsigned)(x - parallax - _frustumFixedPoint.x0)
		|| (unsigned)width < (unsigned)(x + parallax - _frustumFixedPoint.x0)
		|| (unsigned)height < (unsigned)(y - _frustumFixedPoint.y0)
	)
	{
		if(0 == dx)
		{
			if((unsigned)width < (unsigned)(x - parallax - _frustumFixedPoint.x0) && (unsigned)width < (unsigned)(x + parallax - _frustumFixedPoint.x0))
			{
				return kDirectDrawLineShrinkingInvalid;
			}

			if(_frustumFixedPoint.y0 > y)
			{
				y = _frustumFixedPoint.y0;
			}
			else if(_frustumFixedPoint.y1 < y)
			{
				y = _frustumFixedPoint.y1;
			}

			*y0 = y;

			return 0 == parallax ? kDirectDrawLineShrinkingSafe : kDirectDrawLineShrinkingUnsafe;
		}

		if(0 == dy)
		{
			if((unsigned)height < (unsigned)(y - _frustumFixedPoint.y0))
			{
				return kDirectDrawLineShrinkingInvalid;
			}

			if(0 == parallax)
			{
				if(_frustumFixedPoint.x0 > x)
				{
					x = _frustumFixedPoint.x0;
				}
				else if(_frustumFixedPoint.x1 < x)
				{
					x = _frustumFixedPoint.x1;
				}

				*x0 = x;

				return kDirectDrawLineShrinkingSafe;
			}

			parallax = __ABS(parallax);

			if(_frustumFixedPoint.x0 > x + parallax)
			{
				x = _frustumFixedPoint.x0 - parallax;
			}
			else if(_frustumFixedPoint.x1 < x - parallax)
			{
				x = _frustumFixedPoint.x1 + parallax;
			}

			*x0 = x;

			return kDirectDrawLineShrinkingUnsafe;
		}

		NM_ASSERT(0 != dx, "DirectDraw::shrinkLineToScreenSpace: dx = 0");

		fix19_13 xySlope = __FIX19_13_DIV(__FIX10_6_TO_FIX19_13(dy), __FIX10_6_TO_FIX19_13(dx));
		fix10_6 yParallaxSlope = __FIX10_6_DIV(dParallax, dy);

		if(0 == xySlope)
		{
			return kDirectDrawLineShrinkingUnsafe;
		}

		//(x0 - x1) / dx = (y0 - y1) / dy = (parallax0 - parallax1) / dParallax
		fix10_6 parallaxHelper0 = parallax;
		fix10_6 parallaxHelper1 = parallax1;

		fix10_6 xHelper = x;

		bool shrinkOnXAxis = false;

		if(0 > x)
		{
			// Do not shrking the line if one of the x coordinates (left of right eye) is inside the screen space
			if(_frustumFixedPoint.x0 > x - parallax && _frustumFixedPoint.x0 > x + parallax)
			{
				x = _frustumFixedPoint.x0;

				if(0 > parallax)
				{
					parallaxHelper0 = -parallaxHelper0;
					parallaxHelper1 = -parallaxHelper1;
				}

				shrinkOnXAxis = true;
			}
		}
		else if(width < x)
		{
			// Do not shrking the line if one of the x coordinates (left of right eye) is inside the screen space
			if(_frustumFixedPoint.x1 < x - parallax && _frustumFixedPoint.x1 < x + parallax)
			{
				x = _frustumFixedPoint.x1;

				if(0 < parallax)
				{
					parallaxHelper0 = -parallaxHelper0;
					parallaxHelper1 = -parallaxHelper1;
				}

				shrinkOnXAxis = true;
			}
		}

		if(shrinkOnXAxis)
		{
			// This will compute the y coordinate at which the line for the relevant display (left or right)
			// intersects the screen's left or right border and uses the result to compute the x coordinate 
			// of the actual line
			xHelper += parallaxHelper0;

			fix10_6 dxHelper = (x1 + parallaxHelper1) - xHelper;

			if(0 == dxHelper)
			{
				y = __FIX19_13_TO_FIX10_6(__FIX19_13_MULT(__FIX10_6_TO_FIX19_13(x - x1), xySlope)) + y1;
				parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;

				*x0 = x;
				*y0 = y;
				*parallax0 = parallax;

				return kDirectDrawLineShrinkingUnsafe;
			}

			fix10_6 xySlopeHelper = __FIX10_6_DIV(dy, dxHelper);

			if(0 == xySlopeHelper)
			{
				return kDirectDrawLineShrinkingUnsafe;
			}
			
			if(0 > xySlope * xySlopeHelper)
			{
				xySlopeHelper = -xySlopeHelper;
			}

			y = __FIX10_6_MULT(xySlopeHelper, x - (x1 + parallaxHelper1)) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
			x = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX10_6_TO_FIX19_13(y - y1), xySlope)) + x1;
		}

		if(_frustumFixedPoint.y0 > y)
		{
			// x = (y - y1)/(dx/dy) + x1
			y = _frustumFixedPoint.y0;
			x = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX10_6_TO_FIX19_13(-y1), xySlope)) + x1;
			parallax = __FIX10_6_MULT(yParallaxSlope, -y1) + parallax1;
		}
		else if(_frustumFixedPoint.y1 < y)
		{
			// x = (y - y1)/(dx/dy) + x1
			y = _frustumFixedPoint.y1;
			x = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX10_6_TO_FIX19_13(y - y1), xySlope)) + x1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
		}

		*x0 = x;
		*y0 = y;
		*parallax0 = parallax;

		return 0 != parallax ? kDirectDrawLineShrinkingUnsafe : kDirectDrawLineShrinkingSafe;
	}

	return kDirectDrawLineShrinkingNone;
}

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, int32 clampLimit __attribute__((unused)), uint8 bufferIndex __attribute__((unused)))
{
	if(0 == clampLimit || __FIX10_6_MAXIMUM_VALUE_TO_I < clampLimit)
	{
		fromPoint = DirectDraw::clampPixelVector(fromPoint);
		toPoint = DirectDraw::clampPixelVector(toPoint);
	}

#ifndef __DIRECT_DRAW_INTERLACED
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
	uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;
#else
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
	uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;
#endif


	uint16 totalPixels = 0;

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

	fix10_6 fromPointParallax = __I_TO_FIX10_6(fromPoint.parallax);
	fix10_6 toPointParallax = __I_TO_FIX10_6(toPoint.parallax);

	if(!((unsigned)(_frustum.x1 - _frustum.x0) >= (unsigned)(fromPoint.x - fromPoint.parallax - _frustum.x0) || (unsigned)(_frustum.x1 - _frustum.x0) >= (unsigned)(fromPoint.x + fromPoint.parallax - _frustum.x0) ||
	(unsigned)(_frustum.x1 - _frustum.x0) >= (unsigned)(toPoint.x - toPoint.parallax - _frustum.x0) || (unsigned)(_frustum.x1 - _frustum.x0) >= (unsigned)(toPoint.x + toPoint.parallax - _frustum.x0)))
	{
		return;
	}

	if((unsigned)(_frustum.y1 - _frustum.y0) <= (unsigned)(fromPoint.y - _frustum.y0) && (unsigned)(_frustum.y1 - _frustum.y0) <= (unsigned)(toPoint.y - _frustum.y0))
	{
		return;
	}

	if((unsigned)(_frustum.z1 - _frustum.z0) <= (unsigned)(fromPoint.z - _frustum.z0) && (unsigned)(_frustum.z1 - _frustum.z0) <= (unsigned)(toPoint.z - _frustum.z0))
	{
		return;
	}

	fix10_6 dParallax = (toPointParallax - fromPointParallax);

	bool useUnsafeDrawPixel = true;

	switch(DirectDraw::shrinkLineToScreenSpace(&fromPointX, &fromPointY, &fromPointParallax, dx, dy, dParallax, toPointX, toPointY, toPointParallax))
	{
		case kDirectDrawLineShrinkingInvalid:

			return;

		case kDirectDrawLineShrinkingUnsafe:

			useUnsafeDrawPixel = false;
			break;
	}

	switch(DirectDraw::shrinkLineToScreenSpace(&toPointX, &toPointY, &toPointParallax, dx, dy, dParallax, fromPointX, fromPointY, fromPointParallax))
	{
		case kDirectDrawLineShrinkingInvalid:

			return;

		case kDirectDrawLineShrinkingUnsafe:

			useUnsafeDrawPixel = false;
			break;
	}

	fix10_6 dxABS = __ABS(dx);
	fix10_6 dyABS = __ABS(dy);

	fix10_6 parallaxStart = fromPointParallax;

	if(dyABS == dxABS || dyABS < dxABS || 0 == dy)
	{
		totalPixels = __ABS(toPoint.x - fromPoint.x);

		if(_directDraw->totalDrawPixels + totalPixels > _directDraw->maximuDrawPixels)
		{
			return;
		}

		if(toPointX < fromPointX)
		{
			fix10_6 auxPoint = fromPointX;
			fromPointX = toPointX;
			toPointX = auxPoint;

			dxABS = -dxABS;
			fromPointY = toPointY;

			parallaxStart = toPointParallax; 
		}

		fix10_6 secondaryStep = __FIX10_6_DIV(dy, dxABS);
		fix10_6 parallaxStep = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX10_6_TO_FIX19_13(dParallax), __FIX10_6_TO_FIX19_13(dxABS)));

#ifdef __DIRECT_DRAW_INTERLACED
		if(bufferIndex)
		{
			parallaxStart = -parallaxStart;
			parallaxStep = -parallaxStep;
		}
#endif
		int16 mainCoordinateStart = __FIX10_6_TO_I(fromPointX);
		int16 mainCoordinateEnd = __FIX10_6_TO_I(toPointX);
		fix10_6 secondaryStart = fromPointY;

		int16 mainCoordinate = mainCoordinateStart;
		fix10_6 secondaryCoordinate = secondaryStart;
		fix10_6 parallax = parallaxStart;

		if(useUnsafeDrawPixel)
		{
			CACHE_ENABLE;

	#ifndef __DIRECT_DRAW_INTERLACED
			for(;mainCoordinateEnd >= mainCoordinate;
	#else
			for(;mainCoordinateEnd - 1 >= mainCoordinate;
	#endif
				mainCoordinate += 1,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixelUnsafe((BYTE*)leftBuffer, (BYTE*)rightBuffer, mainCoordinate, secondaryHelper, parallaxHelper0, color);

	#ifdef __DIRECT_DRAW_INTERLACED
				mainCoordinate += 1;
				secondaryCoordinate += secondaryStep;
				parallax += parallaxStep;

				secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixelUnsafe((BYTE*)rightBuffer, (BYTE*)leftBuffer, mainCoordinate, secondaryHelper, -parallaxHelper0, color);
	#endif
			}			
		}
		else
		{
			CACHE_ENABLE;

	#ifndef __DIRECT_DRAW_INTERLACED
			for(;mainCoordinateEnd >= mainCoordinate;
	#else
			for(;mainCoordinateEnd - 1 >= mainCoordinate;
	#endif
				mainCoordinate += 1,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, mainCoordinate, secondaryHelper, parallaxHelper0, color);

	#ifdef __DIRECT_DRAW_INTERLACED
				mainCoordinate += 1;
				secondaryCoordinate += secondaryStep;
				parallax += parallaxStep;

				secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixel((BYTE*)rightBuffer, (BYTE*)leftBuffer, mainCoordinate, secondaryHelper, -parallaxHelper0, color);
	#endif
			}
		}
	}
	else if(dxABS < dyABS || 0 == dx)
	{
		totalPixels = __ABS(toPoint.y - fromPoint.y);

		if(_directDraw->totalDrawPixels + totalPixels > _directDraw->maximuDrawPixels)
		{
			return;
		}

		if(toPointY < fromPointY)
		{
			fix10_6 auxPoint = fromPointY;
			fromPointY = toPointY;
			toPointY = auxPoint;

			dyABS = -dyABS;
			fromPointX = toPointX;

			parallaxStart = toPointParallax; 
		}

		fix10_6 secondaryStep = __FIX10_6_DIV(dx, dyABS);
		fix10_6 parallaxStep = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX10_6_TO_FIX19_13(dParallax), __FIX10_6_TO_FIX19_13(dyABS)));

#ifdef __DIRECT_DRAW_INTERLACED
		if(bufferIndex)
		{
			parallaxStart = -parallaxStart;
			parallaxStep = -parallaxStep;
		}
#endif
		int16 mainCoordinateStart = __FIX10_6_TO_I(fromPointY);
		int16 mainCoordinateEnd = __FIX10_6_TO_I(toPointY);
		fix10_6 secondaryStart = fromPointX;

		int16 mainCoordinate = mainCoordinateStart;
		fix10_6 secondaryCoordinate = secondaryStart;
		fix10_6 parallax = parallaxStart;

		if(useUnsafeDrawPixel)
		{
			CACHE_ENABLE;

	#ifndef __DIRECT_DRAW_INTERLACED
			for(;mainCoordinateEnd >= mainCoordinate;
	#else
			for(;mainCoordinateEnd - 1 >= mainCoordinate;
	#endif
				mainCoordinate += 1,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixelUnsafe((BYTE*)leftBuffer, (BYTE*)rightBuffer, secondaryHelper, mainCoordinate, parallaxHelper0, color);

	#ifdef __DIRECT_DRAW_INTERLACED
				mainCoordinate += 1;
				secondaryCoordinate += secondaryStep;
				parallax += parallaxStep;

				secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixelUnsafe((BYTE*)rightBuffer, (BYTE*)leftBuffer, secondaryHelper, mainCoordinate, -parallaxHelper0, color);
	#endif
			}
		}
		else
		{
			CACHE_ENABLE;

	#ifndef __DIRECT_DRAW_INTERLACED
			for(;mainCoordinateEnd >= mainCoordinate;
	#else
			for(;mainCoordinateEnd - 1 >= mainCoordinate;
	#endif
				mainCoordinate += 1,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, secondaryHelper, mainCoordinate, parallaxHelper0, color);

	#ifdef __DIRECT_DRAW_INTERLACED
				mainCoordinate += 1;
				secondaryCoordinate += secondaryStep;
				parallax += parallaxStep;

				secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				parallaxHelper0 = __FIX10_6_TO_I(parallax);

				DirectDraw::drawColorPixel((BYTE*)rightBuffer, (BYTE*)leftBuffer, secondaryHelper, mainCoordinate, -parallaxHelper0, color);
	#endif
			}
		}
	}

	CACHE_DISABLE;
}

void DirectDraw::setFrustum(CameraFrustum frustum)
{
	if(frustum.x1 > __SCREEN_WIDTH)
	{
		frustum.x1 = __SCREEN_WIDTH - 1;
	}

	if(frustum.y1 > __SCREEN_HEIGHT)
	{
		frustum.y1 = __SCREEN_HEIGHT - 1;
	}

	// 9: 2's power equal to the math type fix10_6
	if(frustum.z1 > (1 << (9 + __PIXELS_PER_METER_2_POWER)))
	{
		frustum.z1 = 1;
	}

	if(frustum.x0 > frustum.x1)
	{
		frustum.x0 = frustum.x1 - 1;
	}

	if(frustum.y0 > frustum.y1)
	{
		frustum.y0 = frustum.y1 - 1;
	}

	if(frustum.z0 > frustum.z1)
	{
		frustum.z0 = frustum.z1 - 1;
	}

	_frustum = frustum;

	_frustumFixedPoint = (RightBox)
	{
		__I_TO_FIX10_6(_frustum.x0), __I_TO_FIX10_6(_frustum.y0), __I_TO_FIX10_6(_frustum.z0),
		__I_TO_FIX10_6(_frustum.x1), __I_TO_FIX10_6(_frustum.y1), __I_TO_FIX10_6(_frustum.z1),
	};
}