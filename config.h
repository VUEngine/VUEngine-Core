/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Oop.h>


//---------------------------------------------------------------------------------------------------------
// 											DEBUGGING TOOLS
//---------------------------------------------------------------------------------------------------------

#ifdef __TOOLS
#define __PRINT_FRAMERATE
#define __PRINT_MEMORY_POOL_STATUS
#define __ALERT_STACK_OVERFLOW
#define __ALERT_VPU_OVERTIME
#define __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VPU
#define __DEBUG_TOOLS
#define __STAGE_EDITOR
#define __ANIMATION_EDITOR
#endif


//---------------------------------------------------------------------------------------------------------
// 											OPTICS / PROJECTION
//---------------------------------------------------------------------------------------------------------

// screen width in pixels
#define __SCREEN_WIDTH							384

// screen height in pixels
#define __SCREEN_HEIGHT							224

// screen depth in pixels
#define __SCREEN_DEPTH							384

// distance from player's eyes to the virtual screen
#define __DISTANCE_EYE_SCREEN					384

// maximum view distance (depth) (power of two)
#define __MAXIMUM_VIEW_DISTANCE_POWER			9

// distance between eyes
#define __BASE_FACTOR							768

// player's eyes' horizontal position
#define __HORIZONTAL_VIEW_POINT_CENTER			192

// player's eyes' vertical position
#define __VERTICAL_VIEW_POINT_CENTER			256

// parallax values are divide by this factor to control their strength
#define __PARALLAX_CORRECTION_FACTOR			16


//---------------------------------------------------------------------------------------------------------
// 											FRAME RATE CONTROL
//---------------------------------------------------------------------------------------------------------

// disable VPU's XPEND interrupt, and thus rendering while transformation operations have not finished
#undef __FORCE_VPU_SYNC

// clock resolution
#define __TIMER_RESOLUTION						10

// target frames per second
// __FRAME_CYCLE = 0 means __TARGET_FPS = 50
// __FRAME_CYCLE = 1 means __TARGET_FPS = 25
#define	__FRAME_CYCLE							0

#define __TARGET_FPS 							(50 >> __FRAME_CYCLE)

// target frames per second
#define __OPTIMUM_FPS 							(__TARGET_FPS >> __FRAME_CYCLE)

// target frames per second
#define __MINIMUM_GOOD_FPS 						(__TARGET_FPS - 2)



//---------------------------------------------------------------------------------------------------------
// 												ANIMATION
//---------------------------------------------------------------------------------------------------------

// max length of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH	20

// max number of frames per animation function
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION		16

// max number of animation functions per description
#define __MAX_ANIMATION_FUNCTIONS				32


//---------------------------------------------------------------------------------------------------------
// 												MEMORY POOL
//---------------------------------------------------------------------------------------------------------

// reset to 0 each byte of each free block on resetting game
// only use for debugging, proper object's initialization must make this macro unnecessary
#undef __MEMORY_POOL_CLEAN_UP

#undef __MEMORY_POOLS
#define __MEMORY_POOLS							15

#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS																			\
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
	__BLOCK_DEFINITION(68, 56)																			\
	__BLOCK_DEFINITION(28, 276)																			\
	__BLOCK_DEFINITION(20, 588)																			\
	__BLOCK_DEFINITION(16, 276)																		\

#undef __SET_MEMORY_POOL_ARRAYS
#define __SET_MEMORY_POOL_ARRAYS																		\
	__SET_MEMORY_POOL_ARRAY(192)																		\
	__SET_MEMORY_POOL_ARRAY(160)																		\
	__SET_MEMORY_POOL_ARRAY(144)																		\
	__SET_MEMORY_POOL_ARRAY(136)																		\
	__SET_MEMORY_POOL_ARRAY(120)																		\
	__SET_MEMORY_POOL_ARRAY(112)																		\
	__SET_MEMORY_POOL_ARRAY(104)																		\
	__SET_MEMORY_POOL_ARRAY(100)																		\
	__SET_MEMORY_POOL_ARRAY(92)																			\
	__SET_MEMORY_POOL_ARRAY(84)																			\
	__SET_MEMORY_POOL_ARRAY(76)																			\
	__SET_MEMORY_POOL_ARRAY(68)																			\
	__SET_MEMORY_POOL_ARRAY(28)																			\
	__SET_MEMORY_POOL_ARRAY(20)																			\
	__SET_MEMORY_POOL_ARRAY(16)																			\

// percentage (0-100) above which the memory pool's status shows the pool usage
#define __MEMORY_POOL_WARNING_THRESHOLD			85


//---------------------------------------------------------------------------------------------------------
// 											CHAR MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// number of char segments
// the fourth segment is used for text allocation, changing this value to 4 may cause text corruption
#define __CHAR_SEGMENTS							3

// number of chars per char segment
#define __CHAR_SEGMENT_TOTAL_CHARS 				512


//---------------------------------------------------------------------------------------------------------
// 											SPRITE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// total number of layers (basically the number of Worlds)
#define __TOTAL_LAYERS							32


//---------------------------------------------------------------------------------------------------------
// 											TEXTURE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// total number of bgmap segments
#define __TOTAL_NUMBER_OF_BGMAPS_SEGMENTS 		14

// bgmaps to use (leave 2 bgmaps to allocate param table, 1 for printing)
#define __MAX_NUMBER_OF_BGMAPS_SEGMENTS 		(__TOTAL_NUMBER_OF_BGMAPS_SEGMENTS - 3)

// number of bgmap definitions in each bgmap segment
#define __NUM_BGMAPS_PER_SEGMENT 				16

