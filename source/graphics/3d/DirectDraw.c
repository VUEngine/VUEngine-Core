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


typedef struct CustomCameraFrustum
{
	fix8_8_ext x0;
	fix8_8_ext y0;
	fix8_8_ext z0;
	fix8_8_ext x1;
	fix8_8_ext y1;
	fix8_8_ext z1;
} CustomCameraFrustum;

static CameraFrustum _frustum;
CustomCameraFrustum _frustumFixedPoint;
int16 _frustumWidth = __SCREEN_WIDTH;
int16 _frustumHeight = __SCREEN_HEIGHT;
int16 _frustumDepth = __SCREEN_DEPTH;


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

	if(this->totalDrawPixels < _directDraw->maximuDrawPixels)
	{
		_directDraw->maximuDrawPixels += __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY;
	}

	this->totalDrawPixels = 0;
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
	_frustumWidth = _frustum.x1 - _frustum.x0 + 1;
	_frustumHeight = _frustum.y1 - _frustum.y0 + 1;
	_frustumDepth = _frustum.z1 - _frustum.z0 + 1;

	_frustumFixedPoint = (CustomCameraFrustum)
	{
		__I_TO_FIX8_8_EXT(_frustum.x0), __I_TO_FIX8_8_EXT(_frustum.y0), __I_TO_FIX8_8_EXT(_frustum.z0),
		__I_TO_FIX8_8_EXT(_frustum.x1), __I_TO_FIX8_8_EXT(_frustum.y1), __I_TO_FIX8_8_EXT(_frustum.z1),
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
	int32 displacement = (x - parallax) << 6;

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		leftBuffer[displacement + yHelper] |= pixel;
	}

	displacement += (parallax << 7);

	if((unsigned)__FRAME_BUFFERS_SIZE > (unsigned)displacement)
	{
		rightBuffer[displacement + yHelper] |= pixel;
	}
}

