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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>
#include <BgmapTexture.h>


//=========================================================================================================
// CLASS'S MACROS
//=========================================================================================================

#define __BGMAP_SEGMENT_SIZE	8192


//=========================================================================================================
// CLASS'S DATA
//=========================================================================================================

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kCols,
	kRows,
	kBgmapSegment
};


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class BgmapTextureManager
///
/// Inherits from Texture
///
/// Allocates BGMAP space for textures.
/// @ingroup graphics-2d-textures-bgmap
singleton class BgmapTextureManager : Object
{
	// List of textures with BGMAP space allocated for them
	VirtualList bgmapTextures;

	/// Used tiles per BGMAP segment
	uint16 usedTiles[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];

	/// Start X coordinate of free space in each BGMAP segment
	int8 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];
	
	/// Start Y coordinate of free space in each BGMAP segment
	int8 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];

	/// Multidimensional array to keep track of the offsets for each allocated texture
	int8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][4];
	
	/// Number of available BGMAP segments for texture allocation
	int8 availableBgmapSegmentsForTextures;

	/// BGMAP segment available for printing
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
