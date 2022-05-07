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

#define __MAXIMUM_PIXELS_PER_FRAME					(1125 << __FRAME_CYCLE)

#define __DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME		__MAXIMUM_PIXELS_PER_FRAME

#define __DIRECT_DRAW_INTERLACED

#ifdef __DIRECT_DRAW_INTERLACED
#undef __DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME
#define __DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME		(__MAXIMUM_PIXELS_PER_FRAME << 1)
#endif

#define __FRAME_BUFFER_SIDE_BIT_INDEX				16
#define __FRAME_BUFFER_SIDE_BIT						__RIGHT_FRAME_BUFFER_0
#define __FLIP_FRAME_BUFFER_SIDE_BIT(a)				a ^= __FRAME_BUFFER_SIDE_BIT


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

/**
 * Reset
 */
void DirectDraw::reset()
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
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax, int32 color)
{
	uint16 yHelper = y >> 2;

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	leftBuffer  += (((x - parallax) << 6) + yHelper);
	rightBuffer += (((x + parallax) << 6) + yHelper);

	uint8 pixel = color << ((y & 3) << 1);

	// draw the pixel
	*leftBuffer  |= pixel;
	*rightBuffer |= pixel;

	_directDraw->totalDrawPixels++;
}
#else
static void DirectDraw::drawColorPixel(BYTE* buffer, BYTE* dummyBuffer __attribute__((unused)), int16 x, int16 y, int16 parallax, int32 color)
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

enum DirectDrawTestPoint
{
	kDirectDrawTestPointOk = 0,
	kDirectDrawTestPointContinue,
	kDirectDrawTestPointBreak,
};

