#ifndef	DEBUG_CONFIG_H_
#undef	DEBUG_CONFIG_H_

#include <debugUtilities.h>

// avoid declaration warnings
#include <HardwareManager.h>
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);

// define / undefine as you see fit
//#define __PRINT_FRAMERATE
//#define __PROFILE_GAME
//#define __PROFILE_STREAMING
//#define __SHOW_GAME_PROFILING
//#define __SHOW_STREAMING_PROFILING
//#define __DIMM_FOR_PROFILING
//#define __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
//#define __ALERT_VIP_OVERTIME
//#undef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
//#define __DRAW_SHAPES

//#define __PRINT_MEMORY_POOL_STATUS
//#define __PRINT_DETAILED_MEMORY_POOL_STATUS

//#define __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VIP
//#define __ALERT_STACK_OVERFLOW

//#undef __TIMER_RESOLUTION
//#define __TIMER_RESOLUTION	1
//#undef __TIMER_FREQUENCY
//#define __TIMER_FREQUENCY	__TIMER_20US


// do not delete the following macros!

// show games's profiling during game
#ifdef __SHOW_GAME_PROFILING
#define __PROFILE_GAME
#endif

// show streaming's profiling during game
#ifdef __SHOW_STREAMING_PROFILING
#define __PROFILE_STREAMING
#endif

#endif
