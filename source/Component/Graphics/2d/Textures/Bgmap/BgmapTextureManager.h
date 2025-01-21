/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_TEXTURE_MANAGER_H_
#define BGMAP_TEXTURE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <BgmapTexture.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kCols,
	kRows,
	kBgmapSegment
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BgmapTextureManager
///
/// Inherits from Texture
///
/// Allocates BGMAP space for textures.
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

	/// Reset the manager's state.
	void reset();

	/// Clear the data in the BGMAP spaced defined by the provided segment.
	/// @param segment: Index of the BGMAP segment to clear
	void clearBgmapSegment(int32 segment);

	/// Compute the number of available BGMAP segments for texture allocation.
	/// @param paramTableBase: Displacement in bytes to keep track of the start address of param table space
	void configure(uint32 paramTableBase);

	/// Retrieve the number of available BGMAP segments to allocate textures.
	/// @return Number of available BGMAP segments to allocate textures
	int8 getAvailableBgmapSegmentsForTextures();

	/// Retrieve the number of the BGMAP segment available for printing.
	/// @return Number of the BGMAP segment available for printing
	int8 getPrintingBgmapSegment();

	/// Load textures in function of the provided array of specs.
	/// @param textureSpecs: Array of texture specs in function of which to load textures 
	/// @param removeOldTextures: If true, previously loaded texturered are first destroyed and deallocated
	void loadTextures(const TextureSpec** textureSpecs, bool removeOldTextures);

	/// Retrieve a texture initialized with the provided spec.
	/// @param bgmapTextureSpec: Spec to use to initilize the desired texture
	/// @param minimumSegment: Minimum BGMAP segment where to allocate the texture
	/// @param mustLiveAtEvenSegment: Required BGMAP segment where to allocate the texture
	/// @param scValue: SC configuration value for multi segment textures
	/// @return Texture initialized with the provided spec
	BgmapTexture getTexture(BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue);

	/// Release a texture.
	/// @param bgmapTexture: Texture to release
	void releaseTexture(BgmapTexture bgmapTexture);

	/// Retrieve the X coordinate in BGMAP space for the texture ID provided.
	/// @param id: Texture's id
	/// @return X coordinate in BGMAP space for the texture ID provided
	int16 getXOffset(int32 id);

	/// Retrieve the Y coordinate in BGMAP space for the texture ID provided.
	/// @param id: Texture's id
	/// @return Y coordinate in BGMAP space for the texture ID provided
	int16 getYOffset(int32 id);

	/// Print the manager's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);
}

#endif