static uint8 DirectDraw::testPoint(int16 x, int16 y, int16 parallax, fix10_6 stepX, fix10_6 stepY)
{
	if(__SCREEN_WIDTH <= (unsigned)(x - parallax))
	{
		if(0 > x - parallax)
		{
			if(0 > stepX)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}

		}
		else
		{
			if(0 < stepX)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}
		}
	}
	else if(__SCREEN_WIDTH <= (unsigned)(x + parallax))
	{
		if(0 > x - parallax)
		{
			if(0 > stepX)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}

		}
		else
		{
			if(0 < stepX)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}
		}
	}

	if(__SCREEN_HEIGHT <= (unsigned)y)
	{
		if(0 > y)
		{
			if(0 > stepY)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}

		}
		else
		{
			if(0 < stepY)
			{
				return kDirectDrawTestPointBreak;
			}
			else
			{
				return kDirectDrawTestPointContinue;
			}
		}
	}

	return kDirectDrawTestPointOk;
}

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, int32 clampLimit __attribute__((unused)), uint8 bufferIndex __attribute__((unused)))
{
/*
	if(__DIRECT_DRAW_MAXIMUM_PIXELS_PER_FRAME < _directDraw->totalDrawPixels)
	{
		return;
	}
*/
/*
	if(0 == clampLimit || __FIX10_6_MAXIMUM_VALUE_TO_I < clampLimit)
	{
		fromPoint = DirectDraw::clampPixelVector(fromPoint);
		toPoint = DirectDraw::clampPixelVector(toPoint);
	}
*/
	if(__SCREEN_WIDTH <= (unsigned)(fromPoint.x) && __SCREEN_WIDTH <= (unsigned)(toPoint.x))
	{
		return;
	}

	if(__SCREEN_HEIGHT <= (unsigned)(fromPoint.y) && __SCREEN_HEIGHT <= (unsigned)(toPoint.y))
	{
		return;
	}

#ifndef __DIRECT_DRAW_INTERLACED
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
#else
	uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
#endif

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

	fix10_6 fromPointParallax = __I_TO_FIX10_6(fromPoint.parallax);
	fix10_6 toPointParallax = __I_TO_FIX10_6(toPoint.parallax);

	fix10_6 dParallax = (toPointParallax - fromPointParallax);

	fix10_6 dxABS = __ABS(dx);
	fix10_6 dyABS = __ABS(dy);

	fix10_6 parallaxStart = __I_TO_FIX10_6(fromPoint.parallax);

#ifndef __DIRECT_DRAW_INTERLACED
	uint16 stepFactor = 0;
#else
	uint16 stepFactor = 1;
#endif

	if(dyABS == dxABS || dyABS < dxABS || 0 == dy)
	{
		int16 mainCoordinateStep = (toPointX < fromPointX ? -1 : 1) << stepFactor;
		fix10_6 secondaryStep = __FIX10_6_DIV(dy, dxABS) << stepFactor;
		fix10_6 parallaxStep = __FIX10_6_DIV(dParallax, dxABS) << stepFactor;

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

		int16 displacement = 0;

		if(0 != __MODULO(__ABS(mainCoordinateEnd - mainCoordinateStart), 2))
		{
			displacement = -(mainCoordinateStep >> stepFactor);
		}

		CACHE_ENABLE;

#ifdef __DIRECT_DRAW_INTERLACED
		for(int16 buffers = 0; buffers < 2; buffers++)
		{
#endif		
			for(;mainCoordinateEnd + displacement != mainCoordinate;
				mainCoordinate += mainCoordinateStep,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper = __FIX10_6_TO_I(parallax);

				uint8 pointTest = DirectDraw::testPoint(mainCoordinate, secondaryHelper, parallaxHelper, mainCoordinateStep, secondaryStep);

				if(kDirectDrawTestPointContinue == pointTest)
				{
					continue;
				}
				else if(kDirectDrawTestPointBreak == pointTest)
				{
					break;
				}

				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, mainCoordinate, secondaryHelper, parallaxHelper, color);
			}

#ifdef __DIRECT_DRAW_INTERLACED
			leftBuffer ^= __FRAME_BUFFER_SIDE_BIT;
			
			mainCoordinate = mainCoordinateStart + (mainCoordinateStep >> stepFactor);
			secondaryCoordinate = secondaryStart + secondaryStep;
			parallaxStep = -parallaxStep;
			parallax = -parallaxStart + (parallaxStep >> stepFactor);
			displacement = 0;

			if(0 == __MODULO(__ABS(mainCoordinateEnd - mainCoordinateStart), 2))
			{
				displacement -= (mainCoordinateStep >> stepFactor);
			}
		}
#endif
	}
	else if(dxABS < dyABS || 0 == dx)
	{
		int16 mainCoordinateStep = (toPointY < fromPointY ? -1 : 1) << stepFactor;
		fix10_6 secondaryStep = __FIX10_6_DIV(dx, dyABS) << stepFactor;
		fix10_6 parallaxStep = __FIX10_6_DIV(dParallax, dyABS) << stepFactor;

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

		int16 displacement = 0;

		if(0 != __MODULO(__ABS(mainCoordinateEnd - mainCoordinateStart), 2))
		{
			displacement = -(mainCoordinateStep >> stepFactor);
		}

		CACHE_ENABLE;

#ifdef __DIRECT_DRAW_INTERLACED
		for(int16 buffers = 0; buffers < 2; buffers++)
		{
#endif		
			for(;mainCoordinateEnd + displacement != mainCoordinate;
				mainCoordinate += mainCoordinateStep,
				secondaryCoordinate += secondaryStep,
				parallax += parallaxStep
			)
			{
				int16 secondaryHelper = __FIX10_6_TO_I(secondaryCoordinate);
				int16 parallaxHelper = __FIX10_6_TO_I(parallax);

				uint8 pointTest = DirectDraw::testPoint(secondaryHelper, mainCoordinate, parallaxHelper, mainCoordinateStep, secondaryStep);

				if(kDirectDrawTestPointContinue == pointTest)
				{
					continue;
				}
				else if(kDirectDrawTestPointBreak == pointTest)
				{
					break;
				}

				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, secondaryHelper, mainCoordinate, parallaxHelper, color);
			}

#ifdef __DIRECT_DRAW_INTERLACED
			leftBuffer ^= __FRAME_BUFFER_SIDE_BIT;
			
			mainCoordinate = mainCoordinateStart + (mainCoordinateStep >> stepFactor);
			secondaryCoordinate = secondaryStart + secondaryStep;
			parallaxStep = -parallaxStep;
			parallax = -parallaxStart + (parallaxStep >> stepFactor);
			displacement = 0;

			if(0 == __MODULO(__ABS(mainCoordinateEnd - mainCoordinateStart), 2))
			{
				displacement -= (mainCoordinateStep >> stepFactor);
			}
		}
#endif
	}

	CACHE_DISABLE;
}

