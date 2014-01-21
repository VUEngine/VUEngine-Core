#ifndef CONFIG_H_
#define CONFIG_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										DEBUGGING
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//define some debug levels
#define __DEBUG_0
#undef __DEBUG_1
#undef __DEBUG_2

		
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 									OPTICS / PROJECTION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */
// screen width in pixels
#define __SCREENWIDTH		384

// screen height in pixels
#define __SCREENHEIGHT		224

// z position of the screen
#define __ZZERO				0

// game world limit to unload entities
#define __Z_GAME_LIMIT		__ZZERO - 30

// lower posible z coordinate
#define __ZLOWLIMIT 		-200

// distance from player's eyes to the virtual screen
#define __DISTANCEEYESCREEN		384

// maximun view distance (deep)
#define __MAXVIEWDISTANCE		512
// always use a power of 2 as the maximun view distance, and update
// the number of bits to make projection faster
#define __MAXVIEWDISTANCE_POW	9

//distance between eyes
#define __BASEFACTOR			768

// player's eyes's horizontal position
#define __HVPC					192

// player's eyes's vertical position
// #define __VVPC					112
#define __VVPC					(56)

// zoom factor to distortoine zooming
#define __ZOOMFACTOR			0.2f

// floor's y position
#define __FLOOR 				224-8*3

// parallax values are divide by this factor to control it's strenght
#define __PARALLAX_CORRECTION_FACTOR	20

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										FRAME RATE CONTROL
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// determine whether frame rate is capped or not
#define __CAP_FPS	1

// clock resolution
#define __TIMER_RESOLUTION		1

//target frames per second
#define __RENDER_FPS 	30

//target frames per second
#define __PHYSICS_FPS 	30

//target frames per second
#define __LOGIC_FPS 	30

// set animation delays as if they are 60 FPS,
// and multiply by this factor
#define __FPS_ANIM_FACTOR 	(__RENDER_FPS / 60.0f)

//you will hardly have more than ten mapcharacters in affine mode  
//on screen at once
#define	__TOTALPARAMOBJECTS 32

//seconds that must elapse to call rest state... in seconds (15 minutes)
#define __RESTDELAY 		900



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										MEMORY POOL
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __MEMORY_POOLS	9

//each block size
#define __BLOCK_512B 		512
#define __BLOCK_256B 		256
#define __BLOCK_192B 		192
#define __BLOCK_128B 		128
#define __BLOCK_96B 		96
#define __BLOCK_64B 		64
#define __BLOCK_48B 		48
#define __BLOCK_32B 		32
#define __BLOCK_24B 		24		// Virtual nodes are 20 bytes long so a 32b block is too much
#define __BLOCK_16B 		16

#define __POOL_512B_SIZE 	(__BLOCK_512B * 0)
#define __POOL_256B_SIZE 	(__BLOCK_256B * 4)
#define __POOL_192B_SIZE 	(__BLOCK_192B * 4)
#define __POOL_128B_SIZE 	(__BLOCK_128B * 8)
#define __POOL_96B_SIZE 	(__BLOCK_96B * 16)
#define __POOL_64B_SIZE 	(__BLOCK_64B * 64)
#define __POOL_48B_SIZE 	(__BLOCK_48B * 128)
#define __POOL_32B_SIZE 	(__BLOCK_32B * 128)
#define __POOL_24B_SIZE 	(__BLOCK_24B * 128)
#define __POOL_16B_SIZE 	(__BLOCK_16B * 128)

#define __MIN_BLOCK 	__BLOCK_16B

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										CHAR MANAGEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define CHAR_SEGMENTS	4

// number of chargroups per char segment
#define CHAR_GRP_PER_SEG	32

// number of chargroups per char segment
#define CHAR_SEGMENT_SIZE 16


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										SPRITE MANAGEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// basically the number of WORLDS
#define __TOTAL_LAYERS	31


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										TEXTURE MANAGEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//bgmaps to use (leave 2 bgmaps to allocate param table)
#define NUM_BGMAPS 12

//number of bgmap definitions in each bgmap segment 
#define NUM_MAPS_PER_SEG 16

#define PALLET_MASK			0x0600
#define WORLD_LAYER_MASK	0x01F0
#define SEGMENT_MASK		0x000F


#define PALLET_MASK_DISP		0x09 /* 6 */
#define WORLD_LAYER_MASK_DISP	0x04 /* 1 */
#define SEGMENT_MASK_DISP		0x00 /* 0 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PARAM TABLE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//param table for affine and hbias render
#define __PARAMEND 0x0003D7FF

/* param table initial address
 * To increase param table's size, decrease PARAMBase
 * in libgccvb/video.h
 */

#define __PARAMINI (PARAMBase - 4084)

/* Number of the power of 2 to multiply by the number
 * of rows of a given bgmap to allocate space
 */
#define __PARAMSPACEFACTOR	1


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										WORLD'S CAPACITY
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//maximun number of entities per world
#define __ENTITIESPERWORLD 	128

//padd to determine if a character must be loaded
#define __ENTITYLOADPAD		20

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PHYSICS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define it to make the collision system inform your entities if they can fall
#define __GRAVITY_WORLD

//physical friction
#define __NOFRICTION 		0.0f
#define __FLOORFRICTION 	10.0f
#define __AIRFRICTION 		50.0f
#define __ICEFRICTION 		0.03f
#define __WATERFRICTION 	1.5f

#define __GRAVITY			980
//#define __GRAVITY			9.8f

#define __MAX_SHAPES_PER_LEVEL		32
#define __MAX_BODIES_PER_LEVEL		32

// channels per bgms
#define __BGM_CHANNELS	2

// channels per fx
#define __FX_CHANNELS	1

// simultaneous bgms
#define __BGMS			1

// simultaneous fx
#define __FXS			2


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										SOUND MANAGEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __TOTALSOUNDS	(__BGMS + __FXS)
#define __LEFT_EAR_CENTER		96
#define __RIGHT_EAR_CENTER	288


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										STAGE MANAGEMENT
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __STAGE_BITS_PER_ENTITY		2
#define __ENTITIES_IN_STAGE (__ENTITIESPERWORLD / (sizeof(WORD) << 3) * __STAGE_BITS_PER_ENTITY)

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										COLOR PALETS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __GPLT0VALUE  0xE4	/* Set all eight palettes to: 11100100 */
#define __GPLT1VALUE  0xE0	/* (i.e. "Normal" dark to light progression.) */
#define __GPLT2VALUE  0xF0
#define __GPLT3VALUE  0xEA
#define __JPLT0VALUE  0xE4
#define __JPLT1VALUE  0xE4
#define __JPLT2VALUE  0xE4
#define __JPLT3VALUE  0xE4

#define	__BKCOL		0x00

#define __BRTA		0x00
#define __BRTB		0x00
#define __BRTC		0x00



#endif /*CONFIG_H_*/
