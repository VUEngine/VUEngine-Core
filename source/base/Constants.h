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
 * 										MOVEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//axis definitions
#define __XYZAXIS		0x07 
#define __XYAXIS		0x03
#define __XZAXIS		0x05
#define __YZAXIS		0x06
#define __XAXIS 		0x01
#define __YAXIS 		0x02
#define __ZAXIS 		0x04


//movement type
#define	__UNIFORM_MOVEMENT		0x00
#define	__ACCELERATED_MOVEMENT	0x01

//state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0

//number of possible colliding objects
#define __SPRITE_LIST_SIZE			32


/*-----------------------------posible directions--------------------------*/
#define __LEFT		 ((int)-1)
#define __RIGHT		 ((int)1)
#define __UP		 ((int)-1)
#define __DOWN		 ((int)1)
#define __NEAR		 ((int)-1)
#define __FAR		 ((int)1)


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										GRAPHIC MEMORY 3D
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//left buffer base addresses
#define __LEFT_BUFFER_1 (u32)0x00000000

//right buffer base address
#define __RIGHT_BUFFER_1 (u32)0x00010000


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										IN GAME STATES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __UNLOADED	0x00
#define __LOADED	0x01

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										ANIMATION TYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//definition of a chargroup of an animated character or background
#define __ANIMATED			0x01
//definition of a chargroup of an unanimated character or background
#define __NO_ANIMATED		0x02
//definition of a chargroup of an animated character which it's all frames are written
//and shared
#define __ANIMATED_SHARED	0x03

// future expansion
#define __ANIMATED_SHARED_2	0x04

/* Animated character which it's chars are rewritten
 * within each frame change
 */
//#define	__WRITEANIMATED 		0x00
/* Animated character which it's param table is
 * rewriten within each frame change
 */
//#define	__WRITEANIMATEDSHARED	0x01
/* Animated character which it's param table
 * is written at one for all it's frames of animation
 */
//#define	__WRITEANIMATEDSHARED2	0x02
/* Non animated character
 */
//#define __WRITENOANIMATED		0x03

/*------------------------------ animation sizes---------------------------*/
//max leght of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGHT	20

// max number of frames per animation function
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION		16

// max number of animation functions per description
#define __MAX_ANIMATION_FUNCTIONS				32



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										GAME ENGINE'S MESSAGES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


enum MessagesTypes{
	
	// general porpuse messages
	kFRSareHigh = 0,
	
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




/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										RENDER FLAGS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										     DEBUG
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


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