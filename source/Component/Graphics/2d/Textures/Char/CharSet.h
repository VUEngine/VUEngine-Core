/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CHARSET_H_
#define CHARSET_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Bytes per CHAR
#define __BYTES_PER_CHARS(n)				((n) << 4)

// CHARS PER BYTE
#define __CHARS_PER_BYTE(n)				((n) >> 4)

// uint32s per CHAR
#define __UINT32S_PER_CHARS(n)				((n) << 2)

// Compression types
#define __CHAR_SET_COMPRESSION_RLE			0x00000001	

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A CharSet spec
/// @memberof CharSet
typedef struct CharSetSpec
{
	/// Number of CHARs in function of the number of frames to load at the same time
	uint16 numberOfChars;

	/// Whether the char set is shared or not
	bool shared;

	/// Whether the tiles are optimized or not
	bool optimized;

	/// Pointer to the graphical data
	uint32* tiles;

	/// Pointer to the array of frames offsets
	uint32* frameOffsets;

} CharSetSpec;

/// A CharSet spec that is stored in ROM
/// @memberof CharSet
typedef const CharSetSpec CharSetROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class CharSet
///
/// Inherits from ListenerObject
///
/// Manages the color data of tile array and writes it to VRAM.
class CharSet : ListenerObject
{
	/// @protectedsection

	/// Spec used in the construction of the char set
	CharSetSpec* charSetSpec;

	// Offset in the array of graphical data
	uint32 tilesDisplacement;

	/// Offset in CHAR space where the block allocated for this char set starts
	uint16 offset;

	/// If true the graphical data is written to VRAM; false otherwise
	bool written;

	/// Number of references to this char set instance
	uint8 usageCount;

	/// @publicsection

	/// Class' constructor
	/// @param charSetSpec: Spec to use in the construction of the char set
	/// @param offset: Offset in CHAR space where the block allocated for this char set starts
	void constructor(CharSetSpec* charSetSpec, uint16 offset);

	/// Increase the usage count.
	void increaseUsageCount();

	/// Decrease the usage count.
	bool decreaseUsageCount();

	/// Retrieve the usage count.
	/// @return Usage count
	uint8 getUsageCount();

	/// Check if the char set is shared.
	/// @return True if the char set is share; false otherwise
	bool isShared();

	/// Check if the char set is optimized.
	/// @return True if the char set is optimized; false otherwise
	bool isOptimized();

	/// Set the offset within CHAR space.
	/// @param offset: Offset within CHAR space
	void setOffset(uint16 offset);

	/// Retrieve the offset within CHAR space.
	/// @return Offset within CHAR space
	uint16 getOffset();

	/// Retrieve the spec used in the construction of the char set.
	/// @return Spec used in the construction of the char set
	CharSetSpec* getSpec();

	/// Retrieve the number of CHARs used by the char set.
	/// @return Number of CHARs used by the char set
	uint16 getNumberOfChars();

	/// Add the color provided color data to a CHAR.
	/// @param charToAddTo: Index of the CHAR to add to
	/// @param newChar: Color data array for the CHAR 
	void addChar(uint32 charToAddTo, const uint32* newChar);

	/// Replace the color provided color data to a CHAR.
	/// @param charToReplace: Index of the CHAR to replace
	/// @param newChar: Color data array for the CHAR 
	void putChar(uint32 charToReplace, const uint32* newChar);

	/// Replace a pixel in a CHAR.
	/// @param charToReplace: Index of the CHAR to replace
	/// @param charSetPixel: Coordinate in CHAR space of the CHAR to replace
	/// @param newPixelColor: Color data for the pixel 
	void putPixel(const uint32 charToReplace, const Pixel* charSetPixel, BYTE newPixelColor);

	/// Set the current frame (frame * number of CHARs + number of CHARs) to write to CHAR memory.
	/// @param frame: The frame to write to CHAR memory
	void setFrame(uint16 frame);

	/// Write the tile graphical data to VRAM.
	void write();
}

#endif