// printing area
#define __PRINTING_BGMAP_X_OFFSET				0
#define __PRINTING_BGMAP_Y_OFFSET				0
#define __PRINTING_BGMAP_Z_OFFSET				0
#define __PRINTABLE_BGMAP_AREA 					(64 * 28)

#define __PALETTE_MASK							0x0600
#define __WORLD_LAYER_MASK						0x01F0
#define __SEGMENT_MASK							0x000F

#define __PALETTE_MASK_DISP						0x09 /* 6 */
#define __WORLD_LAYER_MASK_DISP					0x04 /* 1 */
#define __SEGMENT_MASK_DISP						0x00 /* 0 */


//---------------------------------------------------------------------------------------------------------
// 												PARAM TABLE
//---------------------------------------------------------------------------------------------------------

// maximum possible scale: affects param table allocation space
#define __MAXIMUM_SCALE							2

// maximum number of rows to write on each call to affine calculation functions
#define __MAXIMUM_AFFINE_ROWS_PER_CALL			16


//---------------------------------------------------------------------------------------------------------
// 											    STREAMING
//---------------------------------------------------------------------------------------------------------

// the number of total calls to the streaming method which completes a cycle
// there are 4 parts for the streaming algorithm:
// 1) unload entities
// 2) select the next entity to load
// 3) create the selected entity
// 4) initialize the loaded entity
#define __STREAM_CYCLE_DURATION					24

// pad to determine if an entity must be loaded/unloaded
// load pad must always be lower than unload pad!
// too close values will put the streaming under heavy usage!
#define __ENTITY_LOAD_PAD 						256
#define __ENTITY_UNLOAD_PAD 					(__ENTITY_LOAD_PAD + 56)


//---------------------------------------------------------------------------------------------------------
// 												PHYSICS
//---------------------------------------------------------------------------------------------------------

#define __GRAVITY								13000

// number of bodies to check for gravity on each cycle
#define __BODIES_TO_CHECK_FOR_GRAVITY		    10

#define __MAX_SHAPES_PER_LEVEL					32
#define __MAX_BODIES_PER_LEVEL					32

// used to make an approximation of Lorentz' contraction
// to handle collisions on very fast moving shapes
#define __LIGHT_SPEED		                    ITOFIX19_13(50000)


//---------------------------------------------------------------------------------------------------------
// 												SOUND
//---------------------------------------------------------------------------------------------------------

// channels per bgms
#define __BGM_CHANNELS							2

// channels per fx
#define __FX_CHANNELS							1

// simultaneous bgms
#define __BGMS									1

// simultaneous fx
#define __FXS									2

#define __TOTAL_SOUNDS							(__BGMS + __FXS)
#define __LEFT_EAR_CENTER						96
#define __RIGHT_EAR_CENTER						288


//---------------------------------------------------------------------------------------------------------
// 											COLOR PALETTES
//---------------------------------------------------------------------------------------------------------

#define __PRINTING_PALETTE						0

// default palette values, actual values are set in stage definitions

#define __BGMAP_PALETTE_0						0xE4 // normal progression
#define __BGMAP_PALETTE_1						0xE0 // show dark red as black
#define __BGMAP_PALETTE_2						0x90 // background layer
#define __BGMAP_PALETTE_3						0x50 // very dark, used when getting hit

#define __OBJECT_PALETTE_0						__BGMAP_PALETTE_0
#define __OBJECT_PALETTE_1						__BGMAP_PALETTE_1
#define __OBJECT_PALETTE_2						__BGMAP_PALETTE_2
#define __OBJECT_PALETTE_3						__BGMAP_PALETTE_3


//---------------------------------------------------------------------------------------------------------
// 											LOW BATTERY INDICATOR
//---------------------------------------------------------------------------------------------------------

// when this is defined, the engine's default low battery indicator is used
#define __LOW_BATTERY_INDICATOR

// position of low battery indicator
#define __LOW_BATTERY_INDICATOR_POS_X			45
#define __LOW_BATTERY_INDICATOR_POS_Y			1

// delay between showing/not showing the low battery indicator (in milliseconds)
#define __LOW_BATTERY_INDICATOR_BLINK_DELAY		500

// wait this long after first receiving the PWR signal before showing the low battery indicator
// (in milliseconds)
#define __LOW_BATTERY_INDICATOR_INITIAL_DELAY	2000


//---------------------------------------------------------------------------------------------------------
// 											AUTOMATIC PAUSE
//---------------------------------------------------------------------------------------------------------

// amount of time after which to show auto pause (in milliseconds)
#define __AUTO_PAUSE_DELAY						(30 * 60 * 1000)

// the automatic pause state is not pushed until there is only one state in the game's stack.
// the following defines the time between checks whether the condition is met (in milliseconds)
#define __AUTO_PAUSE_RECHECK_DELAY				(60 * 1000)


//---------------------------------------------------------------------------------------------------------
// 												FONTS
//---------------------------------------------------------------------------------------------------------

// when this is defined, custom fonts are loaded instead of the default one
#define __CUSTOM_FONTS


//---------------------------------------------------------------------------------------------------------
// 										RANDOM NUMBER GENERATION
//---------------------------------------------------------------------------------------------------------

// how many times the randomSeed function cycles generate a random seed
#define __RANDOM_SEED_CYCLES					2


//---------------------------------------------------------------------------------------------------------
// 												EXCEPTIONS
//---------------------------------------------------------------------------------------------------------

#define __EXCEPTION_COLUMN						0
#define __EXCEPTION_LINE						0


#endif
