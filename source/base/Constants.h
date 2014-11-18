/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
#define	__UNIFORMMOVE	0x00
#define	__ACCELMOVEX	0x01
#define	__ACCELMOVEY	0x02
#define	__ACCELMOVEZ	0x04
#define	__RETARMOVEX	0x10
#define	__RETARMOVEY	0x20
#define	__RETARMOVEZ	0x40

//state of movement
#define __ACTIVE 		(int)0x1
#define __PASSIVE		(int)0x0

//rendering order
#define __FORWARD		0x01
#define __BACKWARD		0x02

//number of possible colliding objects
#define __OBJECTLISTTAM			32
//displacement increment to align to a colliding object
#define __COLLISIONDISP 		0.5f
#define __COLLISIONDISPY		2
#define __COLLISIONMULTIPLIER	2


/*-----------------------------posible directions--------------------------*/
#define __LEFT		 ((int)-1)
#define __RIGHT		 ((int)1)
#define __UP		 ((int)-1)
#define __DOWN		 ((int)1)
#define __NEAR		 ((int)-1)
#define __FAR		 ((int)1)

//Collision types
#define __NOCOLLISION		0x00
#define __NORMALCOLLISION	0x01
#define __INSIDEOBJECTX		0x02
#define __INSIDEOBJECTY		0x03



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										GRAPHIC MEMORY 3D
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//left buffer base addresses
#define __LEFTBUFFER1 (u32)0x00000000
#define __LEFTBUFFER2 (u32)0x00008000

//right buffer base address
#define __RIGHTBUFFER1 (u32)0x00010000
#define __RIGHTBUFFER2 (u32)0x00018000

//maximun number of solids being rendered
#define __MAXSOLIDS		16

//maximun number of vertex per polygon
#define __MAXVERTEXS	4

//maximun number of polygons per solid
#define __MAXPOLYS		16


/*-------------------------Collision logic results--------------------------*/
#define __STOPMOVEMENT		0x01
#define __NOUPDATEXPOSITION	0x02
#define __NOUPDATEYPOSITION	0x04
#define __NOUPDATEZPOSITION	0x08
#define __NOSTOPMOVEMENT	0x10
#define __NOTALIGN			0x20


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
 * 										IN GAME OBJECT TYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//define an object type
//don't override this values with another in game type
#define __NOSOLID		1
#define __SOLID			2
#define __TRANSPARENT	3

/*--------------------------general object's types-------------------------*/
#define __NOTYPE	 	(int)0
#define __CHARACTER 	(int)1
#define __OBJCHARACTER	(int)2
#define __SCROLL 		(int)3
#define __TEXTBOX 		(int)4
#define __BACKGROUND	(int)5


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										GAME ENGINE'S STATES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//masks to define the kind of state 
#define __MASK_PAUSE_ST		(int)0x0080
#define __MASK_RUNNING_ST	(int)0x8000

//game engine's states
#define __PRECAUTION_ST	(int)0x0000
#define __INITFOCUS_ST	(int)0x0001
#define __VBJAE_ST		(int)0x0002
#define __CREDITS_ST	(int)0x0003
#define __RESUME_ST		(int)0x0004
#define __REST_ST		(int)0x0005
#define __AUTOPAUSE_ST	(int)0x0006

/* mask every pause state with 0x0080 in order
 * to always preserve the gameplay gameworld
 */
#define __PAUSE_ST		((int)0x0000 | __MASK_PAUSE_ST)
#define __OPTIONS_ST	((int)0x0001 | __MASK_PAUSE_ST)


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


#define __UPDATEHEAD	0x0F
#define __UPDATEG		0x01
#define __UPDATEPARAM	0x02
#define __UPDATESIZE	0x04
#define __UPDATEM		0x08


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