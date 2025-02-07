////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////  THIS FILE WAS AUTO-GENERATED - DO NOT EDIT  ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H_
#define CONFIG_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include "PluginsConfig.h"
#include "RomInfo.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// GAME ENTRY POINT
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __GAME_ENTRY_POINT
#define __GAME_ENTRY_POINT game
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FIXED POINT DATA TYPE
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __FIXED_POINT_TYPE 6

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// COMMUNICATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ENABLE_COMMUNICATIONS

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DEBUGGING / PROFILING
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __STACK_HEADROOM 500

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DEBUGGING TOOLS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __TOOLS

#ifndef __DEBUG_TOOL
#define __DEBUG_TOOL
#endif

#ifndef __STAGE_EDITOR
#define __STAGE_EDITOR
#endif

#ifndef __ANIMATION_INSPECTOR
#define __ANIMATION_INSPECTOR
#endif

#ifndef __SOUND_TEST
#define __SOUND_TEST
#endif

#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// WIREFRAMES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#undef __WIREFRAME_MANAGER_SORT_FOR_DRAWING

#define __DIRECT_DRAW_INTERLACED_THRESHOLD	  __PIXELS_TO_METERS(4096)  

#define __DIRECT_DRAW_LINE_SHRINKING_PADDING  0 

#define __DIRECT_DRAW_FRUSTUM_EXTENSION_POWER 0 

#undef __DIRECT_DRAW_OPTIMIZED_VERTICAL_LINES

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// OPTICS / PROJECTION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __LEGACY_COORDINATE_PROJECTION

#define __SCREEN_WIDTH				   384
#define __SCREEN_HEIGHT				   224
#define __SCREEN_DEPTH				   2048

#define __MAXIMUM_X_VIEW_DISTANCE	   1024
#define __MAXIMUM_Y_VIEW_DISTANCE	   1024

#define __CAMERA_NEAR_PLANE			   0

#define __BASE_FACTOR				   21

#define __HORIZONTAL_VIEW_POINT_CENTER 192
#define __VERTICAL_VIEW_POINT_CENTER   112

#define __SCALING_MODIFIER_FACTOR	   1.0f

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FRAME RATE CONTROL
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TIMER_RESOLUTION			   10

#define __FRAME_CYCLE				   0

#define __TARGET_FPS				   (__MAXIMUM_FPS >> __FRAME_CYCLE)
#define __GAME_FRAME_DURATION		   (__MILLISECONDS_PER_SECOND / __TARGET_FPS)
#define __OPTIMUM_FPS				   (__TARGET_FPS >> __FRAME_CYCLE)

#define __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ANIMATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH 10
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION	 241

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// MEMORY POOL
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#undef __MEMORY_POOL_CLEAN_UP

#undef __MEMORY_POOLS
#define __MEMORY_POOLS 13

#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS \
	__BLOCK_DEFINITION(356, 1) \
	__BLOCK_DEFINITION(332, 8) \
	__BLOCK_DEFINITION(188, 1) \
	__BLOCK_DEFINITION(168, 25) \
	__BLOCK_DEFINITION(144, 20) \
	__BLOCK_DEFINITION(112, 25) \
	__BLOCK_DEFINITION(100, 50) \
	__BLOCK_DEFINITION(84, 55) \
	__BLOCK_DEFINITION(48, 90) \
	__BLOCK_DEFINITION(32, 190) \
	__BLOCK_DEFINITION(20, 965) \
	__BLOCK_DEFINITION(16, 265) \
	__BLOCK_DEFINITION(12, 265) \

#undef __SET_MEMORY_POOL_ARRAYS
#define __SET_MEMORY_POOL_ARRAYS \
	__SET_MEMORY_POOL_ARRAY(356) \
	__SET_MEMORY_POOL_ARRAY(332) \
	__SET_MEMORY_POOL_ARRAY(188) \
	__SET_MEMORY_POOL_ARRAY(168) \
	__SET_MEMORY_POOL_ARRAY(144) \
	__SET_MEMORY_POOL_ARRAY(112) \
	__SET_MEMORY_POOL_ARRAY(100) \
	__SET_MEMORY_POOL_ARRAY(84) \
	__SET_MEMORY_POOL_ARRAY(48) \
	__SET_MEMORY_POOL_ARRAY(32) \
	__SET_MEMORY_POOL_ARRAY(20) \
	__SET_MEMORY_POOL_ARRAY(16) \
	__SET_MEMORY_POOL_ARRAY(12) \

