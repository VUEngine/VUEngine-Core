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
		PRINT_TEXT("Total pixels:    /      ", 1, 27);
		PRINT_INT(this->totalDrawPixels, 14, 27);
		PRINT_INT(this->maximuDrawPixels, 19, 27);
		counter = 0;
	}
#endif

	this->totalDrawPixels = 0;

	_directDraw->maximuDrawPixels += __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY;
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

static void DirectDraw::drawColorPixelInterlaced(BYTE* buffer, int16 x, int16 y, int16 parallax, int32 color)
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
static void DirectDraw::drawBlackPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax)
{
	uint16 yHelper = y >> 2;
	uint8 pixel = ~(0b11 << ((y & 3) << 1));

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = (((x - parallax) << 6) + yHelper);

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		leftBuffer[displacement] &= pixel;
	}

	displacement = (((x + parallax) << 6) + yHelper);

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		rightBuffer[displacement] &= pixel;
	}

	_directDraw->totalDrawPixels++;
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

	if(color == __COLOR_BLACK)
	{
		DirectDraw::drawBlackPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, point.x, point.y, point.parallax);
	}
	else
	{
		DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, point.x, point.y, point.parallax, color);
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

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, int32 clampLimit, uint8 bufferIndex, bool interlaced)
{
	if(0 == clampLimit || __FIX10_6_MAXIMUM_VALUE_TO_I < clampLimit)
	{
		fromPoint = DirectDraw::clampPixelVector(fromPoint);
		toPoint = DirectDraw::clampPixelVector(toPoint);
	}

	if((unsigned)(_frustum.x1 - _frustum.x0) <= (unsigned)(fromPoint.x - _frustum.x0) && (unsigned)(_frustum.x1 - _frustum.x0) <= (unsigned)(toPoint.x - _frustum.x0))
	{
		return;
	}

	if((unsigned)(_frustum.y1 - _frustum.y0) <= (unsigned)(fromPoint.y - _frustum.y0) && (unsigned)(_frustum.y1 - _frustum.y0) <= (unsigned)(toPoint.y - _frustum.y0))
	{
		return;
	}

	if((unsigned)(_frustum.z1 - _frustum.z0) <= (unsigned)(fromPoint.z - _frustum.z0) || (unsigned)(_frustum.z1 - _frustum.z0) <= (unsigned)(toPoint.z - _frustum.z0))
	{
		return;
	}

	uint32 leftBuffer = *_currentDrawingFrameBufferSet | (interlaced ? (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX) : __LEFT_FRAME_BUFFER_0);
	uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

	fix8_8_ext fromPointX = __I_TO_FIX8_8_EXT(fromPoint.x);
	fix8_8_ext fromPointY = __I_TO_FIX8_8_EXT(fromPoint.y);

	fix8_8_ext toPointX = __I_TO_FIX8_8_EXT(toPoint.x);
	fix8_8_ext toPointY = __I_TO_FIX8_8_EXT(toPoint.y);

	fix8_8_ext dx = toPointX - fromPointX;
	fix8_8_ext dy = toPointY - fromPointY;

	if(0 == dx && 0 == dy)
	{
		return;
	}

	fix8_8_ext fromPointParallax = __I_TO_FIX8_8_EXT(fromPoint.parallax);
	fix8_8_ext toPointParallax = __I_TO_FIX8_8_EXT(toPoint.parallax);
	fix8_8_ext dParallax = (toPointParallax - fromPointParallax);

	fix8_8_ext dxABS = __ABS(dx);
	fix8_8_ext dyABS = __ABS(dy);

	fix8_8_ext parallaxStart = fromPointParallax;

	fix8_8_ext xStep = __1I_FIX8_8;
	fix8_8_ext yStep = __1I_FIX8_8;
	fix8_8_ext parallaxStep = 0;
	
	fix8_8_ext mainCoordinateStart = 0;
	fix8_8_ext mainCoordinateEnd = 0;

	if(dyABS == dxABS || dyABS < dxABS || 0 == dy)
	{
		if(toPointX < fromPointX)
		{
			fix8_8_ext auxPoint = fromPointX;
			fromPointX = toPointX;
			toPointX = auxPoint;

			dxABS = -dxABS;
			fromPointY = toPointY;

			parallaxStart = toPointParallax; 
		}

		yStep = __FIX8_8_EXT_DIV(dy, dxABS);
		parallaxStep = __FIX8_8_EXT_DIV(dParallax, dxABS);

		mainCoordinateStart = fromPointX;
		mainCoordinateEnd = toPointX;
	}
	else if(dxABS < dyABS || 0 == dx)
	{
		if(toPointY < fromPointY)
		{
			fix8_8_ext auxPoint = fromPointY;
			fromPointY = toPointY;
			toPointY = auxPoint;

			dyABS = -dyABS;
			fromPointX = toPointX;

			parallaxStart = toPointParallax; 
		}

		xStep = __FIX8_8_EXT_DIV(dx, dyABS);
		parallaxStep = __FIX8_8_EXT_DIV(dParallax, dyABS);

		mainCoordinateStart = fromPointY;
		mainCoordinateEnd = toPointY;
	}

	fix8_8_ext totalPixels = __ABS(mainCoordinateEnd - mainCoordinateStart);

	if(_directDraw->totalDrawPixels + __FIX8_8_EXT_TO_I(totalPixels) > _directDraw->maximuDrawPixels)
	{
		return;
	}

	if(0 != bufferIndex)
	{
		parallaxStart = -parallaxStart;
		parallaxStep = -parallaxStep;
	}

	if(interlaced)
	{
		for(fix8_8_ext mainCoordinate = mainCoordinateStart; mainCoordinateEnd - __1I_FIX8_8 >= mainCoordinate;
			mainCoordinate += __1I_FIX8_8,
			fromPointX += xStep,
			fromPointY += yStep,
			parallaxStart += parallaxStep
		)
		{
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), __FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);

			mainCoordinate += __1I_FIX8_8;
			fromPointX += xStep;
			fromPointY += yStep;
			parallaxStart += parallaxStep;

			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), -__FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);
		}
	}
	else
	{
		for(fix8_8_ext mainCoordinate = mainCoordinateStart; mainCoordinateEnd >= mainCoordinate;
			mainCoordinate += __1I_FIX8_8,
			fromPointX += xStep,
			fromPointY += yStep,
			parallaxStart += parallaxStep
		)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), __FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);
		}
	}
}