static void DirectDraw::drawColorPixelInterlaced(BYTE* buffer, int16 x, int16 y, int16 parallax, int32 color)
{
	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	int32 displacement = ((x - parallax) << 6) + (y >> 2);

	if((unsigned)__FRAME_BUFFERS_SIZE <= (unsigned)displacement)
	{
		return;
	}

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	buffer[displacement] |= color << ((y & 3) << 1);
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

	if((unsigned)__FRAME_BUFFERS_SIZE <= (unsigned)displacement)
	{
		leftBuffer = 0;
	}
	else
	{
		leftBuffer[displacement] &= pixel;
	}

	displacement = (((x + parallax) << 6) + yHelper);

	if((unsigned)__FRAME_BUFFERS_SIZE <= (unsigned)displacement)
	{
		rightBuffer = 0;
	}
	else
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

static inline void DirectDraw::shrinkLineToScreenSpace(fix8_8_ext* x0, fix8_8_ext* y0, fix8_8_ext* parallax0, fix8_8_ext dx, fix8_8_ext dy, fix8_8_ext dParallax, fix8_8_ext x1, fix8_8_ext y1, fix8_8_ext parallax1)
{
	fix8_8_ext x = *x0;
	fix8_8_ext y = *y0;
	fix8_8_ext parallax = *parallax0;

	if(0 == dx)
	{
		if(_frustumFixedPoint.y0 > y)
		{
			*y0 = _frustumFixedPoint.y0;
		}
		else if(_frustumFixedPoint.y1 < y)
		{
			*y0 = _frustumFixedPoint.y1;
		}

		return;
	}

	if(0 == dy)
	{
		parallax = __ABS(parallax);

		if(_frustumFixedPoint.x0 > x + parallax)
		{
			*x0 = _frustumFixedPoint.x0 - parallax;
		}
		else if(_frustumFixedPoint.x1 < x - parallax)
		{
			*x0 = _frustumFixedPoint.x1 + parallax;
		}

		return;
	}

	NM_ASSERT(0 != dx, "DirectDraw::shrinkLineToScreenSpace: dx = 0");

	fix8_8_ext xySlope = __FIX8_8_EXT_DIV(dy, dx);
	fix8_8_ext yParallaxSlope = __FIX8_8_EXT_DIV(dParallax, dy);

	if(0 == xySlope)
	{
		return;
	}

	//(x0 - x1) / dx = (y0 - y1) / dy = (parallax0 - parallax1) / dParallax
	fix8_8_ext parallaxHelper0 = parallax;
	fix8_8_ext parallaxHelper1 = parallax1;

	fix8_8_ext xHelper = x;

	bool shrinkOnXAxis = false;

	if(_frustumFixedPoint.x0 > x)
	{
		// Do not shrking the line if one of the x coordinates (left of right eye) is inside the screen space
		if(_frustumFixedPoint.x0 > x - __ABS(parallax) && _frustumFixedPoint.x0 > x + __ABS(parallax))
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
	else if(_frustumFixedPoint.x1 < x)
	{
		// Do not shrking the line if one of the x coordinates (left of right eye) is inside the screen space
		if(_frustumFixedPoint.x1 < x - __ABS(parallax) && _frustumFixedPoint.x1 < x + __ABS(parallax))
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

		fix8_8_ext dxHelper = (x1 + parallaxHelper1) - xHelper;

		if(0 == dxHelper)
		{
			y = __FIX8_8_EXT_MULT(x - x1, xySlope) + y1;
			parallax = __FIX8_8_EXT_MULT(yParallaxSlope, y - y1) + parallax1;

			*y0 = y;
			*parallax0 = parallax;

			return;
		}

		fix8_8_ext xySlopeHelper = __FIX8_8_EXT_DIV(dy, dxHelper);
		
		if(0 == xySlopeHelper)
		{
			return;
		}
		
		if(0 > xySlope * xySlopeHelper)
		{
			xySlopeHelper = -xySlopeHelper;
		}

		y = __FIX8_8_EXT_MULT(xySlopeHelper, x - (x1 + parallaxHelper1)) + y1;
		parallax = __FIX8_8_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
		x = __FIX8_8_EXT_DIV(y - y1, xySlope) + x1;
	}

	if(_frustumFixedPoint.y0 > y)
	{
		// x = (y - y1)/(dx/dy) + x1
		y = _frustumFixedPoint.y0;
		x = __FIX8_8_EXT_DIV(y - y1, xySlope) + x1;
		parallax = __FIX8_8_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
	}
	else if(_frustumFixedPoint.y1 < y)
	{
		// x = (y - y1)/(dx/dy) + x1
		y = _frustumFixedPoint.y1;
		x = __FIX8_8_EXT_DIV(y - y1, xySlope) + x1;
		parallax = __FIX8_8_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
	}

	*x0 = x;
	*y0 = y;
	*parallax0 = parallax;
}

static void DirectDraw::drawColorLine(PixelVector fromPoint, PixelVector toPoint, int32 color, uint8 bufferIndex, bool interlaced)
{
	bool xFromOutside = (unsigned)_frustumWidth < (unsigned)(fromPoint.x - _frustumFixedPoint.x0);
	bool yFromOutside = (unsigned)_frustumHeight < (unsigned)(fromPoint.y - _frustumFixedPoint.y0);
	bool zFromOutside = (unsigned)_frustumDepth < (unsigned)(fromPoint.z - _frustumFixedPoint.z0);

	bool xToOutside = (unsigned)_frustumWidth < (unsigned)(toPoint.x - _frustumFixedPoint.x0);
	bool yToOutside = (unsigned)_frustumHeight < (unsigned)(toPoint.y - _frustumFixedPoint.y0);
	bool zToOutside = (unsigned)_frustumDepth < (unsigned)(toPoint.z - _frustumFixedPoint.z0);

	if((xFromOutside && xToOutside) || (yFromOutside && yToOutside) || (zFromOutside || zToOutside))
	{
		return;
	}

	uint32 leftBuffer = *_currentDrawingFrameBufferSet | (interlaced ? (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX) : __LEFT_FRAME_BUFFER_0);
	uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

	fix8_8_ext fromPointX = __I_TO_FIX8_8_EXT(fromPoint.x);
	fix8_8_ext fromPointY = __I_TO_FIX8_8_EXT(fromPoint.y);
	fix8_8_ext fromPointParallax = __I_TO_FIX8_8_EXT(fromPoint.parallax);

	fix8_8_ext toPointX = __I_TO_FIX8_8_EXT(toPoint.x);
	fix8_8_ext toPointY = __I_TO_FIX8_8_EXT(toPoint.y);
	fix8_8_ext toPointParallax = __I_TO_FIX8_8_EXT(toPoint.parallax);

	fix8_8_ext dx = toPointX - fromPointX;
	fix8_8_ext dy = toPointY - fromPointY;
	fix8_8_ext dParallax = (toPointParallax - fromPointParallax);

	if(0 == dx && 0 == dy)
	{
		return;
	}

	if(xFromOutside || yFromOutside)
	{
		DirectDraw::shrinkLineToScreenSpace(&fromPointX, &fromPointY, &fromPointParallax, dx, dy, dParallax, toPointX, toPointY, toPointParallax);
	}

	if(xToOutside || yToOutside)
	{
		DirectDraw::shrinkLineToScreenSpace(&toPointX, &toPointY, &toPointParallax, dx, dy, dParallax, fromPointX, fromPointY, fromPointParallax);
	}

	fix8_8_ext dxABS = __ABS(dx);
	fix8_8_ext dyABS = __ABS(dy);

	fix8_8_ext parallaxStart = fromPointParallax;

	fix8_8_ext xStep = __1I_FIX8_8;
	fix8_8_ext yStep = __1I_FIX8_8;
	fix8_8_ext parallaxStep = 0;
	
	int16 totalPixels = 0;

	if(dyABS == dxABS || dyABS < dxABS || 0 == dy)
	{
		if(toPointX < fromPointX)
		{
			fix8_8_ext auxPoint = fromPointX;
			fromPointX = toPointX;
			toPointX = auxPoint;

			fromPointY = toPointY;
			parallaxStart = toPointParallax; 

			dxABS = -dxABS;
		}

		yStep = __FIX8_8_EXT_DIV(dy, dxABS);
		parallaxStep = __FIX8_8_EXT_DIV(dParallax, dxABS);

		totalPixels = __FIX8_8_EXT_TO_I(toPointX - fromPointX);
	}
	else if(dxABS < dyABS || 0 == dx)
	{
		if(toPointY < fromPointY)
		{
			fix8_8_ext auxPoint = fromPointY;
			fromPointY = toPointY;
			toPointY = auxPoint;

			fromPointX = toPointX;
			parallaxStart = toPointParallax; 

			dyABS = -dyABS;
		}

		xStep = __FIX8_8_EXT_DIV(dx, dyABS);
		parallaxStep = __FIX8_8_EXT_DIV(dParallax, dyABS);

		totalPixels = __FIX8_8_EXT_TO_I(toPointY - fromPointY);
	}

	if(_directDraw->totalDrawPixels + totalPixels > _directDraw->maximuDrawPixels)
	{
		return;
	}

	_directDraw->totalDrawPixels += totalPixels;

	if(0 != bufferIndex)
	{
		parallaxStart = -parallaxStart;
		parallaxStep = -parallaxStep;
	}

	if(interlaced)
	{
		for(; 1 < totalPixels; totalPixels -=2)
		{
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), __FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);

			fromPointX += xStep;
			fromPointY += yStep;
			parallaxStart += parallaxStep;

			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), -__FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);

			fromPointX += xStep;
			fromPointY += yStep;
			parallaxStart += parallaxStep;
		}
	}
	else
	{
		for(; 0 < totalPixels; totalPixels -=1)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, __FIX8_8_EXT_TO_I(fromPointX + __05F_FIX8_8), __FIX8_8_EXT_TO_I(fromPointY + __05F_FIX8_8), __FIX8_8_EXT_TO_I(parallaxStart + __05F_FIX8_8), color);

			fromPointX += xStep;
			fromPointY += yStep;
			parallaxStart += parallaxStep;
		}
	}
}

