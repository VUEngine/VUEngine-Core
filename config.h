/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//											DEBUGGING/ PROFILING
//---------------------------------------------------------------------------------------------------------

// Print memory pool's status
#undef __PRINT_MEMORY_POOL_STATUS
#undef __PRINT_DETAILED_MEMORY_POOL_STATUS

// Print frame rate
#undef __PRINT_FRAMERATE

// Alert stack overflows
#undef __ALERT_STACK_OVERFLOW

/* Enable detailed profiling of each of the game's main processes
 * it is more useful when __TIMER_RESOLUTION approaches 1
 */
#undef __PROFILE_GAME

// Enable streaming's profiling
#undef __PROFILE_STREAMING

// Show games's profiling during game
#undef __SHOW_GAME_PROFILING

// Show streaming's profiling during game
#undef __SHOW_STREAMING_PROFILING

// To make it easier to read the profiling output
#undef __DIMM_FOR_PROFILING

/* Print the game's current process while the VIP's frame start
 * and idle interrupts are fired, but the game frame is still pending
 * processes to complete
 */
#undef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT

// Alert VIP's overtime
#define __ALERT_VIP_OVERTIME


//---------------------------------------------------------------------------------------------------------
//											DEBUGGING TOOLS
//---------------------------------------------------------------------------------------------------------

#ifdef __TOOLS

// Print frame rate
#define __PRINT_FRAMERATE

/* Enable detailed profiling of each of the game's main processes
 * it is more useful when __TIMER_RESOLUTION approaches 1
 */
#define __PROFILE_GAME

// Enable streaming's profiling
#define __PROFILE_STREAMING

// Tools
#define __DEBUG_TOOLS
#define __STAGE_EDITOR
#define __ANIMATION_INSPECTOR

#endif

//---------------------------------------------------------------------------------------------------------
//											OPTICS / PROJECTION
//---------------------------------------------------------------------------------------------------------

// Screen width in pixels
#define __SCREEN_WIDTH							384

// Screen height in pixels
#define __SCREEN_HEIGHT							224

// Screen depth in pixels
#define __SCREEN_DEPTH							2048

// Distance from player's eyes to the virtual screen
#define __DISTANCE_EYE_SCREEN					384

// Maximum x view distance (depth) (power of two)
#define __MAXIMUM_X_VIEW_DISTANCE				2048

// Maximum y view distance (depth) (power of two)
#define __MAXIMUM_Y_VIEW_DISTANCE				4096

// Distance between eyes
#define __BASE_FACTOR							32

// Player's eyes' horizontal position
#define __HORIZONTAL_VIEW_POINT_CENTER			__SCREEN_WIDTH / 2

// Player's eyes' vertical position
#define __VERTICAL_VIEW_POINT_CENTER			__SCREEN_HEIGHT / 2

// Parallax values are divide by this factor to control their strength
#define __PARALLAX_CORRECTION_FACTOR			4

// Affects the strength of the scaling
#define __SCALING_MODIFIER_FACTOR				1.0f

// minimum number of pixels that the camera can move
#define __CAMERA_MINIMUM_DISPLACEMENT_PIXELS_POWER	1

//---------------------------------------------------------------------------------------------------------
//											FRAME RATE CONTROL
//---------------------------------------------------------------------------------------------------------

/* If defined, when the VIP's GAMESTART interrupt is fired before
 * the current game frame is done, the engine skips to the next
 * game frame.
 */
#undef __FORCE_VIP_SYNC

// Timer resolution
#define __TIMER_RESOLUTION						10

/* __FRAME_CYCLE = 0 means __TARGET_FPS = 50
 * __FRAME_CYCLE = 1 means __TARGET_FPS = 25
 */
#define	__FRAME_CYCLE							0

// Target frames per second
#define __TARGET_FPS 							(50 >> __FRAME_CYCLE)

// Milliseconds that must take to complete a game cycle
#define __GAME_FRAME_DURATION					(__MILLISECONDS_IN_SECOND / __TARGET_FPS)

// Target frames per second
#define __OPTIMUM_FPS 							(__TARGET_FPS >> __FRAME_CYCLE)

// Define to dispatch the delayed messages every other game frame cycle
#undef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE


//---------------------------------------------------------------------------------------------------------
//												ANIMATION
//---------------------------------------------------------------------------------------------------------

// Maximum length of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH	16

// Maximum number of frames per animation function
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION		16

// Maximum number of animation functions per description
#define __MAX_ANIMATION_FUNCTIONS				32


