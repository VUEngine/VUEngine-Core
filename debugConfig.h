#ifndef	DEBUG_CONFIG_H_
#define	DEBUG_CONFIG_H_

// print frame rate
#define __PRINT_FRAMERATE

// define the timer's resolution used by the physics
#undef __TIMER_RESOLUTION
#define __TIMER_RESOLUTION      10

// set  frame rate (0: 50, 1: 25)
#undef	__FRAME_CYCLE
#define	__FRAME_CYCLE		    0

// print memory pool's status
#define __PRINT_MEMORY_POOL_STATUS
#undef __PRINT_DETAILED_MEMORY_POOL_STATUS

// show game's process profiling
#undef __PROFILING

// alert stack overflows
#define __ALERT_STACK_OVERFLOW

// define to place the whole memory pool in SRAM
#undef __PUT_MEMORY_POOL_IN_SRAM

// test memory pool's config
#undef __MEMORY_POOL_ARRAYS1
#define __MEMORY_POOL_ARRAYS1																	        \
	__BLOCK_DEFINITION(192, 1)																			\
	__BLOCK_DEFINITION(160, 10)																			\
	__BLOCK_DEFINITION(144, 15)																			\
	__BLOCK_DEFINITION(136, 35)																			\
	__BLOCK_DEFINITION(120, 40)																			\
	__BLOCK_DEFINITION(112, 15)																			\
	__BLOCK_DEFINITION(104, 10)																			\
	__BLOCK_DEFINITION(100, 25)																			\
	__BLOCK_DEFINITION(92, 30)																			\
	__BLOCK_DEFINITION(84, 30)																			\
	__BLOCK_DEFINITION(76, 10)																			\
	__BLOCK_DEFINITION(68, 60)																			\
	__BLOCK_DEFINITION(28, 300)																			\
	__BLOCK_DEFINITION(20, 600)																			\
	__BLOCK_DEFINITION(16, 350)

// test memory pool's config
#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(192, 1)																			\
	__BLOCK_DEFINITION(160, 5)																			\
	__BLOCK_DEFINITION(144, 5)																			\
	__BLOCK_DEFINITION(136, 24)																			\
	__BLOCK_DEFINITION(120, 30)																			\
	__BLOCK_DEFINITION(112, 12)																			\
	__BLOCK_DEFINITION(104, 10)																			\
	__BLOCK_DEFINITION(100, 20)																			\
	__BLOCK_DEFINITION(92, 22)																			\
	__BLOCK_DEFINITION(84, 25)																			\
	__BLOCK_DEFINITION(76, 8)																			\
	__BLOCK_DEFINITION(68, 52)																			\
	__BLOCK_DEFINITION(28, 250)																			\
	__BLOCK_DEFINITION(20, 550)																			\
	__BLOCK_DEFINITION(16, 250)																		\


#endif
