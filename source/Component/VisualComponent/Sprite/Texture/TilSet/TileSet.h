/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TILE_SET_H_
#define TILE_SET_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class TileSet;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Bytes per TILE
#define __BYTES_PER_TILES(n)				((n) << 4)

// TILES PER uint8
#define __TILES_PER_BYTE(n)					((n) >> 4)

// uint32s per TILE
#define __UINT32S_PER_TILES(n)				((n) << 2)

// Compression types
#define __TILE_SET_COMPRESSION_RLE			0x00000001	

// Start address for TILE memory
#define __TILE_SPACE_BASE_ADDRESS			0x00078000

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A TileSet spec
/// @memberof TileSet
typedef struct TileSetSpec
{
	/// Number of tiles in function of the number of frames to load at the same time
	uint16 numberOfChars;

	/// Whether the char set is shared or not
	bool shared;

	/// Whether the tiles are optimized or not
	bool optimized;

	/// Pointer to the graphical data
	uint32* tiles;

	/// Pointer to the array of frames offsets
	uint32* frameOffsets;

} TileSetSpec;

/// A TileSet spec that is stored in ROM
/// @memberof TileSet
typedef const TileSetSpec TileSetROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class TileSet
///
/// Inherits from ListenerObject
///
/// Manages the color data of tile array and writes it to VRAM.
class TileSet : ListenerObject
{
	/// @protectedsection

	/// Spec used in the construction of the char set
	const TileSetSpec* tileSetSpec;

	/// Keeps track of writes
	uint32 generation;

	// Offset in the array of graphical data
	uint32 tilesDisplacement;

	/// Indicator of the block inside the tiles array to write to DRAM
	uint16 frame;

	/// Offset in TILE space where the block allocated for this char set starts
	uint16 offset;

	/// If true the graphical data is written to VRAM; false otherwise
	bool written;

	/// Number of references to this char set instance
	int8 usageCount;

	/// @publicsection

	/// Get a tileSet configured with the provided spec.
	/// @param tileSetSpec: Spec used to select or initialize a texture with
	/// @return TileSet initialized with the provided spec
	static TileSet get(const TileSetSpec* tileSetSpec);

	/// Release a tileSet.
	/// @param tileSet: TileSet to release
	/// @return True if the char set is successfully deleted; false otherwise
	static bool release(TileSet tileSet);

	/// Class' constructor
	/// @param tileSetSpec: Spec to use in the construction of the char set
	/// @param offset: Offset in TILE space where the block allocated for this char set starts
	void constructor(const TileSetSpec* tileSetSpec, uint16 offset);

	/// Increase the usage count.
	void increaseUsageCount();

	/// Decrease the usage count.
	bool decreaseUsageCount();

	/// Retrieve the usage count.
	/// @return Usage count
	int8 getUsageCount();

	/// Check if the TileSet has a non NULL array of frame offests.
	/// @return True if the TileSet has an array of animation frames
	bool hasMultipleFrames();

	/// Check if the char set is shared.
	/// @return True if the char set is share; false otherwise
	bool isShared();

	/// Check if the char set is optimized.
	/// @return True if the char set is optimized; false otherwise
	bool isOptimized();

	/// Set the offset within TILE space.
	/// @param offset: Offset within TILE space
	void setOffset(uint16 offset);

	/// Retrieve the offset within TILE space.
	/// @return Offset within TILE space
	uint16 getOffset();

	/// Retrieve the spec used in the construction of the char set.
	/// @return Spec used in the construction of the char set
	const TileSetSpec* getSpec();

	/// Retrieve the number of TILEs used by the char set.
	/// @return Number of TILEs used by the char set
	uint16 getNumberOfChars();

	/// Add the color provided color data to a TILE.
	/// @param charToAddTo: Index of the TILE to add to
	/// @param newChar: Color data array for the TILE 
	void addChar(uint32 charToAddTo, const uint32* newChar);

	/// Replace the color provided color data to a TILE.
	/// @param charToReplace: Index of the TILE to replace
	/// @param newChar: Color data array for the TILE
	/// __UINT32S_PER_TILES(n) provides the offset within a a uint32 array of color data.
	void putChar(uint32 charToReplace, const uint32* newChar);

	/// Replace a pixel in a TILE.
	/// @param charToReplace: Index of the TILE to replace
	/// @param tileSetPixel: Coordinate in TILE space of the TILE to replace
	/// @param newPixelColor: Color data for the pixel 
	void putPixel(const uint32 charToReplace, const Pixel* tileSetPixel, uint8 newPixelColor);

	/// Set the current frame (frame * number of TILEs + number of TILEs) to write to TILE memory.
	/// @param frame: The frame to write to TILE memory
	void setFrame(uint16 frame);

	/// Retrieve the current frame (frame * number of TILEs + number of TILEs) to write to TILE memory.
	/// @return The frame to write to TILE memory
	uint16 getFrame();

	/// Retrieve the writing count.
	/// @return The count of writes
	uint32 getGeneration();

	/// Write the tile graphical data to VRAM.
	/// @return The count of writes
	uint32 write();
}

#endif
