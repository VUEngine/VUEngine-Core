#ifndef	DEBUG_CONFIG_H_
#define	DEBUG_CONFIG_H_

#ifdef __DEBUG
#define __DEBUG_NO_FADE
#endif

#include <debugUtilities.h>


#define __ALERT_STACK_OVERFLOW
//#define __DEBUG_NO_FADE

#undef __TIMER_RESOLUTION						
#define __TIMER_RESOLUTION						10

//#define __DEBUG_NO_FADE
#define __PRINT_FRAMERATE
#undef	__FRAME_CYCLE
#define	__FRAME_CYCLE							1

#undef __LIGHT_SPEED
#define __LIGHT_SPEED		ITOFIX19_13(50000)

#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(188, 1)																			\
	__BLOCK_DEFINITION(168, 4)																			\
	__BLOCK_DEFINITION(140, 28)																			\
	__BLOCK_DEFINITION(128, 8)																			\
	__BLOCK_DEFINITION(112, 58)																			\
	__BLOCK_DEFINITION(96, 60)																			\
	__BLOCK_DEFINITION(76, 28)																			\
	__BLOCK_DEFINITION(68, 64)																			\
	__BLOCK_DEFINITION(28, 240)																			\
	__BLOCK_DEFINITION(20, 612)																			\
	__BLOCK_DEFINITION(16, 234)																		\

#define __SET_MEMORY_POOL_ARRAYS																		\
	__SET_MEMORY_POOL_ARRAY(188)																		\
	__SET_MEMORY_POOL_ARRAY(168)																		\
	__SET_MEMORY_POOL_ARRAY(140)																		\
	__SET_MEMORY_POOL_ARRAY(128)																		\
	__SET_MEMORY_POOL_ARRAY(112)																		\
	__SET_MEMORY_POOL_ARRAY(96)																			\
	__SET_MEMORY_POOL_ARRAY(76)																			\
	__SET_MEMORY_POOL_ARRAY(68)																			\
	__SET_MEMORY_POOL_ARRAY(28)																			\
	__SET_MEMORY_POOL_ARRAY(20)																			\
	__SET_MEMORY_POOL_ARRAY(16)	

#endif