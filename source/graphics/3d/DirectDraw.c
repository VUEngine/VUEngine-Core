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
#define __PROFILE_DIRECT_DRAWING
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
#define __DIRECT_DRAW_STRICT_BUFFER_CHECK
#define __DIRECT_DRAW_INTERLACED
#ifndef __DIRECT_DRAW_INTERLACED
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax, int32 color)
{
	uint16 yHelper = y >> 2;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = (((x - parallax) << 6) + yHelper);

#ifdef __DIRECT_DRAW_STRICT_BUFFER_CHECK
	if((unsigned)0x6000 <= (unsigned)displacement)
	{
//		PRINT_INT(x, 30, 0);
//		PRINT_INT(x + parallax, 30, 1);
//		PRINT_TIME(30, 2);
		return;
	}
#endif

	uint8 pixel = color << ((y & 3) << 1);
	leftBuffer[displacement] |= pixel;

	displacement = (((x + parallax) << 6) + yHelper);

#ifdef __DIRECT_DRAW_STRICT_BUFFER_CHECK
	if((unsigned)0x6000 <= (unsigned)displacement)
	{
//		PRINT_INT(x, 40, 0);
//		PRINT_INT(x + parallax, 40, 1);
//		PRINT_TIME(40, 2);
		return;
	}
#endif

	rightBuffer[displacement] |= pixel;

	_directDraw->totalDrawPixels++;
}
#else
static void DirectDraw::drawColorPixel(BYTE* buffer, BYTE* dummyBuffer __attribute__((unused)), int16 x, int16 y, int16 parallax, int32 color)
{
	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = (((x - parallax) << 6) + (y >> 2));

#ifdef __DIRECT_DRAW_STRICT_BUFFER_CHECK
	if((unsigned)0x6000 <= (unsigned)displacement)
	{
//		PRINT_INT(x, 40, 0);
//		PRINT_INT(x + parallax, 40, 1);
//		PRINT_TIME(40, 2);
		return;
	}
#endif

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	buffer[displacement] |= color << ((y & 3) << 1);

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

static bool DirectDraw::reduceToScreenSpace(fix10_6* x0, fix10_6* y0, fix10_6* parallax0, fix10_6 dx, fix10_6 dy, fix10_6 dParallax, fix10_6 x1, fix10_6 y1, fix10_6 parallax1)
{
	fix10_6 x = *x0;
	fix10_6 y = *y0;
	fix10_6 parallax = *parallax0;

	fix10_6 width = __I_TO_FIX10_6(__SCREEN_WIDTH - 1);
	fix10_6 height = __I_TO_FIX10_6(__SCREEN_HEIGHT - 1);

	if((unsigned)width < (unsigned)(x - parallax)
		|| (unsigned)width < (unsigned)(x + parallax)
		|| (unsigned)height < (unsigned)(y)
	)
	{
		if(0 == dx)
		{
			if((unsigned)width < (unsigned)(x - parallax))
			{
				return false;
			}

			if((unsigned)width < (unsigned)(x + parallax))
			{
				return false;
			}

			if(0 > y)
			{
				y = 0;
			}
			else if(height < y)
			{
				y = height;
			}

			*y0 = y;

			return true;
		}

		if(0 == dy)
		{
			if((unsigned)height < (unsigned)(y))
			{
				return false;
			}

			if(0 > x - parallax)
			{
				x = parallax;
			}
			else if(width < x - parallax)
			{
				x = width + parallax;
			}

			if(0 > x + parallax)
			{
				x = -parallax;
			}
			else if(width < x + parallax)
			{
				x = width - parallax;
			}

			*x0 = x;

			return true;
		}

		fix10_6 xySlope = __FIX10_6_DIV(dy, dx);
		fix10_6 xParallaxSlope = __FIX10_6_DIV(dParallax, dx);
		fix10_6 yParallaxSlope = __FIX10_6_DIV(dParallax, dy);

		if(0 == xySlope)
		{
			xySlope = 1;
		}

		//(x0 - x1) / dx = (y0 - y1) / dy = (parallax0 - parallax1) / dParallax

		if(0 > y)
		{
			// x = (y - y1)/(dx/dy) + x1
			y = 0;
			x = __FIX10_6_DIV(y - y1, xySlope) + x1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
		}
		else if(height < y)
		{
			// x = (y - y1)/(dx/dy) + x1
			y = height;
			x = __FIX10_6_DIV(y - y1, xySlope) + x1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
		}

		if(0 > x)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = 0;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
		}
		else if(width < x)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = width;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
		}

		//	(y - y1) = (dx/dy)*(x - x1)
/*		if(0 > x - parallax)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = 0;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
			x = parallax;
		}
		else if(width < x - parallax)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = width;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
			x = width + parallax;
		}
		else if(0 > x + parallax)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = 0;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
			x = -parallax;
		}
		else if(width < x + parallax)
		{
			// y = (dx/dy)*(x - x1) + y1
			x = width;
			y = __FIX10_6_MULT(xySlope, x - x1) + y1;
			parallax = __FIX10_6_MULT(yParallaxSlope, y - y1) + parallax1;
			x = width - parallax;
		}
		*/
	}

	*x0 = x;
	*y0 = y;
	*parallax0 = parallax;

	return true;
}

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, int32 clampLimit __attribute__((unused)), uint8 bufferIndex __attribute__((unused)))
{
	if(0 == clampLimit || __FIX10_6_MAXIMUM_VALUE_TO_I < clampLimit)
	{
		fromPoint = DirectDraw::clampPixelVector(fromPoint);
		toPoint = DirectDraw::clampPixelVector(toPoint);
	}

	if((unsigned)__SCREEN_WIDTH <= (unsigned)(fromPoint.x) && (unsigned)__SCREEN_WIDTH <= (unsigned)(toPoint.x))
	{
		return;
	}

	if((unsigned)__SCREEN_HEIGHT <= (unsigned)(fromPoint.y) && (unsigned)__SCREEN_HEIGHT <= (unsigned)(toPoint.y))
	{
		return;
	}

	if((unsigned)(1 << (__PIXELS_PER_METER_2_POWER + _optical->maximumXViewDistancePower)) < (unsigned)fromPoint.z)
	{
		return;
	}

	if((unsigned)(1 << (__PIXELS_PER_METER_2_POWER + _optical->maximumXViewDistancePower)) < (unsigned)toPoint.z)
	{
		return;
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

	fix10_6 fromPointXHelper = fromPointX;
	fix10_6 fromPointYHelper = fromPointY;
	fix10_6 fromPointParallaxHelper = fromPointParallax;

	fix10_6 dParallax = (toPointParallax - fromPointParallax);

//	PixelVector::print(fromPoint, 1, 20);
//	PixelVector::print(toPoint, 10, 20);

/*
	if(!DirectDraw::reduceToScreenSpace(&fromPointX, &fromPointY, &fromPointParallax, dx, dy, dParallax, toPointX, toPointY, toPointParallax))
	{
		return;
	}

	if(!DirectDraw::reduceToScreenSpace(&toPointX, &toPointY, &toPointParallax, dx, dy, dParallax, fromPointXHelper, fromPointYHelper, fromPointParallaxHelper))
	{
		return;
	}
	*/

/*
	PRINT_INT(__FIX10_6_TO_I(fromPointX), 21, 20);
	PRINT_INT(__FIX10_6_TO_I(fromPointY), 21, 21);
	PRINT_INT(__FIX10_6_TO_I(fromPointParallax), 21, 23);

	PRINT_INT(__FIX10_6_TO_I(toPointX), 31, 20);
	PRINT_INT(__FIX10_6_TO_I(toPointY), 31, 21);
	PRINT_INT(__FIX10_6_TO_I(toPointParallax), 31, 23);
*/
	fix10_6 dxABS = __ABS(dx);
	fix10_6 dyABS = __ABS(dy);

	fix10_6 parallaxStart = __I_TO_FIX10_6(fromPoint.parallax);

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

			parallaxStart = __I_TO_FIX10_6(toPoint.parallax); 
		}

		fix10_6 secondaryStep = __FIX10_6_DIV(dy, dxABS);
		fix10_6 parallaxStep = __FIX10_6_DIV(dParallax, dxABS);

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
			int16 parallaxHelper = __FIX10_6_TO_I(parallax);

			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, mainCoordinate, secondaryHelper, parallaxHelper, color);

