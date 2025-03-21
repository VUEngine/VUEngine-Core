/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef	DEBUG_CONFIG_H_
#undef	DEBUG_CONFIG_H_

// avoid declaration warnings

// define / undefine as you see fit
//#define __PRINT_FRAMERATE
//#define __PRINT_FRAME_TIMES
//#define __PROFILE_STREAMING
//#define __SHOW_STREAMING_PROFILING
//#define __SHOW_VIP_STATUS
//#define __SHOW_SPRITES_PROFILING
//#define __DIMM_FOR_PROFILING
//#define __SHOW_VIP_OVERTIME_COUNT
//#define __DRAW_SHAPES
//#define __SHOW_PROCESS_NAME_DURING_FRAMESTART
//#define __SHOW_PROCESS_NAME_DURING_XPEND
//#define __SHOW_STACK_OVERFLOW_ALERT
//#define __SHOW_DETAILED_MEMORY_POOL_STATUS
//#define __SHOW_MEMORY_POOL_STATUS
//#define __SHOW_PHYSICS_PROFILING
//#define __SHOW_WIREFRAME_MANAGER_STATUS
//#define __SHOW_CAMERA_STATUS
//#define __SHOW_CHAR_MEMORY_STATUS
//#define __SHOW_BGMAP_MEMORY_STATUS
//#define __SHOW_SOUND_STATUS
//#define __FORCE_PRINTING_LAYER
//#define __FORCE_UPPERCASE
//#define __MAXIMUM_BOUNCINESS_COEFFICIENT		5.5f
//#define __FORCE_FONT "GuiTime"
//#define __DEFAULT_FONT "GuiTime"
//#define __MUTE_ALL_SOUND
//#define __DRAW_COMPLETE_BOXES
//#define __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
//#define __SHOW_TIMER_MANAGER_STATUS
//#define __PROFILE_DIRECT_DRAWING
//#define __DIRECT_DRAW_INTERLACED
//#define __PROFILE_WIREFRAMES

// do not delete the following macros!

#ifdef __PRINT_FRAME_TIMES
#undef __PRINT_FRAMERATE
#define __PRINT_FRAMERATE
#endif

#undef __SHOW_PROCESS_NAME_DURING_GAMESTART
#undef __SHOW_PROCESS_NAME_DURING_FRAMESTART
#undef __SHOW_PROCESS_NAME_DURING_XPEND

// show games's profiling during game
#ifdef __ENABLE_PROFILER
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#endif

// show streaming's profiling during game
#ifdef __SHOW_STREAMING_PROFILING
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#endif

#ifdef __SHOW_VIP_STATUS
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

// show sprites's profiling during game
#ifdef __SHOW_SPRITES_PROFILING
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

// enable game profiling when checking the VIP interrupt
#ifdef __ENABLE_PROFILER
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

#ifdef __SHOW_DETAILED_MEMORY_POOL_STATUS
#undef __SHOW_MEMORY_POOL_STATUS
#define __SHOW_MEMORY_POOL_STATUS
#undef __DIMM_FOR_PROFILING
#define __DIMM_FOR_PROFILING
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

#ifdef __SHOW_PHYSICS_PROFILING
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

#ifdef __PRINT_FRAME_TIMES
#undef __FORCE_PRINTING_LAYER
#define __FORCE_PRINTING_LAYER
#endif

#endif
#undef __DIMM_FOR_PROFILING
