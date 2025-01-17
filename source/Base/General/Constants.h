/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Globals.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// messages
enum MessagesTypes
{
	kMessageNone = 0,

	// General purpose messages
	kMessageHighFps,
	kMessageActorRemoved,

	// Keypad massages
	kMessageKeyPressed,
	kMessageKeyReleased,
	kMessageKeyHold,

	// Communication messages
	kMessageCheckIfRemoteIsReady,
	kMessageLastCommunicationMessage,

	// Framerate
	kMessageRestoreFPS,

	// Physics messages
	kMessageBodyStopped,
	kMessageBodyBounced,
	kMessageBodyStartedMoving,
	kMessageColliderShow,
	kMessageColliderHide,

	// Actor messages
	kMessageShow,
	kMessageHide,
	kMessageSetTransparency,

	// Don't place messages below this
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Memory manipulation
#define __MEMORY_USED_BLOCK_FLAG				((uint16)0xFFFF)
#define __MEMORY_FREE_BLOCK_FLAG				0x00000000

// Graphics
#define __TRANSPARENCY_NONE						0
#define __TRANSPARENCY_ODD						1
#define __TRANSPARENCY_EVEN						2

#define __NO_RENDER_INDEX						-1

#define	__COLOR_BLACK							0x00
#define	__COLOR_DARK_RED						0x01
#define	__COLOR_MEDIUM_RED						0x02
#define	__COLOR_BRIGHT_RED						0x03

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

// Camera's FOV (128 fix7_9 = 90)
#define __CAMERA_FOV_DEGREES					128

// used for exceptions
#define __EXCEPTIONS_BGMAP						0
#define __EXCEPTIONS_WORLD						31

// override null spec (because we don't want to include standard C libraries)
#define NULL 									(void *)0x00000000

// axis specs
#define __NO_AXIS 								0x00
#define __X_AXIS 								0x01
#define __Y_AXIS 								0x02
#define __Z_AXIS 								0x04
#define __ALL_AXIS								(__X_AXIS | __Y_AXIS | __Z_AXIS)
#define __LOCK_AXIS								(~__ALL_AXIS)

// direction
#define __LEFT									((int32)-1)
#define __RIGHT									((int32)1)
#define __UP									((int32)-1)
#define __DOWN									((int32)1)
#define __NEAR									((int32)-1)
#define __FAR									((int32)1)

// Transformations
#define __VALID_TRANSFORMATION					0x00
#define __NON_TRANSFORMED						0xFF
#define __INVALIDATE_TRANSFORMATION				0x0F
#define __INVALIDATE_POSITION					0x01
#define __INVALIDATE_ROTATION					0x02
#define __INVALIDATE_SCALE						0x04
#define __INVALIDATE_PROJECTION					0x08

#define __INHERIT_NONE							__VALID_TRANSFORMATION
#define __INHERIT_TRANSFORMATION				__INVALIDATE_TRANSFORMATION
#define __INHERIT_POSITION						__INVALIDATE_POSITION
#define __INHERIT_ROTATION						__INVALIDATE_ROTATION
#define __INHERIT_SCALE							__INVALIDATE_SCALE

#define __MAXIMUM_FPS							50

#define __MILLISECONDS_PER_SECOND				1000
#define __MICROSECONDS_PER_MILLISECOND			1000
#define __MICROSECONDS_PER_SECOND				(__MILLISECONDS_PER_SECOND * __MICROSECONDS_PER_MILLISECOND)

#define __PIXELS_PER_METER						16
#define __METERS_PER_PIXEL						__F_TO_FIXED(1.0f/(float)__PIXELS_PER_METER)

#define __PIXELS_PER_METER_2_POWER				4
#define __PIXELS_TO_METERS(pixels)				(fixed_t)((pixels) << (__FIXED_TO_I_BITS - __PIXELS_PER_METER_2_POWER))
#define __REAL_PIXELS_TO_METERS(pixels)			(fixed_t)(__F_TO_FIXED_EXT(pixels) >> __PIXELS_PER_METER_2_POWER)
#define __METERS_TO_PIXELS(meters)				(((fixed_ext_t)(meters)) >> (__FIXED_TO_I_BITS - __PIXELS_PER_METER_2_POWER))

#define __SCREEN_WIDTH_METERS					__PIXELS_TO_METERS(__SCREEN_WIDTH)
#define __SCREEN_HEIGHT_METERS					__PIXELS_TO_METERS(__SCREEN_HEIGHT)
#define __SCREEN_DEPTH_METERS					__PIXELS_TO_METERS(__SCREEN_DEPTH)

#define __MINIMUM_X_VIEW_DISTANCE_POWER			4
#define __MINIMUM_Y_VIEW_DISTANCE_POWER			4

#define __HIDE									0
#define __SHOW									1

#define __STRINGIFY(a)							__MAKE_STRING(a)

// Optimizations
#define __LIKELY(expression)					(__builtin_expect(!!(expression), 1))
#define __UNLIKELY(expression)					(__builtin_expect(!!(expression), 0))

#ifndef __RELEASE
#define __REGISTER_LAST_PROCESS_NAME

void HardwareManager_printStackStatus(int32 x, int32 y, bool resumed);
void Error_triggerException(char* message, char* detail);

#define __CHECK_STACK_STATUS																			\
	extern bool _stackHeadroomViolation;																\
	if(!_stackHeadroomViolation)																		\
	{																									\
		int32 _vuengineStackPointer;																	\
		asm(" mov sp,%0  ": "=r" (_vuengineStackPointer));												\
																										\
		if((0x05000000 & _vuengineStackPointer) &&														\
			_vuengineStackPointer - __STACK_HEADROOM < (int32)&_bssEnd)									\
		{																								\
			_stackHeadroomViolation = true;																\
			/* HardwareManager_printStackStatus(1, 15, false); */										\
			NM_ASSERT(false, "HardwareManager_checkStack: surpassed headroom boundary!");				\
		}																								\
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

#endif
