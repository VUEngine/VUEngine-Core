/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Error.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

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
	kBodyStoped,
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

#define NM_ASSERT(STATEMENT, ...)														\
	 																					\
	if(!(STATEMENT))																	\
	{ 																					\
		/* thrown exception */															\
		Error_triggerException(Error_getInstance(), __MAKE_STRING(__VA_ARGS__), NULL);	\
	}

#undef ASSERT

#ifndef __DEBUG
	#define ASSERT(Statement, ...)

#else
#define ASSERT(Statement, Message)														\
	 																					\
	if(!(Statement)) 																	\
	{																					\
		int sp;																			\
		asm(" mov sp,%0  ": "=r" (sp));													\
		int lp;																			\
		asm(" mov lp,%0  ": "=r" (lp));													\
		int x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 48 / 2 ? 				\
				__EXCEPTION_COLUMN : 0;													\
		int y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? 						\
				__EXCEPTION_LINE : 0;													\
		y += 2; 																		\
		Printing_text(Printing_getInstance(), " SP:" , x, y, NULL);						\
		Printing_hex(Printing_getInstance(), sp, x + 7, y, NULL);						\
		Printing_text(Printing_getInstance(), " LP:" , x, ++y, NULL);					\
		Printing_hex(Printing_getInstance(), lp, x + 7, y, NULL);						\
																						\
		/* thrown exception */															\
		Error_triggerException(Error_getInstance(), Message, NULL);						\
	}
#endif


#endif