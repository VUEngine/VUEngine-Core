/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	// Allocated textures
	VirtualList bgmapTextures;
	// Number of chars occupied
	uint16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];
	// Current x offset to set the next bgmap
	int8 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	// Current y offset to set the next bgmap
	int8 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	// 12 segments, 28 maps, 2 indexes (x,y) and bgmap segment
	int8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][4];
	// Number of available bgmap segments
	int8 availableBgmapSegmentsForTextures;
	// Segment for printing
	int8 printingBgmapSegment;

	/// @publicsection
	static BgmapTextureManager getInstance();
	void reset();
	void loadTextures(const TextureSpec** textureSpecs);
	void calculateAvailableBgmapSegments();
	int8 getAvailableBgmapSegmentsForTextures();
	int8 getPrintingBgmapSegment();
	BgmapTexture getTexture(BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue);
	int16 getXOffset(int32 id);
	int16 getYOffset(int32 id);
	void print(int32 x, int32 y);
	void releaseTexture(BgmapTexture bgmapTexture);
	void setDeferTextureUpdate(bool value);
}


#endif