//---------------------------------------------------------------------------------------------------------
//												MEMORY POOL
//---------------------------------------------------------------------------------------------------------

/* Reset to 0 each byte of each free block on resetting game
 * only use for debugging, proper object's initialization must make this macro unnecessary
 */
#undef __MEMORY_POOL_CLEAN_UP

#undef __MEMORY_POOLS
#define __MEMORY_POOLS							11

#undef __MEMORY_POOL_ARRAYS
#define __MEMORY_POOL_ARRAYS																			\
	__BLOCK_DEFINITION(164, 1)																			\
	__BLOCK_DEFINITION(152, 10)																			\
	__BLOCK_DEFINITION(140, 10)																			\
	__BLOCK_DEFINITION(116, 40)																			\
	__BLOCK_DEFINITION(108, 40)																			\
	__BLOCK_DEFINITION(80, 50)																			\
	__BLOCK_DEFINITION(68, 60)																			\
	__BLOCK_DEFINITION(40, 30)																			\
	__BLOCK_DEFINITION(28, 350)																			\
	__BLOCK_DEFINITION(20, 700)																			\
	__BLOCK_DEFINITION(16, 450)																			\

#undef __SET_MEMORY_POOL_ARRAYS
#define __SET_MEMORY_POOL_ARRAYS																		\
	__SET_MEMORY_POOL_ARRAY(164)																		\
	__SET_MEMORY_POOL_ARRAY(152)																		\
	__SET_MEMORY_POOL_ARRAY(140)																		\
	__SET_MEMORY_POOL_ARRAY(116)																		\
	__SET_MEMORY_POOL_ARRAY(108)																		\
	__SET_MEMORY_POOL_ARRAY(80)																			\
	__SET_MEMORY_POOL_ARRAY(68)																			\
	__SET_MEMORY_POOL_ARRAY(40)																			\
	__SET_MEMORY_POOL_ARRAY(28)																			\
	__SET_MEMORY_POOL_ARRAY(20)																			\
	__SET_MEMORY_POOL_ARRAY(16)


// Percentage (0-100) above which the memory pool's status shows the pool usage
#define __MEMORY_POOL_WARNING_THRESHOLD			85


//---------------------------------------------------------------------------------------------------------
//												SRAM
//---------------------------------------------------------------------------------------------------------

/* Amount of available sram space, in bytes
 * the vb allows up to 16 MB, but all known
 * carts support only 8 KB of SRAM
 */
#define __TOTAL_SAVE_RAM 						8192


//---------------------------------------------------------------------------------------------------------
//											CHAR MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// Total number of available chars in char memory
#define __CHAR_MEMORY_TOTAL_CHARS 				2048


//---------------------------------------------------------------------------------------------------------
//											SPRITE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// Total number of layers (basically the number of worlds)
#define __TOTAL_LAYERS							32


//---------------------------------------------------------------------------------------------------------
//											TEXTURE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

// Total number of bgmap segments
#define __TOTAL_NUMBER_OF_BGMAPS_SEGMENTS 		14

// BGMAP segments to use (leave 2 to allocate param table, 1 for printing)
#define __MAX_NUMBER_OF_BGMAPS_SEGMENTS 		(__TOTAL_NUMBER_OF_BGMAPS_SEGMENTS - 3)

// Number of BGMAP definitions in each BGMAP segment
#define __NUM_BGMAPS_PER_SEGMENT 				16

// Printing area
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
//												PARAM TABLE
//---------------------------------------------------------------------------------------------------------

// Maximum possible scale: affects param table allocation space
#define __MAXIMUM_SCALE							2

// Maximum number of rows to write on each call to affine calculation functions
#define __MAXIMUM_AFFINE_ROWS_PER_CALL			16


//---------------------------------------------------------------------------------------------------------
//												STREAMING
//---------------------------------------------------------------------------------------------------------

/* Number of total calls to the streaming method which completes a cycle
 * there are 4 parts for the streaming algorithm:
 * 1) unload entities
 * 2) select the next entity to load
 * 3) create the selected entity
 * 4) initialize the loaded entity
 */
#define __STREAM_CYCLE_DURATION					24

/* Pad to determine if an entity must be loaded/unloaded
 * load pad must always be lower than unload pad!
 * too close values will put the streaming under heavy usage!
 */
#define __ENTITY_LOAD_PAD 						256
#define __ENTITY_UNLOAD_PAD 					(__ENTITY_LOAD_PAD + 56)


//---------------------------------------------------------------------------------------------------------
//												PHYSICS
//---------------------------------------------------------------------------------------------------------

