/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PARAM_TABLE_MANAGER_H_
#define PARAM_TABLE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// Add at least 8 pixels of headroom
#define __PARAM_TABLE_PADDING	8


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

/**
 * An Affine Entry
 *
 * @memberof Affine
 */
typedef struct AffineEntry
{
	fix13_3	pb_y;		// *y+Dx /= 8.0
	int16		parallax;
	fix13_3	pd_y;		// *y+Dy /= 8.0
	fix7_9	pa;			// /=512.0
	fix7_9	pc;			// /=512.0
	uint16 	spacer[3];		//unknown
} AffineEntry;

/**
 * A Fixed Affine Matrix
 *
 * @memberof Affine
 */
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

/**
 * Affine Info
 *
 * @memberof Affine
 */
typedef struct AffineInfo
{
	fix10_6 	x;
	fix10_6 	y;
	fix13_3 	mx;
	fix13_3 	my;
	fix13_3 	halfWidth;
	fix13_3 	halfHeight;
	Rotation* 	rotation;
	Scale* 		scale;
	uint32 		param;
	int16			parallax;
} AffineInfo;

/**
 * An Hbias Entry
 *
 * @memberof ParamTableManager
 */
typedef struct HbiasEntry
{
	int16 offsetLeft;
	int16 offsetRight;
} HbiasEntry;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-bgmap
singleton class ParamTableManager : Object
{
	// total size of param table
	uint32 size;
	// number of used bytes
	uint32 used;
	// allocated bgmapSprites
	VirtualList bgmapSprites;
	// used for defragmentation
	ParamTableFreeData paramTableFreeData;
	// used for defragmentation
	BgmapSprite previouslyMovedBgmapSprite;
	// used for defragmentation
	uint32 paramTableBase;

	/// @publicsection
	static ParamTableManager getInstance();
	uint32 allocate(BgmapSprite bsprite);
	void calculateParamTableBase(int32 availableBgmapSegmentsForParamTable);
	bool defragmentProgressively();
	void free(BgmapSprite bsprite);
	uint32 getParamTableBase();
	void print(int32 x, int32 y);
	void reset();
}


#endif
