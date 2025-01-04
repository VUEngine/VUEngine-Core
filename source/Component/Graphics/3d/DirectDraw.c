/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <VIPManager.h>

#include "DirectDraw.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define	__DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS				10000
#define	__DIRECT_DRAW_MINIMUM_NUMBER_OF_PIXELS				1000
#define __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_OVERHEAD		100
#define __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY		100
#define __FRAME_BUFFER_SIDE_BIT_INDEX						16
#define __FRAME_BUFFER_SIDE_BIT								__RIGHT_FRAME_BUFFER_0
#define __FLIP_FRAME_BUFFER_SIDE_BIT(a)						a ^= __FRAME_BUFFER_SIDE_BIT

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum DirectDrawLineShrinkingResult
{
	kDirectDrawLineShrinkingInvalid = 0,
	kDirectDrawLineShrinkingNone,
	kDirectDrawLineShrinkingSafe,
	kDirectDrawLineShrinkingUnsafe
};

typedef struct CustomCameraFrustum
{
	fixed_ext_t x0;
	fixed_ext_t y0;
	fixed_ext_t z0;
	fixed_ext_t x1;
	fixed_ext_t y1;
	fixed_ext_t z1;
} CustomCameraFrustum;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CameraFrustum _frustum __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
CustomCameraFrustum _frustumFixedPoint __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
uint16 _frustumWidth __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = __SCREEN_WIDTH;
uint16 _frustumHeight __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = __SCREEN_HEIGHT;
uint16 _frustumDepth __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = __SCREEN_DEPTH;
DirectDraw _directDraw __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;
uint16 _frustumWidthExtended = __SCREEN_WIDTH << __DIRECT_DRAW_FRUSTUM_EXTENSION_POWER;
uint16 _frustumHeightExtended = __SCREEN_HEIGHT << __DIRECT_DRAW_FRUSTUM_EXTENSION_POWER;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline bool DirectDraw::shrinkLineToScreenSpace(fixed_ext_t* x0, fixed_ext_t* y0, fixed_ext_t* parallax0, fixed_ext_t dx, fixed_ext_t dy, fixed_ext_t dParallax, fixed_ext_t x1, fixed_ext_t y1, fixed_ext_t parallax1)
{
	fixed_ext_t x = *x0;
	fixed_ext_t y = *y0;
	fixed_ext_t parallax = *parallax0;

	if(0 == dx)
	{
		if
		(
			((unsigned)(_frustumFixedPoint.x1 - _frustumFixedPoint.x0) < (unsigned)(x - parallax)) 
			&& 
			((unsigned)(_frustumFixedPoint.x1 - _frustumFixedPoint.x0) < (unsigned)(x + parallax))
		)
		{
			return false;
		}

		if(_frustumFixedPoint.y0 > y)
		{
			*y0 = _frustumFixedPoint.y0;
		}
		else if(_frustumFixedPoint.y1 < y)
		{
			*y0 = _frustumFixedPoint.y1;
		}

		return true;
	}
	else if(0 == dy)
	{
		if((unsigned)(_frustumFixedPoint.y1 - _frustumFixedPoint.y0) < (unsigned)(y - _frustumFixedPoint.y0))
		{
			return false;
		}

		parallax = __ABS(parallax);

		if(_frustumFixedPoint.x0 > x + parallax)
		{
			*x0 = _frustumFixedPoint.x0 - parallax;
		}
		else if(_frustumFixedPoint.x1 < x - parallax)
		{
			*x0 = _frustumFixedPoint.x1 + parallax;
		}

		return true;
	}

	NM_ASSERT(0 != dx, "DirectDraw::shrinkLineToScreenSpace: dx = 0");

	fixed_ext_t xySlope = __FIXED_EXT_DIV(dy, dx);

	if(0 == xySlope)
	{
		return false;
	}

	fixed_ext_t yParallaxSlope = __FIXED_EXT_DIV(dParallax, dy);

	//(x0 - x1) / dx = (y0 - y1) / dy = (parallax0 - parallax1) / dParallax
	fixed_ext_t parallaxHelper0 = parallax;
	fixed_ext_t parallaxHelper1 = parallax1;

	fixed_ext_t xHelper = x;

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

		fixed_ext_t dxHelper = (x1 + parallaxHelper1) - xHelper;

		if(0 == dxHelper)
		{
			y = __FIXED_EXT_MULT(x - x1, xySlope) + y1;
			parallax = __FIXED_EXT_MULT(yParallaxSlope, y - y1) + parallax1;

			*y0 = y;
			*parallax0 = parallax;

			return true;
		}

		fixed_ext_t xySlopeHelper = __FIXED_EXT_DIV(dy, dxHelper);
		
		if(0 == xySlopeHelper)
		{
			return false;
		}
		
		if(0 > xySlope * xySlopeHelper)
		{
			xySlopeHelper = -xySlopeHelper;
		}

		y = __FIXED_EXT_MULT(xySlopeHelper, x - (x1 + parallaxHelper1)) + y1;
		parallax = __FIXED_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
		x = __FIXED_EXT_DIV(y - y1, xySlope) + x1;
	}

	// (x0 - x1) / dx = (y0 - y1) / dy = (parallax0 - parallax1) / dParallax
	// (x0 - x1) / dx = (y0 - y1) / dy
	// x0 = (y0 - y1) * (dx / dy) * xySlope / xySlope + x1
	// x0 = (y0 - y1) * (dx / dy) * (dy / dx) / (dy / dx) + x1
	// x0 = (y0 - y1) / (dy / dx) + x1
	// x0 = (y0 - y1) / xySlope + x1
	if(_frustumFixedPoint.y0 > y)
	{
		y = _frustumFixedPoint.y0;
		x = __FIXED_EXT_DIV(y - y1, xySlope) + x1;
		parallax = __FIXED_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
	}
	else if(_frustumFixedPoint.y1 < y)
	{
		y = _frustumFixedPoint.y1;
		x = __FIXED_EXT_DIV(y - y1, xySlope) + x1;
		parallax = __FIXED_EXT_MULT(yParallaxSlope, y - y1) + parallax1;
	}

	// Check for overflows
	if(*x0 < x1 && !(*x0 < x && x < x1))
	{
		return false;
	}
	else if(*x0 > x1 && !(x1 < x && x < *x0))
	{
		return false;
	}

	if(*y0 < y1 && !(*y0 < y && y < y1))
	{
		return false;
	}
	else if(*y0 > y1 && !(y1 < y && y < *y0))
	{
		return false;
	}
