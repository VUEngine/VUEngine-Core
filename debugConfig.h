#ifndef	DEBUG_CONFIG_H_
#define	DEBUG_CONFIG_H_

#ifdef __DEBUG
#define __DEBUG_NO_FADE
#endif

#include <debugUtilities.h>

#define __0_5F_FIX19_13		0x00001000

#define __ALERT_STACK_OVERFLOW
//#define __DEBUG_NO_FADE

#undef __TIMER_RESOLUTION						
#define __TIMER_RESOLUTION						10

//#define __DEBUG_NO_FADE
#define __PRINT_FRAMERATE
#undef	__FRAME_CYCLE
#define	__FRAME_CYCLE							0

/*
#undef __MEMORY_POOL_ARRAYS																			
#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(188, 1)																			\
	__BLOCK_DEFINITION(164, 4)																			\
	__BLOCK_DEFINITION(132, 30)																			\
	__BLOCK_DEFINITION(112, 52)																			\
	__BLOCK_DEFINITION(96, 58)																			\
	__BLOCK_DEFINITION(76, 32)																			\
	__BLOCK_DEFINITION(68, 75)																			\
	__BLOCK_DEFINITION(28, 254)																			\
	__BLOCK_DEFINITION(20, 632)																			\
	__BLOCK_DEFINITION(16, 290)	
*/

/*
// these improve performance in the real machine
#undef __OPTICS_NORMALIZE
#define __OPTICS_NORMALIZE(Vector)													\
	Vector.x -= (_screenPosition->x + 0x00001000)& 0xFFFFE000;						\
	Vector.y -= (_screenPosition->y + 0x00001000)& 0xFFFFE000;						\
	Vector.z -= (_screenPosition->z + 0x00001000)& 0xFFFFE000;

#undef __OPTICS_PROJECT_TO_2D
#define __OPTICS_PROJECT_TO_2D(Vector3D, Vector2D)									\
		Vector2D.x = Vector3D.x 													\
			+ (FIX19_13_MULT(_optical->horizontalViewPointCenter - 					\
					Vector3D.x, Vector3D.z) >> _optical->maximumViewDistancePower);	\
		Vector2D.y = Vector3D.y 													\
			- (FIX19_13_MULT(Vector3D.y - _optical->verticalViewPointCenter,		\
				Vector3D.z) >> _optical->maximumViewDistancePower);					\
		Vector2D.z = Vector3D.z;													\
		Vector2D.x += 0x00001000;													\
		Vector2D.y += 0x00001000;													\
		Vector2D.x &= 0xFFFFE000;													\
		Vector2D.y &= 0xFFFFE000;					
*/

#endif