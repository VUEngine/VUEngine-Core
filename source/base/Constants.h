/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Error.h>
#include <Globals.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

// use for faster rounding on fix19_13 values
#define __0_5F_FIX19_13		0x00001000

// override null definition (because we don't want to include standard C libraries)
#define NULL 		(void *)0x00000000

// axis definitions
#define __XAXIS 	0x01
#define __YAXIS 	0x02
#define __ZAXIS 	0x04

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
	kHighFPS = 0,
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

	// don't place messages below this:
	kLastEngineMessage
};

#define NM_ASSERT(STATEMENT, ...)																		\
	 																									\
	if(!(STATEMENT))																					\
	{ 																									\
		asm(" mov sp,%0  ": "=r" (_sp));																\
		asm(" mov lp,%0  ": "=r" (_lp));																\
																										\
		/* thrown exception */																			\
		Error_triggerException(Error_getInstance(), __MAKE_STRING(__VA_ARGS__), NULL);					\
	}

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