/*
	if(*parallax0 < parallax1 && !(*parallax0 < parallax && parallax < parallax1))
	{
//		return false;
	}
	else if(*parallax0 > parallax1 && !(parallax1 < parallax && parallax < *parallax0))
	{
//		return false;
	}
*/	
	*x0 = x;
	*y0 = y;
	*parallax0 = parallax;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawPoint(PixelVector point, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(interlaced)
	{
		uint32 buffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);

		DirectDraw::drawColorPixelInterlaced((BYTE*)buffer, point.x, point.y, 0!= bufferIndex ? -point.parallax : point.parallax, color);
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		if(!DirectDraw::isPointInsideFrustum(point))
		{
			return false;
		}

		DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, point.x, point.y, point.parallax, color);
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawLine(PixelVector fromPoint, PixelVector toPoint, int32 color, uint8 bufferIndex, bool interlaced)
{
	uint16 xFromDeltaLeft = (unsigned)(fromPoint.x - fromPoint.parallax - _frustum.x0);
	uint16 xFromDeltaRight = (unsigned)(fromPoint.x +  fromPoint.parallax - _frustum.x0);
	uint16 yFromDelta = (unsigned)(fromPoint.y - _frustum.y0);

	uint16 xToDeltaLeft = (unsigned)(toPoint.x - toPoint.parallax - _frustum.x0);
	uint16 xToDeltaRight = (unsigned)(toPoint.x + toPoint.parallax - _frustum.x0);
	uint16 yToDelta = (unsigned)(toPoint.y - _frustum.y0);

	bool xFromOutside = _frustumWidthExtended < xFromDeltaLeft && _frustumWidthExtended < xFromDeltaRight;
	bool yFromOutside = _frustumHeightExtended < yFromDelta;
	bool zFromOutside = _frustumDepth < (unsigned)(fromPoint.z - _frustum.z0);

	bool xToOutside = _frustumWidthExtended < xToDeltaRight && _frustumWidthExtended < xToDeltaRight;
	bool yToOutside = _frustumHeightExtended < yToDelta;
	bool zToOutside = _frustumDepth < (unsigned)(toPoint.z - _frustum.z0);

	bool xOutside = (xFromOutside && xToOutside) && (0 <= ((unsigned)fromPoint.x ^ (unsigned)toPoint.x));
	bool yOutside = (yFromOutside && yToOutside) && (0 <= ((unsigned)fromPoint.y ^ (unsigned)toPoint.y));
	bool zOutside = (zFromOutside + zToOutside);

	if(xOutside || yOutside || zOutside)
	{
		return false;
	}

	fixed_ext_t fromPointX = __I_TO_FIXED_EXT(fromPoint.x);
	fixed_ext_t fromPointY = __I_TO_FIXED_EXT(fromPoint.y);
	fixed_ext_t fromPointParallax = __I_TO_FIXED_EXT(fromPoint.parallax);

	fixed_ext_t toPointX = __I_TO_FIXED_EXT(toPoint.x);
	fixed_ext_t toPointY = __I_TO_FIXED_EXT(toPoint.y);
	fixed_ext_t toPointParallax = __I_TO_FIXED_EXT(toPoint.parallax);

	fixed_ext_t dx = toPointX - fromPointX;
	fixed_ext_t dy = toPointY - fromPointY;
	fixed_ext_t dParallax = toPointParallax - fromPointParallax;

	int16 totalPixelRounding = 1;
	
	if(0 == dx && 0 == dy)
	{
		return false;
	}

	if
	(
		_frustumWidth + __DIRECT_DRAW_LINE_SHRINKING_PADDING < xFromDeltaLeft 
		|| 
		_frustumWidth + __DIRECT_DRAW_LINE_SHRINKING_PADDING < xFromDeltaRight 
		|| 
		_frustumHeight + __DIRECT_DRAW_LINE_SHRINKING_PADDING < yFromDelta
	)
	{
		totalPixelRounding = 0;

		if
		(
			!DirectDraw::shrinkLineToScreenSpace
			(
				&fromPointX, &fromPointY, &fromPointParallax, dx, dy, dParallax, toPointX, toPointY, toPointParallax
			)
		)
		{
			return false;
		}

		xFromOutside = (unsigned)_frustumFixedPoint.x1 - _frustumFixedPoint.x0 < (unsigned)(fromPointX - _frustumFixedPoint.x0);
		yFromOutside = (unsigned)_frustumFixedPoint.y1 - _frustumFixedPoint.y0 < (unsigned)(fromPointY - _frustumFixedPoint.y0);
	}

	if
	(
		_frustumWidth + __DIRECT_DRAW_LINE_SHRINKING_PADDING < xToDeltaLeft 
		|| 
		_frustumWidth + __DIRECT_DRAW_LINE_SHRINKING_PADDING < xToDeltaRight 
		|| 
		_frustumHeight + __DIRECT_DRAW_LINE_SHRINKING_PADDING < yToDelta
	)
	{
		totalPixelRounding = 0;

		if
		(
			!DirectDraw::shrinkLineToScreenSpace
			(
				&toPointX, &toPointY, &toPointParallax, dx, dy, dParallax, fromPointX, fromPointY, fromPointParallax
			)
		)
		{
			return false;
		}

		xToOutside = (unsigned)_frustumFixedPoint.x1 - _frustumFixedPoint.x0 < (unsigned)(toPointX - _frustumFixedPoint.x0);
		yToOutside = (unsigned)_frustumFixedPoint.y1 - _frustumFixedPoint.y0 < (unsigned)(toPointY - _frustumFixedPoint.y0);
	}

	if((xFromOutside && xToOutside) || (yFromOutside && yToOutside))
	{
		return false;
	}

/*
	PixelVector::print(fromPoint, 1, 16);
	PixelVector::print(toPoint, 11, 16);

	PRINT_TEXT("x:    ", 1, 10);
	PRINT_TEXT("y:    ", 1, 11);
	PRINT_TEXT("p:    ", 1, 12);
	PRINT_TEXT("x:    ", 11, 10);
	PRINT_TEXT("y:    ", 11, 11);
	PRINT_TEXT("p:    ", 11, 12);
	PRINT_INT(__FIXED_EXT_TO_I(fromPointX), 1, 10);
	PRINT_INT(__FIXED_EXT_TO_I(fromPointY), 1, 11);
	PRINT_INT(__FIXED_EXT_TO_I(fromPointParallax), 1, 12);
	PRINT_INT(__FIXED_EXT_TO_I(toPointX), 11, 10);
	PRINT_INT(__FIXED_EXT_TO_I(toPointY), 11, 11);
	PRINT_INT(__FIXED_EXT_TO_I(toPointParallax), 11, 12);
*/
	fixed_ext_t dxABS = __ABS(dx);
	fixed_ext_t dyABS = __ABS(dy);

	fixed_ext_t parallaxStart = fromPointParallax;

	fixed_ext_t xStep = toPointX < fromPointX ? -__1I_FIXED : __1I_FIXED;
	fixed_ext_t yStep = toPointY < fromPointY ? -__1I_FIXED : __1I_FIXED;
	fixed_ext_t parallaxStep = 0;
	
	int16 totalPixels = 0;

	if(0 <= dxABS - dyABS)
	{
		yStep = __FIXED_EXT_DIV(dy, dxABS);
		parallaxStep = __FIXED_EXT_DIV(dParallax, dxABS);
		totalPixels = __ABS(__FIXED_EXT_TO_I(toPointX - fromPointX)) + totalPixelRounding;
	}
	else
	{
		xStep = __FIXED_EXT_DIV(dx, dyABS);
		parallaxStep = __FIXED_EXT_DIV(dParallax, dyABS);
		totalPixels = __ABS(__FIXED_EXT_TO_I(toPointY - fromPointY)) + totalPixelRounding;
	}

	_directDraw->drawnPixelsCounter += totalPixels;

	if(_directDraw->drawnPixelsCounter > _directDraw->maximumPixelsToDraw)
	{
		return false;
	}

	if(interlaced)
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
		uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

		parallaxStep <<= 1;

		if(0 != bufferIndex)
		{
			parallaxStart = -parallaxStart;
			parallaxStep = -parallaxStep;

			for(; 1 < totalPixels; totalPixels -=2)
			{
				DirectDraw::drawColorPixelInterlaced
				(
					(BYTE*)leftBuffer, __FIXED_EXT_TO_I(fromPointX), __FIXED_EXT_TO_I(fromPointY), __FIXED_EXT_TO_I(parallaxStart), color
				);

				fromPointX += xStep;
				fromPointY += yStep;

				DirectDraw::drawColorPixelInterlaced
				(
					(BYTE*)rightBuffer, __FIXED_EXT_TO_I(fromPointX), __FIXED_EXT_TO_I(fromPointY), -__FIXED_EXT_TO_I(parallaxStart), color
				);

				fromPointX += xStep;
				fromPointY += yStep;
				parallaxStart += parallaxStep;
			}
		}
		else
		{
			for(; 1 < totalPixels; totalPixels -=2)
			{
				DirectDraw::drawColorPixelInterlaced
				(
					(BYTE*)leftBuffer, __FIXED_EXT_TO_I(fromPointX), __FIXED_EXT_TO_I(fromPointY), __FIXED_EXT_TO_I(parallaxStart), color
				);

				fromPointX += xStep;
				fromPointY += yStep;

				DirectDraw::drawColorPixelInterlaced
				(
					(BYTE*)rightBuffer, __FIXED_EXT_TO_I(fromPointX), __FIXED_EXT_TO_I(fromPointY), -__FIXED_EXT_TO_I(parallaxStart), color
				);

				fromPointX += xStep;
				fromPointY += yStep;
				parallaxStart += parallaxStep;
			}
		}
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		for(; 0 < totalPixels; totalPixels -=1)
		{
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, __FIXED_EXT_TO_I(fromPointX), __FIXED_EXT_TO_I(fromPointY), 
				__FIXED_EXT_TO_I(parallaxStart), color
			);

			fromPointX += xStep;
			fromPointY += yStep;
			parallaxStart += parallaxStep;
		}

