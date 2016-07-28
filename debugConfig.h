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
#undef __PRINT_MEMORY_POOL_STATUS
#undef __PRINT_DETAILED_MEMORY_POOL_STATUS

// show game's process profiling
#undef __PROFILING

// alert stack overflows
#define __ALERT_STACK_OVERFLOW

// alert VPU's overtime
#define __ALERT_VPU_OVERTIME

// alert transformation - VPU unsync warning
#undef __PRINT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU_WARNING

// avoid declaration warnings
#include <HardwareManager.h>
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);

// test memory pool's config
#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS																	        \
	__BLOCK_DEFINITION(192, 1)																			\
	__BLOCK_DEFINITION(160, 5)																			\
	__BLOCK_DEFINITION(144, 6)																			\
	__BLOCK_DEFINITION(136, 26)																			\
	__BLOCK_DEFINITION(120, 35)																			\
	__BLOCK_DEFINITION(112, 12)																			\
	__BLOCK_DEFINITION(104, 10)																			\
	__BLOCK_DEFINITION(100, 20)																			\
	__BLOCK_DEFINITION(92, 24)																			\
	__BLOCK_DEFINITION(84, 30)																			\
	__BLOCK_DEFINITION(76, 8)																			\
	__BLOCK_DEFINITION(68, 60)																			\
	__BLOCK_DEFINITION(28, 276)																			\
	__BLOCK_DEFINITION(20, 588)																			\
	__BLOCK_DEFINITION(16, 276)					    													\

// test memory pool's config
#undef __MEMORY_POOL_ARRAYS2
#define __MEMORY_POOL_ARRAYS2																			\
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
