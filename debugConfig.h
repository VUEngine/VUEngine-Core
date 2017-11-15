#ifndef	DEBUG_CONFIG_H_
#undef	DEBUG_CONFIG_H_

#include <debugUtilities.h>

// avoid declaration warnings
#include <HardwareManager.h>
void HardwareManager_checkStackStatus(HardwareManager this);
void HardwareManager_printStackStatus(HardwareManager this, int x, int y, bool resumed);

// define / undefine as you see fit
//#undef __LOW_BATTERY_INDICATOR
//#define __PRINT_FRAMERATE
//#define __PROFILE_GAME
//#define __PROFILE_STREAMING
//#define __SHOW_GAME_PROFILING
//#define __SHOW_GAME_DETAILED_PROFILING
//#define __SHOW_GAME_PROFILE_DURING_TORN_FRAMES
//#define __SHOW_STREAMING_PROFILING
//#define __SHOW_SPRITES_PROFILING
//#define __DIMM_FOR_PROFILING
//#define __ALERT_VIP_OVERTIME
//#undef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
//#define __DRAW_SHAPES
//#define __REGISTER_LAST_PROCESS_NAME
//#define __ALERT_STACK_OVERFLOW
//#define __PRINT_MEMORY_POOL_STATUS
//#define __PRINT_DETAILED_MEMORY_POOL_STATUS
//#define __FORCE_PRINTING_LAYER

//#undef __TIMER_RESOLUTION
//#define __TIMER_RESOLUTION						1


// do not delete the following macros!

// show games's profiling during game
#ifdef __SHOW_GAME_DETAILED_PROFILING
#undef __PRINT_FRAMERATE
#undef __SHOW_GAME_PROFILING
#define __SHOW_GAME_PROFILING
#endif

#ifdef __SHOW_GAME_PROFILE_DURING_TORN_FRAMES
#undef __PROFILE_GAME
#define __PROFILE_GAME
#endif

// show games's profiling during game
#ifdef __SHOW_GAME_PROFILING
#undef __PROFILE_GAME
#define __PROFILE_GAME
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#endif

// show streaming's profiling during game
#ifdef __SHOW_STREAMING_PROFILING
#undef __PROFILE_GAME
#define __PROFILE_GAME
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#undef __PROFILE_STREAMING
#define __PROFILE_STREAMING
#endif

// show sprites's profiling during game
#ifdef __SHOW_SPRITES_PROFILING
#undef __PROFILE_GAME
#define __PROFILE_GAME
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#endif

// enable game profiling when checking the VIP interrupt
#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
#define __PROFILE_GAME
#define __DIMM_FOR_PROFILING
#endif

#ifdef __PRINT_DETAILED_MEMORY_POOL_STATUS
#define __PRINT_MEMORY_POOL_STATUS
#endif

#ifdef __ALERT_STACK_OVERFLOW
#undef __PRINT_PROFILING_INFO
#define __PRINT_PROFILING_INFO
#endif

#ifdef __PRINT_FRAMERATE
#ifndef __PRINT_PROFILING_INFO
#define __PRINT_PROFILING_INFO
#endif
#else
#ifdef __PROFILE_GAME
#ifndef __PRINT_PROFILING_INFO
#define __PRINT_PROFILING_INFO
#endif
#else
#ifdef __PRINT_MEMORY_POOL_STATUS
#ifndef __PRINT_PROFILING_INFO
#define __PRINT_PROFILING_INFO
#endif
#else
#ifdef __SHOW_STREAMING_PROFILING
#ifndef __PRINT_PROFILING_INFO
#define __PRINT_PROFILING_INFO
#endif
#endif
#endif
#endif
#endif

#endif