#define __MEMORY_POOL_WARNING_THRESHOLD 85

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// SRAM
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_SAVE_RAM				8192

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CHAR MANAGEMENT
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __CHAR_MEMORY_TOTAL_CHARS		2048

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// SPRITE MANAGEMENT
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_LAYERS					32
#define __TOTAL_OBJECTS					256
#undef __SPRITE_ROTATE_IN_3D
#define __HACK_BGMAP_SPRITE_HEIGHT

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// TEXTURE MANAGEMENT
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_NUMBER_OF_BGMAPS_SEGMENTS 14
#define __PARAM_TABLE_SEGMENTS			  0
#define __NUM_BGMAPS_PER_SEGMENT		  10
#define __MAX_NUMBER_OF_BGMAPS_SEGMENTS	  (__TOTAL_NUMBER_OF_BGMAPS_SEGMENTS - __PARAM_TABLE_SEGMENTS)

#define __PRINTING_BGMAP_X_OFFSET		  0
#define __PRINTING_BGMAP_Y_OFFSET		  (64 * 8 - __SCREEN_HEIGHT)
#define __PRINTING_BGMAP_PARALLAX_OFFSET  0
#define __PRINTABLE_BGMAP_AREA			  1792

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// PARAM TABLE
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAXIMUM_SCALE					  2
#define __MAXIMUM_AFFINE_ROWS_PER_CALL	  16

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// PHYSICS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __GRAVITY						  10.0f
#define __MAXIMUM_FRICTION_COEFFICIENT	  __I_TO_FIXED(10)
#define __PHYSICS_TIME_ELAPSED_DIVISOR	  1
#undef __PHYSICS_HIGH_PRECISION
#define __STOP_VELOCITY_THRESHOLD				 __PIXELS_TO_METERS(8)
#define __STOP_BOUNCING_VELOCITY_THRESHOLD		 __PIXELS_TO_METERS(48)
#define __MAXIMUM_BOUNCINESS_COEFFICIENT		 1.0f
#define __FRICTION_FORCE_FACTOR_POWER			 2
#define __COLLIDER_ANGLE_TO_PREVENT_DISPLACEMENT __FIX7_9_TO_FIXED(__COS(10))
#define __COLLIDER_MAXIMUM_SIZE					 __PIXELS_TO_METERS(256)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// SOUND
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __EAR_DISPLACEMENT						 192
#define __SOUND_STEREO_ATTENUATION_DISTANCE		 512

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// BRIGHTNESS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __BRIGHTNESS_DARK_RED					 32
#define __BRIGHTNESS_MEDIUM_RED					 64
#define __BRIGHTNESS_BRIGHT_RED					 128

#define __FADE_DELAY							 50
#define __CAMERA_EFFECT_FADE_INCREMENT			 8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// COLOR PALETTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __PRINTING_PALETTE						 3

#define __BGMAP_PALETTE_0						 0xE4  // 11100100
#define __BGMAP_PALETTE_1						 0xE0  // 11100000
#define __BGMAP_PALETTE_2						 0xD0  // 11010000
#define __BGMAP_PALETTE_3						 0x90  // 10010000

#define __OBJECT_PALETTE_0						 0xE4  // 11100100
#define __OBJECT_PALETTE_1						 0xE0  // 11100000
#define __OBJECT_PALETTE_2						 0xD0  // 11010000
#define __OBJECT_PALETTE_3						 0x90  // 10010000

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// RANDOM NUMBER GENERATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#undef __ADD_USER_INPUT_AND_TIME_TO_RANDOM_SEED

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// EXCEPTIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __EXCEPTION_COLUMN 0
#define __EXCEPTION_LINE   0

#endif
