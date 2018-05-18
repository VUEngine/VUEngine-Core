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

#define __PARAM_TABLE_PADDING	1


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
 * @memberof Hbias
 */
typedef struct HbiasEntry
{
	s16 offsetLeft;
	s16 offsetRight;
} HbiasEntry;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class ParamTableManager : Object
{
	/**
	 * @var int 				size
	 * @brief 					total size of param table
	 * @memberof				ParamTableManager
	 */
	u32 size;
	/**
	 * @var u32 				used
	 * @brief 					number of used bytes
	 * @memberof				ParamTableManager
	 */
	u32 used;
	/**
	 * @var VirtualList 		bgmapSprites
	 * @brief 					allocated bgmapSprites
	 * @memberof				ParamTableManager
	 */
	VirtualList bgmapSprites;
	/**
	 * @var VirtualList 		removedBgmapSpritesSizes
	 * @brief 					removed bgmapSprites' sizes
	 * @memberof				ParamTableManager
	 */
	VirtualList removedBgmapSpritesSizes;
	/**
	 * @var ParamTableFreeData 	paramTableFreeData
	 * @brief 					used for defragmentation
	 * @memberof				ParamTableManager
	 */
	ParamTableFreeData paramTableFreeData;
	/**
	 * @var BgmapSprite 		previouslyMovedBgmapSprite
	 * @brief 					used for defragmentation
	 * @memberof				ParamTableManager
	 */
	BgmapSprite previouslyMovedBgmapSprite;
	/**
	 * @var u32 				paramTableBase
	 * @brief 					used for defragmentation
	 * @memberof				ParamTableManager
	 */
	u32 paramTableBase;

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
