/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
	s16		parallax;
	fix13_3	pd_y;		// *y+Dy /= 8.0
	fix7_9	pa;			// /=512.0
	fix7_9	pc;			// /=512.0
	u16 	spacer[3];		//unknown
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
	s16		parallax;
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
	u32 		param;
	s16			parallax;
} AffineInfo;

/**
 * An Hbias Entry
 *
 * @memberof ParamTableManager
 */
typedef struct HbiasEntry
{
	s16 offsetLeft;
	s16 offsetRight;
} HbiasEntry;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-bgmap
singleton class ParamTableManager : Object
{
	// total size of param table
	u32 size;
	// number of used bytes
	u32 used;
	// allocated bgmapSprites
	VirtualList bgmapSprites;
	// removed bgmapSprites' sizes
	VirtualList removedBgmapSpritesSizes;
	// used for defragmentation
	ParamTableFreeData paramTableFreeData;
	// used for defragmentation
	BgmapSprite previouslyMovedBgmapSprite;
	// used for defragmentation
	u32 paramTableBase;

	/// @publicsection
	static ParamTableManager getInstance();
	u32 allocate(BgmapSprite bsprite);
	void calculateParamTableBase(int availableBgmapSegmentsForParamTable);
	bool defragmentProgressively();
	void free(BgmapSprite bsprite);
	u32 getParamTableBase();
	void print(int x, int y);
	void reset();
}


#endif
