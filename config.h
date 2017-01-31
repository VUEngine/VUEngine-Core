/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

// print frame rate
#define __PRINT_FRAMERATE

// print memory pool's status
#define __PRINT_MEMORY_POOL_STATUS
#define __PRINT_DETAILED_MEMORY_POOL_STATUS

// alert stack overflows
#define __ALERT_STACK_OVERFLOW

// tools
#define __DEBUG_TOOLS
#define __STAGE_EDITOR
#define __ANIMATION_EDITOR

#endif


//---------------------------------------------------------------------------------------------------------
// 											PROFILING
//---------------------------------------------------------------------------------------------------------

// print frame rate
#undef __PRINT_FRAMERATE

// show game's process profiling
#undef __PROFILE_GAME

// show detailed profiling of each of the game's main processes
// it is more useful when __TIMER_RESOLUTION approaches 1
#undef __PROFILE_GAME_DETAILED

// to make it easier to read the profiling output
#undef __DIMM_FOR_PROFILING

// show streaming's process profiling
#undef __PROFILE_STREAMING

// print the game's current process while the VIP's frame start
// and idle interrupts are fired, but the game frame is still pending
// processes to complete
#undef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT

// alert VIP's overtime
#define __ALERT_VIP_OVERTIME

// alert transformation - VIP unsync warning
#define __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VIP


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
#define __HORIZONTAL_VIEW_POINT_CENTER			__SCREEN_WIDTH / 2

// player's eyes' vertical position
#define __VERTICAL_VIEW_POINT_CENTER			__SCREEN_HEIGHT / 2

// parallax values are divide by this factor to control their strength
#define __PARALLAX_CORRECTION_FACTOR			16


//---------------------------------------------------------------------------------------------------------
// 											FRAME RATE CONTROL
//---------------------------------------------------------------------------------------------------------

// disable VIP's __XPEND interrupt, and thus rendering while transformation operations have not finished
#undef __FORCE_VIP_SYNC

// timer resolution
#define __TIMER_RESOLUTION						1

// options are __TIMER_20US and __TIMER_100US
#define __TIMER_FREQUENCY                       __TIMER_20US

// target frames per second
// __FRAME_CYCLE = 0 means __TARGET_FPS = 50
// __FRAME_CYCLE = 1 means __TARGET_FPS = 25
#define	__FRAME_CYCLE							0

#define __TARGET_FPS 							(50 >> __FRAME_CYCLE)

#define __GAME_FRAME_DURATION                   __MILLISECONDS_IN_SECOND / __TARGET_FPS

// target frames per second
#define __OPTIMUM_FPS 							(__TARGET_FPS >> __FRAME_CYCLE)

// define to dispatch the delayed messages every other game frame cycle
#define __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE


//---------------------------------------------------------------------------------------------------------
// 												ANIMATION
//---------------------------------------------------------------------------------------------------------

// max length of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH	16

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
#define __MEMORY_POOLS							16

#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(200, 1)																			\
	__BLOCK_DEFINITION(168, 8)																			\
	__BLOCK_DEFINITION(148, 8)																			\
	__BLOCK_DEFINITION(144, 25)																			\
	__BLOCK_DEFINITION(136, 10)																			\
	__BLOCK_DEFINITION(128, 45)																			\
	__BLOCK_DEFINITION(112, 30)																			\
	__BLOCK_DEFINITION(100, 28)																			\
	__BLOCK_DEFINITION(88, 80)																			\
	__BLOCK_DEFINITION(76, 10)																			\
	__BLOCK_DEFINITION(68, 58)																			\
	__BLOCK_DEFINITION(32, 10)																			\
	__BLOCK_DEFINITION(28, 200)																			\
	__BLOCK_DEFINITION(24, 100)																			\
	__BLOCK_DEFINITION(20, 670)																			\
	__BLOCK_DEFINITION(16, 460)						    												\


#undef __SET_MEMORY_POOL_ARRAYS
#define __SET_MEMORY_POOL_ARRAYS																		\
	__SET_MEMORY_POOL_ARRAY(200)																		\
	__SET_MEMORY_POOL_ARRAY(168)																		\
	__SET_MEMORY_POOL_ARRAY(148)																		\
	__SET_MEMORY_POOL_ARRAY(144)																		\
	__SET_MEMORY_POOL_ARRAY(136)																		\
	__SET_MEMORY_POOL_ARRAY(128)																		\
	__SET_MEMORY_POOL_ARRAY(112)																		\
	__SET_MEMORY_POOL_ARRAY(100)																		\
	__SET_MEMORY_POOL_ARRAY(88)																			\
	__SET_MEMORY_POOL_ARRAY(76)																			\
	__SET_MEMORY_POOL_ARRAY(68)																			\
	__SET_MEMORY_POOL_ARRAY(32)																			\
	__SET_MEMORY_POOL_ARRAY(28)																			\
	__SET_MEMORY_POOL_ARRAY(24)																			\
	__SET_MEMORY_POOL_ARRAY(20)																			\
	__SET_MEMORY_POOL_ARRAY(16)                                                                         \


// percentage (0-100) above which the memory pool's status shows the pool usage
#define __MEMORY_POOL_WARNING_THRESHOLD			85


//---------------------------------------------------------------------------------------------------------
// 												SRAM
//---------------------------------------------------------------------------------------------------------

// the amount of available sram space, in bytes
// the vb allows up to 16 mb, but all known carts support only 8 kb of sram
#define __TOTAL_SAVE_RAM 						8192


//---------------------------------------------------------------------------------------------------------
// 											CHAR MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// total number of available chars in char memory
#define __CHAR_MEMORY_TOTAL_CHARS 				2048


//---------------------------------------------------------------------------------------------------------
// 											SPRITE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// total number of layers (basically the number of worlds)
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
// 											BRIGHTNESS
//---------------------------------------------------------------------------------------------------------

// default brightness settings, actual values are set in stage definitions
// for a nice progression, each shade should be about twice as big as the previous one
// _BRIGHT_RED must be larger than _DARK_RED + _MEDIUM_RED
#define __BRIGHTNESS_DARK_RED					32
#define __BRIGHTNESS_MEDIUM_RED					64
#define __BRIGHTNESS_BRIGHT_RED					128

// default total duration for blocking fade in/out effects
#define __FADE_DURATION							320

// default delay between steps in asynchronous fade effect
#define __FADE_DELAY					        16


//---------------------------------------------------------------------------------------------------------
// 											COLOR PALETTES
//---------------------------------------------------------------------------------------------------------

#define __PRINTING_PALETTE						0

// default palette values, actual values are set in stage definitions

#define __BGMAP_PALETTE_0						0b11100100 // normal progression
#define __BGMAP_PALETTE_1						0b11100000 // show dark red as black
#define __BGMAP_PALETTE_2						0b10010000 // background layer
#define __BGMAP_PALETTE_3						0b01010000 // very dark, used when getting hit

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
