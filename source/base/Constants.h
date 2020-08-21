/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Error.h>
#include <Globals.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

// memory manipulation
#define __MEMORY_USED_BLOCK_FLAG	0xFFFFFFFF
#define __MEMORY_FREE_BLOCK_FLAG	0x00000000


// Camera half width in pixels
#define __HALF_SCREEN_WIDTH						(__SCREEN_WIDTH >> 1)

// Camera half height in pixels
#define __HALF_SCREEN_HEIGHT					(__SCREEN_HEIGHT >> 1)

// Camera half depth in pixels
#define __HALF_SCREEN_DEPTH						(__SCREEN_DEPTH >> 1)

// Camera width in chars
#define __SCREEN_WIDTH_IN_CHARS					(__SCREEN_WIDTH >> 3)

// Camera height in chars
#define __SCREEN_HEIGHT_IN_CHARS				(__SCREEN_HEIGHT >> 3)

// Camera depth in chars
#define __SCREEN_DEPTH_IN_CHARS					(__SCREEN_DEPTH >> 3)

// Camera half width in chars
#define __HALF_SCREEN_WIDTH_IN_CHARS			(__SCREEN_WIDTH >> 4)

// Camera half height in chars
#define __HALF_SCREEN_HEIGHT_IN_CHARS			(__SCREEN_HEIGHT >> 4)

// Camera half depth in chars
#define __HALF_SCREEN_DEPTH_IN_CHARS			(__SCREEN_DEPTH >> 4)

// Camera half width in meters
#define __HALF_SCREEN_WIDTH_METERS				__PIXELS_TO_METERS(__SCREEN_WIDTH >> 1)

// Camera half height in meters
#define __HALF_SCREEN_HEIGHT_METERS				__PIXELS_TO_METERS(__SCREEN_HEIGHT >> 1)

// Camera half depth in meters
#define __HALF_SCREEN_DEPTH_METERS				__PIXELS_TO_METERS(__SCREEN_DEPTH >> 1)


// used for exceptions
#define __EXCEPTIONS_BGMAP		0
#define __EXCEPTIONS_WORLD		31

// use for faster rounding on fix* values
#define __1I_FIX7_9 			0x0200
#define __1I_FIX10_6			0x0040
#define __05F_FIX10_6			0x0020

// override null spec (because we don't want to include standard C libraries)
#define NULL 		(void *)0x00000000

// axis specs
#define __NO_AXIS 	0x00
#define __X_AXIS 	0x01
#define __Y_AXIS 	0x02
#define __Z_AXIS 	0x04
#define __ALL_AXIS	(__X_AXIS | __Y_AXIS | __Z_AXIS)

// direction
#define __LEFT		((int)-1)
#define __RIGHT		((int)1)
#define __UP		((int)-1)
#define __DOWN		((int)1)
#define __NEAR		((int)-1)
#define __FAR		((int)1)

#define __MILLISECONDS_PER_SECOND			1000
#define __MICROSECONDS_PER_MILLISECOND		1000
#define __MICROSECONDS_PER_SECOND			(__MILLISECONDS_PER_SECOND * __MICROSECONDS_PER_MILLISECOND)

// messages
enum MessagesTypes
{
	kMessageNone = 0,

	// general purpose messages
	kMessageHighFps,
	kMessageEntityRemoved,

	// physics messages
	kMessageBodyStopped,
	kMessageBodyBounced,
	kMessageBodyStartedMoving,
	kMessageBodyChangedDirection,

	// keypad massages
	kMessageKeyPressed,
	kMessageKeyReleased,
	kMessageKeyHold,

	// Communication messages
	kMessageCheckIfRemoteIsReady,
	kMessageLastCommunicationMessage,

	// don't place messages below this
	kMessageLastEngine
};

enum DefaultInGameTypes
{
	kTypeNone = 0,
};

enum DefaulCollisionLayers
{
    kLayerNone = 0,
};

#ifndef __RELEASE
#define __REGISTER_LAST_PROCESS_NAME

void HardwareManager_printStackStatus(int x, int y, bool resumed);