#ifdef __DIRECT_DRAW_OPTIMIZED_VERTICAL_LINES
		DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, -1, -1, 0, 0);
#endif
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(0 >= radius)
	{
		return false;
	}

	bool xFromOutside = _frustumWidth < (unsigned)(center.x - _frustum.x0);
	bool yFromOutside = _frustumHeight < (unsigned)(center.y - _frustum.y0);
	bool zFromOutside = _frustumDepth < (unsigned)(center.z - _frustum.z0);

	bool xToOutside = _frustumWidth < (unsigned)(center.x - _frustum.x0);
	bool yToOutside = _frustumHeight < (unsigned)(center.y - _frustum.y0);
	bool zToOutside = _frustumDepth < (unsigned)(center.z - _frustum.z0);

	bool xOutside = (xFromOutside && xToOutside);
	bool yOutside = (yFromOutside && yToOutside); 
	bool zOutside = (zFromOutside || zToOutside);

	if(xOutside || yOutside || zOutside)
	{
		return false;
	}

	// Bresenham circle algorithm
	if(interlaced && 3 < radius)
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
		uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

		int16 x = radius;
		int16 y = 0;		
	    int16 radiusError = 1 - x;

		center.parallax /= 2;

		center.x -= radius + center.parallax;
		center.y -= radius;

		if(0 != bufferIndex)
		{
			center.parallax = -center.parallax;
		}

	    while (x >= y)
		{
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x + x + radius, center.y + y + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x + y + radius, center.y + x + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x - x + radius, center.y + y + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x - y + radius, center.y + x + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x - x + radius, center.y - y + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x - y + radius, center.y - x + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x + x + radius, center.y - y + radius, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x + y + radius, center.y - x + radius, center.parallax, color);

			y++;

			if (radiusError < 0) 
			{
				radiusError += 2 * y + 1;
			} 
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}

			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x + x + radius, center.y + y + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x + y + radius, center.y + x + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x - x + radius, center.y + y + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x - y + radius, center.y + x + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x - x + radius, center.y - y + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x - y + radius, center.y - x + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x + x + radius, center.y - y + radius, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x + y + radius, center.y - x + radius, -center.parallax, color);

			y++;

			if (radiusError < 0) 
			{
				radiusError += 2 * y + 1;
			} 
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}
		}		
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		int16 x = radius;
		int16 y = 0;		
	    int16 radiusError = 1 - x;

		center.x -= radius;
		center.y -= radius;

	    while (x >= y)
		{
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x + x + radius, center.y + y + radius, center.parallax, color
			);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x + y + radius, center.y + x + radius, center.parallax, color
			);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x - x + radius, center.y + y + radius, center.parallax, color
			);

			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x - y + radius, center.y + x + radius, center.parallax, color
			);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x - x + radius, center.y - y + radius, center.parallax, color
			);

			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x - y + radius, center.y - x + radius, center.parallax, color
			);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x + x + radius, center.y - y + radius, center.parallax, color
			);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x + y + radius, center.y - x + radius, center.parallax, color
			);

			y++;

			if (radiusError < 0) 
			{
				radiusError += 2 * y + 1;
			} 
			else
			{
				x--;
				radiusError += 2 * (y - x + 1);
			}
		}
	}
	
	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawX(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(!DirectDraw::isPointInsideFrustum(center))
	{
		return false;
	}

	int16 lengthHelper = 0;
	int16 halfLength = length >> 1;
	int16 x = center.x - halfLength;
	int16 y = center.y - halfLength;

	if(interlaced)
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
		uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

		if(0 != bufferIndex)
		{
			center.parallax = -center.parallax;
		}

		for(; lengthHelper <= length; lengthHelper++, x++, y++)
		{
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, x, y, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, (center.x + halfLength) - lengthHelper, y, center.parallax, color);

			x++;
			y++;
			lengthHelper++;

			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, x, y, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, (center.x + halfLength) - lengthHelper, y, -center.parallax, color);
		}
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		for(; lengthHelper <= length; lengthHelper++, x++, y++)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, x, y, center.parallax, color);
			
			DirectDraw::drawColorPixel
			(
				(BYTE*)leftBuffer, (BYTE*)rightBuffer, (center.x + halfLength) - lengthHelper, y, center.parallax, color
			);
		}
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawCross(PixelVector center, int16 length, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(!DirectDraw::isPointInsideFrustum(center))
	{
		return false;
	}

	int16 lengthHelper = 0;
	int16 halfLength = length >> 1;

	if(interlaced)
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
		uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

		if(0 != bufferIndex)
		{
			center.parallax = -center.parallax;
		}

		for(int16 coordinate = -halfLength; lengthHelper < length; coordinate++, lengthHelper++)
		{
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x + coordinate, center.y, center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, center.x, center.y + coordinate, center.parallax, color);

			coordinate++;
			lengthHelper++;

			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x + coordinate, center.y, -center.parallax, color);
			DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, center.x, center.y + coordinate, -center.parallax, color);
		}
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		for(int16 coordinate = -halfLength; lengthHelper < length; coordinate++, lengthHelper++)
		{
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x + coordinate, center.y, center.parallax, color);
			DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, center.x, center.y + coordinate, center.parallax, color);
		}
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawSolidCircle(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(0 >= radius)
	{
		return false;
	}
	
	bool xFromOutside = _frustumWidth < (unsigned)(center.x - _frustum.x0);
	bool yFromOutside = _frustumHeight < (unsigned)(center.y - _frustum.y0);
	bool zFromOutside = _frustumDepth < (unsigned)(center.z - _frustum.z0);

	bool xToOutside = _frustumWidth < (unsigned)(center.x - _frustum.x0);
	bool yToOutside = _frustumHeight < (unsigned)(center.y - _frustum.y0);
	bool zToOutside = _frustumDepth < (unsigned)(center.z - _frustum.z0);

	bool xOutside = (xFromOutside && xToOutside);
	bool yOutside = (yFromOutside && yToOutside); 
	bool zOutside = (zFromOutside || zToOutside);

	if(xOutside || yOutside || zOutside)
	{
		return false;
	}

	uint32 radiusSquare = radius * radius;

	for(int16 x = -radius; x <= radius; x++)
	{
		int16 y = Math::squareRoot(radiusSquare - x * x);

		DirectDraw::drawLine
		(
			(PixelVector){center.x + x, center.y - y, center.z, center.parallax}, 
			(PixelVector){center.x + x, center.y + y, center.z, center.parallax}, color, bufferIndex, interlaced
		);
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::drawSolidRhumbus(PixelVector center, int16 radius, int32 color, uint8 bufferIndex, bool interlaced)
{
	if(!DirectDraw::isPointInsideFrustum(center))
	{
		return false;
	}

	int16 radiusHelper = 0;
	int16 y = center.y - radius;

	if(interlaced)
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | (bufferIndex << __FRAME_BUFFER_SIDE_BIT_INDEX);
		uint32 rightBuffer = leftBuffer ^ __FRAME_BUFFER_SIDE_BIT;

		center.parallax >>= 1;

		if(0 != bufferIndex)
		{
			center.parallax = -center.parallax;
		}

		for(; radiusHelper <= radius; radiusHelper++, y++)
		{
			for(int16 x = center.x - radiusHelper; x <= center.x + radiusHelper; x++)
			{
				DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, x, y, center.parallax, color);

				x++;

				DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, x, y, -center.parallax, color);
			}
		}

		for(; 0 <= radiusHelper; --radiusHelper, y++)
		{
			for(int16 x = center.x - radiusHelper; x <= center.x + radiusHelper; x++)
			{
				DirectDraw::drawColorPixelInterlaced((BYTE*)leftBuffer, x, y, center.parallax, color);

				x++;

				DirectDraw::drawColorPixelInterlaced((BYTE*)rightBuffer, x, y, -center.parallax, color);
			}
		}
	}
	else
	{
		uint32 leftBuffer = *_currentDrawingFrameBufferSet | __LEFT_FRAME_BUFFER_0;
		uint32 rightBuffer = *_currentDrawingFrameBufferSet | __RIGHT_FRAME_BUFFER_0;

		for(; radiusHelper <= radius; radiusHelper++, y++)
		{
			for(int16 x = center.x - radiusHelper; x <= center.x + radiusHelper; x++)
			{
				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, x, y, center.parallax, color);
			}
		}

		for(; 0 <= radiusHelper; --radiusHelper, y++)
		{
			for(int16 x = center.x - radiusHelper; x <= center.x + radiusHelper; x++)
			{
				DirectDraw::drawColorPixel((BYTE*)leftBuffer, (BYTE*)rightBuffer, x, y, center.parallax, color);
			}
		}
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Draws a pixel on the screen.
/// This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
/// instead.
/// @param buffer	Buffer base address
/// @param x			Camera x coordinate
/// @param y			Camera y coordinate
/// @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Draws a non black pixel on the screen.
/// This will yield no result for color = 0, so for drawing a black pixel, use DirectDraw_drawBlackPixel
/// instead.
/// @param buffer	Buffer base address
/// @param x			Camera x coordinate
/// @param y			Camera y coordinate
/// @param color		The color to draw (__COLOR_BRIGHT_RED, __COLOR_MEDIUM_RED or __COLOR_DARK_RED)
static void DirectDraw::drawColorPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax, int32 color)
{
#ifdef __DIRECT_DRAW_OPTIMIZED_VERTICAL_LINES
	uint16 pixel = color << ((y & 0x7) << 1);

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	uint16 displacement = ((x - parallax) << 5) + (y >> 3);	

	static uint16 cachedDisplacement = __FRAME_BUFFERS_SIZE;

	static uint16 cachedLeftData = 0;
	static uint16 cachedRightData = 0;

	if(displacement != cachedDisplacement)
	{
		if((__FRAME_BUFFERS_SIZE >> 1) > cachedDisplacement)
		{
			((uint16*)leftBuffer)[cachedDisplacement] = cachedLeftData;
		}

		if((__FRAME_BUFFERS_SIZE >> 1) > cachedDisplacement + (parallax << 5))
		{
			((uint16*)rightBuffer)[cachedDisplacement + (parallax << 5)] = cachedRightData;
		}

		cachedDisplacement = displacement;

		cachedLeftData = ((uint16*)leftBuffer)[cachedDisplacement];
		cachedRightData = ((uint16*)rightBuffer)[cachedDisplacement + (parallax << 5)];
	}

	cachedLeftData |= pixel;
	cachedRightData |= pixel;
#else
	uint8 pixel = color << ((y & 0x3) << 1);

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	uint16 displacement = ((x - parallax) << 6) + (y >> 2);	

	if(__FRAME_BUFFERS_SIZE > displacement)
	{
		leftBuffer[displacement] |= pixel;
	}

	displacement += parallax << 7;

	if(__FRAME_BUFFERS_SIZE > displacement)
	{
		rightBuffer[displacement] |= pixel;
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void DirectDraw::drawColorPixelInterlaced(BYTE* buffer, int16 x, int16 y, int16 parallax, int32 color)
{
	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	uint16 displacement = ((x - parallax) << 6) + (y >> 2);

	if(__FRAME_BUFFERS_SIZE <= displacement)
	{
		return;
	}

	// calculate pixel position
	// each column has 16 words, so 16 * 4 bytes = 64, each byte represents 4 pixels
	buffer[displacement] |= color << ((y & 3) << 1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Draws a black pixel on the screen.
/// We have a separate function for black pixes since nulling bits requires a slightly different way than
/// simply writing 1's. Adding an if clause instead to the putPixel method instead would be too heavy on
/// the processor when used inside a loop due to the branching.
/// @param buffer	Buffer base address
/// @param x			Camera x coordinate
/// @param y			Camera y coordinate
static void DirectDraw::drawBlackPixel(BYTE* leftBuffer, BYTE* rightBuffer, int16 x, int16 y, int16 parallax)
{
	uint16 yHelper = y >> 2;
	uint8 pixel = ~(3 << ((y & 3) << 1));

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

	_directDraw->drawnPixelsCounter++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool DirectDraw::isPointInsideFrustum(PixelVector point)
{
	bool xOutside = _frustumWidth < (unsigned)(point.x - _frustum.x0);
	bool yOutside = _frustumHeight < (unsigned)(point.y - _frustum.y0);
	bool zOutside = _frustumDepth < (unsigned)(point.z);

	if(xOutside || yOutside || zOutside)
	{
		return false;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DirectDraw::reset()
{
	this->maximumPixelsToDraw = __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS;

	DirectDraw::setFrustum(this, (CameraFrustum)
	{
		0, 0, 0, __SCREEN_WIDTH - 1, __SCREEN_HEIGHT - 1, 8191
	});
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DirectDraw::preparteToDraw()
{
#ifdef __SHOW_DIRECT_DRAWING_PROFILING
	static int counter = 0;

	if(__TARGET_FPS <= counter++)
	{
		DirectDraw::print(this, 1, 1);
		counter = 0;
	}
#endif

	if(this->drawnPixelsCounter <= this->maximumPixelsToDraw)
	{
		this->maximumPixelsToDraw += __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_RECOVERY;

		if(__DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS < this->maximumPixelsToDraw)
		{
			this->maximumPixelsToDraw = __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS;
		}
	}

	this->drawnPixelsCounter = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DirectDraw::setFrustum(CameraFrustum frustum)
{
	if(frustum.x1 >= __SCREEN_WIDTH)
	{
		frustum.x1 = __SCREEN_WIDTH - 1;
	}

	if(frustum.y1 >= __SCREEN_HEIGHT)
	{
		frustum.y1 = __SCREEN_HEIGHT - 1;
	}

	// 9: 2's power equal to the math type fixed_t
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
	_frustumWidth = _frustum.x1 > _frustum.x0 ? _frustum.x1 - _frustum.x0 : _frustum.x0 - _frustum.x1;
	_frustumHeight = _frustum.y1 > _frustum.y0 ? _frustum.y1 - _frustum.y0 : _frustum.y0 - _frustum.y1;
	_frustumDepth = _frustum.z1 > _frustum.z0 ? _frustum.z1 - _frustum.z0 : _frustum.z0 - _frustum.z1;

	_frustumWidthExtended = _frustumWidth << __DIRECT_DRAW_FRUSTUM_EXTENSION_POWER;
	_frustumHeightExtended = _frustumHeight << __DIRECT_DRAW_FRUSTUM_EXTENSION_POWER;

	_frustumFixedPoint = (CustomCameraFrustum)
	{
		__I_TO_FIXED_EXT(_frustum.x0), __I_TO_FIXED_EXT(_frustum.y0), __I_TO_FIXED_EXT(_frustum.z0),
		__I_TO_FIXED_EXT(_frustum.x1), __I_TO_FIXED_EXT(_frustum.y1), __I_TO_FIXED_EXT(_frustum.z1),
	};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CameraFrustum DirectDraw::getFrustum()
{
	return _frustum;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void DirectDraw::print(int16 x, int16 y)
{
	Printing::text(Printing::getInstance(), "DIRECT DRAW", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Drawn pixels:      ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->drawnPixelsCounter, x + 14, y++, NULL);
	Printing::text(Printing::getInstance(), "Max. pixels:       ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->maximumPixelsToDraw, x + 14, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DirectDraw::constructor()
{
	_directDraw = this;

	// Always explicitly call the base's constructor 
	Base::constructor();

	this->drawnPixelsCounter = 0;
	this->maximumPixelsToDraw = 0;

	DirectDraw::reset(this);

	VIPManager::addEventListener
	(
		VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)DirectDraw::onVIPManagerGAMESTARTDuringXPEND, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DirectDraw::destructor()
{
	VIPManager::removeEventListener
	(
		VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)DirectDraw::onVIPManagerGAMESTARTDuringXPEND, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);

	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool DirectDraw::onVIPManagerGAMESTARTDuringXPEND(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->maximumPixelsToDraw = this->drawnPixelsCounter - __DIRECT_DRAW_MAXIMUM_NUMBER_OF_PIXELS_OVERHEAD;

	if(__DIRECT_DRAW_MINIMUM_NUMBER_OF_PIXELS > this->maximumPixelsToDraw)
	{
		this->maximumPixelsToDraw = __DIRECT_DRAW_MINIMUM_NUMBER_OF_PIXELS;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

