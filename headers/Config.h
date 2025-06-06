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
// DEBUGGING / PROFILING
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __STACK_HEADROOM 1000

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

#undef __LEGACY_COORDINATE_PROJECTION

#define __SCREEN_WIDTH				   384
#define __SCREEN_HEIGHT				   224
#define __SCREEN_DEPTH				   2048

#define __MAXIMUM_X_VIEW_DISTANCE	   2048
#define __MAXIMUM_Y_VIEW_DISTANCE	   4096

#define __CAMERA_NEAR_PLANE			   0

#define __BASE_FACTOR				   32

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ANIMATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH 16
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION	 16

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// MEMORY POOL
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#undef __MEMORY_POOLS
#define __MEMORY_POOLS 11

#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS \
	__BLOCK_DEFINITION(164, 1) \
	__BLOCK_DEFINITION(152, 10) \
	__BLOCK_DEFINITION(140, 10) \
	__BLOCK_DEFINITION(116, 40) \
	__BLOCK_DEFINITION(108, 40) \
	__BLOCK_DEFINITION(80, 50) \
	__BLOCK_DEFINITION(68, 60) \
	__BLOCK_DEFINITION(40, 30) \
	__BLOCK_DEFINITION(28, 350) \
	__BLOCK_DEFINITION(20, 700) \
	__BLOCK_DEFINITION(16, 450) \

#undef __SET_MEMORY_POOL_ARRAYS
#define __SET_MEMORY_POOL_ARRAYS \
	__SET_MEMORY_POOL_ARRAY(164) \
	__SET_MEMORY_POOL_ARRAY(152) \
	__SET_MEMORY_POOL_ARRAY(140) \
	__SET_MEMORY_POOL_ARRAY(116) \
	__SET_MEMORY_POOL_ARRAY(108) \
	__SET_MEMORY_POOL_ARRAY(80) \
	__SET_MEMORY_POOL_ARRAY(68) \
	__SET_MEMORY_POOL_ARRAY(40) \
	__SET_MEMORY_POOL_ARRAY(28) \
	__SET_MEMORY_POOL_ARRAY(20) \
	__SET_MEMORY_POOL_ARRAY(16) \

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
#define __TOTAL_OBJECTS					1024
#define __SPRITE_ROTATE_IN_3D
#define __HACK_BGMAP_SPRITE_HEIGHT

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// TEXTURE MANAGEMENT
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TOTAL_NUMBER_OF_BGMAPS_SEGMENTS 10
#define __PARAM_TABLE_SEGMENTS			  1
#define __NUM_BGMAPS_PER_SEGMENT		  14
#define __MAX_NUMBER_OF_BGMAPS_SEGMENTS	  (__TOTAL_NUMBER_OF_BGMAPS_SEGMENTS - __PARAM_TABLE_SEGMENTS)

#define __PRINTING_BGMAP_X_OFFSET		  0
#define __PRINTING_BGMAP_Y_OFFSET		  (64 * 8 - __SCREEN_HEIGHT)
#define __PRINTING_BGMAP_PARALLAX_OFFSET  0

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// PARAM TABLE
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAXIMUM_SCALE					  2
#define __MAXIMUM_AFFINE_ROWS_PER_CALL	  16

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// PHYSICS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __GRAVITY						  10.0f
#define __MAXIMUM_FRICTION_COEFFICIENT	  __I_TO_FIXED(1)
#define __PHYSICS_TIME_ELAPSED_DIVISOR	  2
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

#define __EAR_DISPLACEMENT						 384
#define __SOUND_STEREO_ATTENUATION_DISTANCE		 2048

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// BRIGHTNESS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __BRIGHTNESS_DARK_RED					 32
#define __BRIGHTNESS_MEDIUM_RED					 64
#define __BRIGHTNESS_BRIGHT_RED					 128

#define __FADE_DELAY							 16
#define __CAMERA_EFFECT_FADE_INCREMENT			 1

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// COLOR PALETTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __PRINTING_PALETTE						 0

#define __BGMAP_PALETTE_0						 0xE4  // 11100100
#define __BGMAP_PALETTE_1						 0xE0  // 11100000
#define __BGMAP_PALETTE_2						 0x90  // 10010000
#define __BGMAP_PALETTE_3						 0x50  // 01010000

#define __OBJECT_PALETTE_0						 0xE4  // 11100100
#define __OBJECT_PALETTE_1						 0xE0  // 11100000
#define __OBJECT_PALETTE_2						 0x90  // 10010000
#define __OBJECT_PALETTE_3						 0x50  // 01010000

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
