/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


#include <Error.h>
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 							!!!!!The next values must NOT be changed!!!!!!
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//override null definition (because we don't want to include standard C libraries)
#define NULL 	(void *)0x00000000

// booleans
#define true 	(u8)1	
#define false 	(u8)0

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										AXES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//axis definitions
#define __XAXIS 		0x01
#define __YAXIS 		0x02
#define __ZAXIS 		0x04


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										MESSAGES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


enum MessagesTypes{
	
	// general porpuse messages
	kHighFPS = 0,
	
	// graphic system's messages
	kCharGroupRewritten,
	
	// physics messages
	kNoCollision,
	kCollision,
	kFloorReached,
	kShapeBelow,
	kNoObjectBelow,
	kCollisionBelow,
	kKeyPressed,
	kKeyUp,
	kKeyHold,
	kEntityRemoved,
	kBodyStoped,
	kBodyBounced,
	kBodyStartedMoving,
	kBodyStartedMovingByGravity,
	kBodyChangedDirection,
	kBodySleep,
	
	
	// don't place messages below this:
	kLastEngineMessage
};




#define NM_ASSERT( STATEMENT, ... )																\
	if(!(STATEMENT)) { 																\
		/* thrown exception */														\
		Error_triggerException(Error_getInstance(), __MAKE_STRING(__VA_ARGS__));	\
	}

#undef ASSERT

#ifndef __DEBUG
	#define ASSERT( STATEMENT, ... )

#else
	#define ASSERT( STATEMENT, MESSAGE )											\
	if(!(STATEMENT)) { 																\
		/* thrown exception */														\
		Error_triggerException(Error_getInstance(), MESSAGE);						\
	}
#endif

#endif /*CONSTANTS_H_*/