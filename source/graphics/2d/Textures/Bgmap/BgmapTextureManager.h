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
//												ENUMS
//---------------------------------------------------------------------------------------------------------

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kCols,
	kRows,
	kBgmapSegment
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-textures-bgmap
singleton class BgmapTextureManager : Object
{
	// Number of chars occupied
	u16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];
	// Current x offset to set the next bgmap
	s8 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	// Current y offset to set the next bgmap
	s8 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	// 12 segments, 28 maps, 2 indexes (x,y) and bgmap segment
	s8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][4];
	// Next free bgmap used for text printing
	s16 freeBgmapSegment;
	// The textures allocated
	BgmapTexture bgmapTextures[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];
	// Number of available bgmap segments
	s16 availableBgmapSegmentsForTextures;
	// Segment for printing
	s16 printingBgmapSegment;

	/// @publicsection
	static BgmapTextureManager getInstance();
	void allocateText(BgmapTexture bgmapTexture);
	void calculateAvailableBgmapSegments();
	s16 getAvailableBgmapSegmentsForTextures();
	s16 getPrintingBgmapSegment();
	BgmapTexture getTexture(BgmapTextureDefinition* bgmapTextureDefinition, s16 minimumSegment, bool mustLiveAtEvenSegment);
	s16 getXOffset(int id);
	s16 getYOffset(int id);
	void print(int x, int y);
	void releaseTexture(BgmapTexture bgmapTexture);
	void reset();
	void setSpareBgmapSegments(u8 paramTableSegments);
}


#endif
