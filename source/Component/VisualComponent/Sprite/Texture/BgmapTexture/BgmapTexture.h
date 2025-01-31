/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_TEXTURE_H_
#define BGMAP_TEXTURE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Texture.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Start address of BGMAP space
#define __BGMAP_SPACE_BASE_ADDRESS			0x00020000	

// Address of BGMap b (0 <= b <= 13)
#define __BGMAP_SEGMENT(b)					(__BGMAP_SPACE_BASE_ADDRESS + ((b) * 0x2000))  

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A BgmapTexture spec
/// @memberof BgmapTexture
typedef const TextureSpec BgmapTextureSpec;

/// A BgmapTexture spec that is stored in ROM
/// @memberof BgmapTexture
typedef const BgmapTextureSpec BgmapTextureROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BgmapTexture
///
/// Inherits from Texture
///
/// A texture allocated in BGMAP memory.
class BgmapTexture : Texture
{
	/// @protectedsection

	/// BGMAP segment where the graphical data is allocated
	int8 segment;

	/// Remaining rows to be written to graphical memory
	int8 remainingRowsToBeWritten;

	/// X coordinate in tiles of the graphical data inside BGMAP memory
	int16 xOffset;

	/// Y coordinate in tiles of the graphical data inside BGMAP memory
	int16 yOffset;

	/// Flag to flip horizontally the texture
	bool horizontalFlip;

	/// Flag to flip vertically the texture
	bool verticalFlip;

	/// @publicsection

	/// Class' constructor
	/// @param bgmapTextureSpec: Specification that determines how to configure the texture
	/// @param id: Texture's identificator
	void constructor(const BgmapTextureSpec* bgmapTextureSpec, uint16 id);

	/// Write graphical data to the allocated BGMAP space.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	/// @return True if the texture was written; false if it fails
	override bool write(int16 maximumTextureRowsToWrite);

	/// Rewrite graphical data to the allocated BGMAP space.
	override void rewrite();

	/// Set the BGMAP segment where the graphical data is allocated.
	/// @param segment: BGMAP segment where the graphical data is allocated
	void setSegment(int8 segment);

	/// Retriev the BGMAP segment where the graphical data is allocated.
	/// @return BGMAP segment where the graphical data is allocated
	int8 getSegment();

	/// Set the coordinates in tiles of the graphical data inside BGMAP memory  BGMAP segment
	/// where the graphical data is allocated.
	/// @param xOffset: X coordinate in tiles of the graphical data inside BGMAP memory
	/// @param yOffset: Y coordinate in tiles of the graphical data inside BGMAP memory
	void setOffsets(int16 xOffset, int16 yOffset);

	/// Retrieve the X coordinate in tiles of the graphical data inside BGMAP memory.
	/// @return X coordinate in tiles of the graphical data inside BGMAP memory
	int16 getXOffset();

	/// Retrieve the Y coordinate in tiles of the graphical data inside BGMAP memory.
	/// @return Y coordinate in tiles of the graphical data inside BGMAP memory
	int16 getYOffset();

	/// Set the horizontal flip flag.
	/// @param value: If true, the texture is flipped horizontally
	void setHorizontalFlip(bool value);

	/// Set the vertical flip flag.
	/// @param value: If true, the texture is flipped vertically
	void setVerticalFlip(bool value);

	/// Retrieve the number of remaining rows to be written to graphical memory.
	/// @return Number of remaining rows to be written to graphical memory
	int8 getRemainingRowsToBeWritten();
}

#endif
