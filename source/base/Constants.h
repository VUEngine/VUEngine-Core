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

// Screen half width in pixels
#define __HALF_SCREEN_WIDTH						(__SCREEN_WIDTH >> 1)

// Screen half height in pixels
#define __HALF_SCREEN_HEIGHT					(__SCREEN_HEIGHT >> 1)

// Screen half depth in pixels
#define __HALF_SCREEN_DEPTH						(__SCREEN_DEPTH >> 1)

// Screen width in chars
#define __SCREEN_WIDTH_IN_CHARS					(__SCREEN_WIDTH >> 3)

// Screen height in chars
#define __SCREEN_HEIGHT_IN_CHARS				(__SCREEN_HEIGHT >> 3)

// Screen depth in chars
#define __SCREEN_DEPTH_IN_CHARS					(__SCREEN_DEPTH >> 3)

// Screen half width in chars
#define __HALF_SCREEN_WIDTH_IN_CHARS			(__SCREEN_WIDTH >> 4)

// Screen half height in chars
#define __HALF_SCREEN_HEIGHT_IN_CHARS			(__SCREEN_HEIGHT >> 4)

// Screen half depth in chars
#define __HALF_SCREEN_DEPTH_IN_CHARS			(__SCREEN_DEPTH >> 4)


// used for exceptions
#define __EXCEPTIONS_BGMAP		0
#define __EXCEPTIONS_WORLD		31

// use for faster rounding on fix* values
#define __1I_FIX7_9 		0x00000200
#define __1I_FIX19_13		0x00002000
#define __0_5F_FIX19_13		0x00001000

// override null definition (because we don't want to include standard C libraries)
#define NULL 		(void *)0x00000000

// axis definitions
#define __X_AXIS 	0x01
#define __Y_AXIS 	0x02
#define __Z_AXIS 	0x04

// direction
#define __LEFT		((int)-1)
#define __RIGHT		((int)1)
#define __UP		((int)-1)
#define __DOWN		((int)1)
#define __NEAR		((int)-1)
#define __FAR		((int)1)

#define __MILLISECONDS_IN_SECOND				1000

// messages
enum MessagesTypes
{
	// general purpose messages
	kHighFps = 0,
	kAutoPause,
	kLowBatteryIndicator,
	kEntityRemoved,

	// physics messages
	kNoCollision,
	kCollision,
	kCollisionWithYou,
	kBodyStopped,
	kBodyBounced,
	kBodyStartedMoving,
	kBodyStartedMovingByGravity,
	kBodyChangedDirection,
	kBodySleep,

	// keypad massages
	kKeyPressed,
	kKeyReleased,
	kKeyHold,

	// don't place messages below this
	kLastEngineMessage
};

#undef NM_ASSERT

#ifndef __RELEASE
#define NM_ASSERT(Statement, ...)																		\
	 																									\
	if(!(Statement))																					\
	{ 																									\
		asm(" mov sp,%0  ": "=r" (_sp));																\
		asm(" mov lp,%0  ": "=r" (_lp));																\
																										\
		/* thrown exception */																			\
		Error_triggerException(Error_getInstance(), __MAKE_STRING(__VA_ARGS__), NULL);					\
	}
#else
	#define NM_ASSERT(Statement, ...)
#endif

#undef ASSERT

#ifndef __DEBUG
	#define ASSERT(Statement, ...)
#else
#define ASSERT(Statement, Message)																		\
	 																									\
	if(!(Statement)) 																					\
	{																									\
		asm(" mov sp,%0  ": "=r" (_sp));																\
		asm(" mov lp,%0  ": "=r" (_lp));																\
																										\
		/* thrown exception */																			\
		Error_triggerException(Error_getInstance(), Message, NULL);										\
	}
#endif


#endif
