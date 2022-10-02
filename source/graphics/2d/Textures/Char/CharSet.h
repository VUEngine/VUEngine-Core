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

// spec of a CharSet for unanimated sprites
#define __NOT_ANIMATED						0x01

// spec of a CharSet for animated sprites
#define __ANIMATED_SINGLE					0x02

// spec of a CharSet for animated sprites
#define __ANIMATED_SINGLE_OPTIMIZED			0x03

// spec of a CharSet for animated sprites with one char set is shared by all
#define __ANIMATED_SHARED					0x04

// spec of a CharSet for animated sprites with a coordinator that syncs them
#define __ANIMATED_SHARED_COORDINATED		0x05

// spec of a charset for animated sprites whose all frames are written to memory and shared
#define __ANIMATED_MULTI					0x06

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
	/// number of chars, depending on allocation type:
	/// __ANIMATED_SINGLE: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_SHARED: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_SHARED_COORDINATED: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_MULTI: sum of chars of all animation frames
	/// __NOT_ANIMATED: number of chars of whole image
	uint16 numberOfChars;

	/// the way its chars and bgtexture will be allocated in graphic memory
	uint8 allocationType;

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

/// @ingroup graphics-2d-textures-char
class CharSet : ListenerObject
{
	// Charset spec
	CharSetSpec* charSetSpec;
	// Array spec of the charSet
	uint32 tilesDisplacement;
	// Memory displacement
	uint16 offset;
	// How many textures are using me
	uint8 usageCount;

	/// @publicsection
	void constructor(CharSetSpec* charSetSpec, uint16 offset);
	void increaseUsageCount();
	bool decreaseUsageCount();
	uint32 getAllocationType();
	uint32 getOffset();
	void setOffset(uint16 offset);
	void setCharSetSpec(CharSetSpec* charSetSpec);
	CharSetSpec* getCharSetSpec();
	uint32 getNumberOfChars();
	void write();
	void rewrite();
	void setTilesDisplacement(uint32 tilesDisplacement);
	void putChar(uint32 charToReplace, uint32* newChar);
	void putPixel(uint32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor);
	void setFrame(uint16 frame);
}


#endif