#define __GRAVITY								(2 * 9.8f)

// Number of bodies to check for gravity on each cycle
#define __BODIES_TO_CHECK_FOR_GRAVITY			10

// divisor to speed up physics simulations
// bigger numbers equal faster computations
#define __PHYSICS_TIME_ELAPSED_DIVISOR			1

// maximum bounciness coefficient allowed
#define __MAXIMUM_BOUNCINESS_COEFFICIENT		1.2f


//---------------------------------------------------------------------------------------------------------
//												SOUND
//---------------------------------------------------------------------------------------------------------

// Channels per bgms
#define __BGM_CHANNELS							2

// Channels per fx
#define __FX_CHANNELS							1

// Simultaneous bgms
#define __BGMS									1

// Simultaneous fx
#define __FXS									2

#define __TOTAL_SOUNDS							(__BGMS + __FXS)
#define __LEFT_EAR_CENTER						96
#define __RIGHT_EAR_CENTER						288

// Affects the amount of attenuation caused by
// the distance between the x coordinate and
// each ear's position defined by __LEFT_EAR_CENTER
// and __RIGHT_EAR_CENTER
#define __SOUND_STEREO_ATTENUATION_FACTOR		__F_TO_FIX10_6(0.75f)


//---------------------------------------------------------------------------------------------------------
//											BRIGHTNESS
//---------------------------------------------------------------------------------------------------------

/* Default brightness settings, actual values are set in stage definitions
 * for a nice progression, each shade should be about twice as big as the previous one
 * _BRIGHT_RED must be larger than _DARK_RED + _MEDIUM_RED
 */
#define __BRIGHTNESS_DARK_RED					32
#define __BRIGHTNESS_MEDIUM_RED					64
#define __BRIGHTNESS_BRIGHT_RED					128

// Default delay between steps in fade effect
#define __FADE_DELAY							8


//---------------------------------------------------------------------------------------------------------
//											COLOR PALETTES
//---------------------------------------------------------------------------------------------------------

#define __PRINTING_PALETTE						0

// Default palette values, actual values are set in stage definitions
#define __BGMAP_PALETTE_0						0b11100100 // normal progression
#define __BGMAP_PALETTE_1						0b11100000 // show dark red as black
#define __BGMAP_PALETTE_2						0b10010000 // background layer
#define __BGMAP_PALETTE_3						0b01010000 // very dark, used when getting hit

#define __OBJECT_PALETTE_0						__BGMAP_PALETTE_0
#define __OBJECT_PALETTE_1						__BGMAP_PALETTE_1
#define __OBJECT_PALETTE_2						__BGMAP_PALETTE_2
#define __OBJECT_PALETTE_3						__BGMAP_PALETTE_3


//---------------------------------------------------------------------------------------------------------
//											LOW BATTERY INDICATOR
//---------------------------------------------------------------------------------------------------------

// When defined, the engine's default low battery indicator is used
#define __LOW_BATTERY_INDICATOR

// Position of low battery indicator
#define __LOW_BATTERY_INDICATOR_POS_X			45
#define __LOW_BATTERY_INDICATOR_POS_Y			1

// Delay between showing/not showing the low battery indicator (in milliseconds)
#define __LOW_BATTERY_INDICATOR_BLINK_DELAY		500

/* Wait this long after first receiving the PWR signal
 * before showing the low battery indicator (in milliseconds)
 */
#define __LOW_BATTERY_INDICATOR_INITIAL_DELAY	2000


//---------------------------------------------------------------------------------------------------------
//											AUTOMATIC PAUSE
//---------------------------------------------------------------------------------------------------------

// Amount of time after which to show auto pause (in milliseconds)
#define __AUTO_PAUSE_DELAY						(30 * 60 * 1000)

/* The automatic pause state is not pushed until
 * there is only one state in the game's stack.
 * the following defines the time between checks
 * whether the condition is met (in milliseconds)
 */
#define __AUTO_PAUSE_RECHECK_DELAY				(60 * 1000)


//---------------------------------------------------------------------------------------------------------
//										RANDOM NUMBER GENERATION
//---------------------------------------------------------------------------------------------------------

// How many times the randomSeed function cycles generate a random seed
#define __RANDOM_SEED_CYCLES					2


//---------------------------------------------------------------------------------------------------------
//												EXCEPTIONS
//---------------------------------------------------------------------------------------------------------

// Camera coordinates for the output of exceptions
#define __EXCEPTION_COLUMN						0
#define __EXCEPTION_LINE						0


#endif