#ifdef __DIRECT_DRAW_INTERLACED
			mainCoordinate += 1;
			secondaryCoordinate += secondaryStep;
			parallax += parallaxStep;

			secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
			parallaxHelper = __FIX10_6_TO_I(parallax);

			DirectDraw::drawColorPixel((BYTE*)rightBuffer, (BYTE*)leftBuffer, mainCoordinate, secondaryHelper, -parallaxHelper, color);
#endif
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
			parallaxStart = __I_TO_FIX10_6(toPoint.parallax); 
		}

		fix10_6 secondaryStep = __FIX10_6_DIV(dx, dyABS);
		fix10_6 parallaxStep = __FIX10_6_DIV(dParallax, dyABS);

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
			int16 parallaxHelper = __FIX10_6_TO_I(parallax);

			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, secondaryHelper, mainCoordinate, parallaxHelper, color);

#ifdef __DIRECT_DRAW_INTERLACED
			mainCoordinate += 1;
			secondaryCoordinate += secondaryStep;
			parallax += parallaxStep;

			secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
			parallaxHelper = __FIX10_6_TO_I(parallax);

			DirectDraw::drawColorPixel((BYTE*)rightBuffer, (BYTE*)leftBuffer, secondaryHelper, mainCoordinate, -parallaxHelper, color);
#endif
		}
	}

	CACHE_DISABLE;
}

