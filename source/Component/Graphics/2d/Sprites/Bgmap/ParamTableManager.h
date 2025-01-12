/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PARAM_TABLE_MANAGER_H_
#define PARAM_TABLE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Affine.h>
#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class BgmapSprite;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Add at least 8 pixels of headroom
#define __PARAM_TABLE_PADDING	8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An Affine entry in param table space
/// @memberof ParamTableManager
typedef struct AffineEntry
{
	fix13_3	pb_y;		// *y+Dx /= 8.0
	int16		parallax;
	fix13_3	pd_y;		// *y+Dy /= 8.0
	fix7_9	pa;			// /=512.0
	fix7_9	pc;			// /=512.0
	uint16 	spacer[3];		//unknown
} AffineEntry;

/// A Fixed Affine Matrix
/// @memberof ParamTableManager
typedef struct FixedAffineMatrix
{
	fix7_9 	pa;
	fix13_3 pb;
	fix7_9 	pc;
	fix13_3 pd;
	fix13_3 dx;
	fix13_3 dy;
	int16		parallax;
} FixedAffineMatrix;

/// Affine Info
/// @memberof ParamTableManager
typedef struct AffineInfo
{
	fixed_t 	x;
	fixed_t 	y;
	fix13_3 	mx;
	fix13_3 	my;
	fix13_3 	halfWidth;
	fix13_3 	halfHeight;
	Rotation* 	rotation;
	Scale* 		scale;
	uint32 		param;
	int16			parallax;
} AffineInfo;

/// An Hbias Entry
/// @memberof ParamTableManager
typedef struct HbiasEntry
{
	int16 offsetLeft;
	int16 offsetRight;
} HbiasEntry;

/// Param Table Free Data
/// @memberof ParamTableManager
typedef struct ParamTableFreeData
{
	uint32 param;
	uint32 recoveredSize;
} ParamTableFreeData;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ParamTableManager
///
/// Inherits from Object
///
/// Manages param table space allocating and releasing blocks in it.
singleton class ParamTableManager : Object
{
	/// @protectedsection

	/// Total size of param table
	uint32 size;

	/// Number of used bytes
	uint32 usedBytes;

	/// List of sprites with allocated param tables
	VirtualList bgmapSprites;
	
	/// Struct used to keep track of the next free block in param table space
	ParamTableFreeData paramTableFreeData;
	
	/// Cache of the last sprite whose param table was moved during the defragmentation
	/// of param table space
	BgmapSprite previouslyMovedBgmapSprite;

	// Displacement in bytes to keep track of the start address of param table space
	uint32 paramTableBase;

	/// @publicsection

	/// Reset the animation coordinator factory's state.
	static void reset();

	/// Retrieve the param table displacement in bytes used to keep track
	/// of the start address of param table space.
	/// @param availableBgmapSegmentsForParamTable: Number of available BGMAP segments for the param tables
	/// @return Displacement in bytes to keep track of the start address of param table space
	static uint32 configure(int32 availableBgmapSegmentsForParamTable);

	/// Allocate a param table for the provided sprite
	/// @param bgmapSprite: Sprite for which a param table will be allocated
	static uint32 allocate(BgmapSprite bgmapSprite);

	/// Free the param table allocated for the provided sprite.
	/// @param bgmapSprite: Sprite whose param table has to be freed
	static void free(BgmapSprite bgmapSprite);

	/// Defragment param table space.
	/// @param deferred: Flag to defragment param table memory over time
	static void defragment(bool deferred);

	/// Print the information about param table space usage.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);
}

#endif
