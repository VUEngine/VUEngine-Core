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

#ifndef BGMAP_TEXTURE_MANAGER_H_
#define BGMAP_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BGMAP_SEGMENT_SIZE	8192


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class BgmapTextureManager : Object
{
	/**
	 * @var u16 			numberOfChars
	 * @brief				Number of chars occupied
	 * @memberof			BgmapTextureManager
	 */
	u16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];
	/**
	 * @var s8 				xOffset
	 * @brief				Current x offset to set the next bgmap
	 * @memberof			BgmapTextureManager
	 */
	s8 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	/**
	 * @var s8 				yOffset
	 * @brief				Current y offset to set the next bgmap
	 * @memberof			BgmapTextureManager
	 */
	s8 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	/**
	 * @var s8 				offset
	 * @brief				12 segments, 28 maps, 2 indexes (x,y) and bgmap segment
	 * @memberof			BgmapTextureManager
	 */
	s8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][4];
	/**
	 * @var s16 			freeBgmapSegment
	 * @brief				Next free bgmap used for text printing
	 * @memberof			BgmapTextureManager
	 */
	s16 freeBgmapSegment;
	/**
	 * @var BgmapTexture 	bgmapTextures
	 * @brief				The textures allocated
	 * @memberof			BgmapTextureManager
	 */
	BgmapTexture bgmapTextures[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];
	/**
	 * @var s16 			availableBgmapSegmentsForTextures
	 * @brief				Number of available bgmap segments
	 * @memberof			BgmapTextureManager
	 */
	s16 availableBgmapSegmentsForTextures;
	/**
	 * @var s16 			printingBgmapSegment
	 * @brief				Segment for printing
	 * @memberof			BgmapTextureManager
	 */
	s16 printingBgmapSegment;

	static BgmapTextureManager getInstance();
	void allocateText(BgmapTextureManager this, BgmapTexture bgmapTexture);
	void calculateAvailableBgmapSegments(BgmapTextureManager this);
	s16 getAvailableBgmapSegmentsForTextures(BgmapTextureManager this);
	s16 getPrintingBgmapSegment(BgmapTextureManager this);
	BgmapTexture getTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);
	s16 getXOffset(BgmapTextureManager this, int id);
	s16 getYOffset(BgmapTextureManager this, int id);
	void print(BgmapTextureManager this, int x, int y);
	void releaseTexture(BgmapTextureManager this, BgmapTexture bgmapTexture);
	void reset(BgmapTextureManager this);
	void setSpareBgmapSegments(BgmapTextureManager this, u8 paramTableSegments);
}


#endif