#define __CHECK_STACK_STATUS																				\
	extern bool _stackHeadroomViolation;																	\
	if(!_stackHeadroomViolation)																			\
	{																										\
		int _vuengineStackPointer;																			\
		asm(" mov sp,%0  ": "=r" (_vuengineStackPointer));													\
																											\
		if((0x05000000 & _vuengineStackPointer) &&															\
			_vuengineStackPointer - __STACK_HEADROOM < (int)&_bss_end)										\
		{																									\
			_stackHeadroomViolation = true;																	\
			Printing_setDebugMode(Printing_getInstance());													\
			Printing_clear(Printing_getInstance());															\
			HardwareManager_printStackStatus(1, 15, false);													\
			NM_ASSERT(false, "HardwareManager_checkStack: surpassed headroom boundary!");					\
		}																									\
	}
#else
	#define __CHECK_STACK_STATUS
#endif

#undef NM_ASSERT

#ifndef __RELEASE
#define NM_ASSERT(Statement, ...)																		\
																										\
	if(!(Statement) && !_triggeringException)															\
	{ 																									\
		_triggeringException = true;																	\
		asm(" mov sp,%0  ": "=r" (_vuengineStackPointer));												\
		asm(" mov lp,%0  ": "=r" (_vuengineLinkPointer));												\
																										\
		/* thrown exception */																			\
		Error_triggerException(__MAKE_STRING(__VA_ARGS__), NULL);										\
	}
#else
	#define NM_ASSERT(Statement, ...)
#endif

#undef NM_CAST_ASSERT

#ifndef __RELEASE
#define NM_CAST_ASSERT(Statement, ...)																	\
																										\
	if(!(Statement) && !_triggeringException)															\
	{ 																									\
		_triggeringException = true;																	\
																										\
		/* thrown exception */																			\
		Error_triggerException(__MAKE_STRING(__VA_ARGS__), NULL);										\
	}
#else
	#define NM_CAST_ASSERT(Statement, ...)
#endif

#undef ASSERT

#ifndef __DEBUG
	#define ASSERT(Statement, ...)
#else
#define ASSERT(Statement, Message)																		\
																										\
	if(!(Statement) && !_triggeringException) 															\
	{																									\
		_triggeringException = true;																	\
		asm(" mov sp,%0  ": "=r" (_vuengineStackPointer));												\
		asm(" mov lp,%0  ": "=r" (_vuengineLinkPointer));												\
																										\
		/* thrown exception */																			\
		Error_triggerException(Message, NULL);															\
	}

	#undef __FORCE_PRINTING_LAYER
	#define __FORCE_PRINTING_LAYER
#endif


#define __PIXELS_PER_METER						16
#define __METERS_PER_PIXEL						__F_TO_FIX10_6(1.0f/(float)__PIXELS_PER_METER)

#define __PIXELS_PER_METER_2_POWER				4
#define __PIXELS_TO_METERS(pixels)				(fix10_6)(__I_TO_FIX10_6_EXT(pixels) >> __PIXELS_PER_METER_2_POWER)
#define __REAL_PIXELS_TO_METERS(pixels)			(fix10_6)(__F_TO_FIX10_6_EXT(pixels) >> __PIXELS_PER_METER_2_POWER)
#define __METERS_TO_PIXELS(meters)				__FIX10_6_TO_I(((fix10_6_ext)meters) << __PIXELS_PER_METER_2_POWER)
#define __METERS_TO_PIXELS_ROUNDED(meters)		__FIX10_6_TO_I(__05F_FIX10_6 + (((fix10_6_ext)meters) << __PIXELS_PER_METER_2_POWER))

#define __SCREEN_WIDTH_METERS					__PIXELS_TO_METERS(__SCREEN_WIDTH)
#define __SCREEN_HEIGHT_METERS					__PIXELS_TO_METERS(__SCREEN_HEIGHT)
#define __SCREEN_DEPTH_METERS					__PIXELS_TO_METERS(__SCREEN_DEPTH)

// round meters to multiples of 4 since that is
// the size of 1 pixel
#define __CLAMP_METERS(value)				((((value) + ((1 << __CAMERA_MINIMUM_DISPLACEMENT_PIXELS_POWER) - 1)) >> __CAMERA_MINIMUM_DISPLACEMENT_PIXELS_POWER) << __CAMERA_MINIMUM_DISPLACEMENT_PIXELS_POWER)

#define __CAMERA_MINIMUM_DISPLACEMENT_ROUNDING(value)		((value) & (0xFFFF << __CAMERA_MINIMUM_DISPLACEMENT_PIXELS_POWER))


#define __MINIMUM_X_VIEW_DISTANCE_POWER			4
#define __MINIMUM_Y_VIEW_DISTANCE_POWER			4

#endif
