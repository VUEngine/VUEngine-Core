/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CHARSET_H_
#define CHARSET_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// Bytes per CHAR
#define __BYTES_PER_CHARS(n)				((n) << 4)

// uint32s per CHAR
#define __UINT32S_PER_CHARS(n)				((n) << 2)

// Compression types
#define __CHAR_SET_COMPRESSION_RLE			0x00000001	

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A CharSet spec
 *
 * @memberof CharSet
 */
typedef struct CharSetSpec
{
	/// number of chars in function of the number of frames to load at the same time
	uint16 numberOfChars;

	/// whether it is shared or not
	bool shared;

	/// whether the tiles are optimized or not
	bool optimized;

	/// pointer to the char spec in ROM
	uint32* tiles;

	/// pointer to the frames offsets
	uint32* frameOffsets;

} CharSetSpec;

/**
 * A CharSet spec that is stored in ROM
 *
 * @memberof CharSet
 */
typedef const CharSetSpec CharSetROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

enum CharSetStatus
{
	kCharSetNotWritten = 0,
	kCharSetPendingRewritting,
	kCharSetWritten
};

/// @ingroup graphics-2d-textures-char
class CharSet : ListenerObject
{
	// Charset spec
	CharSetSpec* charSetSpec;
	// Array spec of the charSet
	uint32 tilesDisplacement;
	// Memory displacement
	uint16 offset;
	// Written flag
	uint16 status;
	// How many textures are using me
	uint8 usageCount;

	/// @publicsection
	void constructor(CharSetSpec* charSetSpec, uint16 offset);
	void increaseUsageCount();
	bool decreaseUsageCount();
	uint8 getUsageCount();
	bool isShared();
	bool isOptimized();
	uint16 getOffset();
	void setOffset(uint16 offset);
	void setCharSetSpec(CharSetSpec* charSetSpec);
	CharSetSpec* getCharSetSpec();
	uint16 getNumberOfChars();
	void write();
	void rewrite();
	void setTilesDisplacement(uint32 tilesDisplacement);
	void putChar(uint32 charToReplace, uint32* newChar);
	void putPixel(uint32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor);
	bool setFrame(uint16 frame);
}


#endif
