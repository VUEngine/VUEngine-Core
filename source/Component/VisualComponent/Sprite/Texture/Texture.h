/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSet.h>
#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Texture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// @memberof Texture
enum TextureStatus
{
	kTexturePendingWriting = 1,
	kTexturePendingRewriting,
	kTextureFrameChanged,
	kTextureWritten,
	kTextureInvalid,
	kTextureNoCharSet
};

/// A Texture spec
/// @memberof Texture
typedef struct TextureSpec
{
	/// Pointer to the char spec that the texture uses
	CharSetSpec* charSetSpec;

	/// Pointer to the map array that defines how to use the tiles from the char set
	uint16* map;

	/// Horizontal size in tiles of the texture
	uint8 cols;

	/// Vertical size in tiles of the texture
	uint8 rows;

	/// Padding added to the size for affine/hbias transformations (cols, rows)
	TexturePadding padding;

	/// Number of frames that the texture supports
	uint8 numberOfFrames;

	/// Palette index to use by the graphical data
	uint8 palette;

	/// Flag to recycle the texture with a different map
	bool recyclable;

	// Flag to vertically flip the image
	bool verticalFlip;

	// Flag to horizontally flip the image
	bool horizontalFlip;

} TextureSpec;

/// A Texture spec that is stored in ROM
/// @memberof Texture
typedef const TextureSpec TextureROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class CharSet
///
/// Inherits from ListenerObject
///
/// A texture to be displayed by a sprite.
abstract class Texture : Object
{
	/// Pointer to the implementation that updates graphical data in DRAM
	void (*doUpdate)(Texture, int16);

	/// Char set that holds the pixel data used by the texture
	CharSet charSet;

	/// Spec used to configure the texture
	const TextureSpec* textureSpec;

	/// Displacement inside the map array modified according to the frame's value
	uint32 mapDisplacement;

	/// Keeps track of writes
	uint32 generation;

	/// Identificator
	uint16 id;

	/// Indicator of the block inside the map array to write to DRAM
	uint16 frame;

	/// Palette index to use by the graphical data
	uint8 palette;

	/// Writing status flag
	uint8 status;

	/// Number of references to this texture instance
	int8 usageCount;

	/// Flag to signal that the texture needs to update DRAM in the next render cycle
	bool update;

	/// Get a texture configured with the provided spec.
	/// @param textureClass: Class of texture to instantiate
	/// @param textureSpec: Spec used to select or initialize a texture with
	/// @param minimumSegment: Minimum BGMAP segment where to allocate the texture
	/// @param mustLiveAtEvenSegment: Required BGMAP segment where to allocate the texture
	/// @param scValue: SC configuration value for multi segment textures
	/// @return Texture initialized with the provided spec
	static Texture get
	(
		ClassPointer textureClass, const TextureSpec* textureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
	);

	/// Release a texture.
	/// @param texture: Texture to release
	static void release(Texture texture);

	/// Update texture pending rewriting of data in DRAM.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	/// @param defer: If true, the texture data is written overtime; otherwise
	/// all is written in a single pass
	static void updateTextures(int16 maximumTextureRowsToWrite, bool defer);

	/// Retrieve the total horizontal size of the textures defined by the provided spec.
	/// @param textureSpec: Spec of which to compute the horizontal size
	/// @return Total horizontal size of the textures defined by the provided spec
	static uint32 getTotalCols(TextureSpec* textureSpec);

	/// Retrieve the total vertical size of the textures defined by the provided spec.
	/// @param textureSpec: Spec of which to compute the vertical size
	/// @return Total vertical size of the textures defined by the provided spec
	static uint32 getTotalRows(TextureSpec* textureSpec);

	/// @publicsection

	/// Class' constructor
	/// @param textureSpec: Specification that determines how to configure the texture
	/// @param id: Texture's identificator
	void constructor(const TextureSpec* textureSpec, uint16 id);

	/// Retrieve the texture's identificator.
	/// @return Texture's identificator
	uint16 getId();

	/// Set the texture's spec.
	/// @param textureSpec: Specification that determines how to configure the texture
	void setSpec(TextureSpec* textureSpec);

	/// Retrieve the texture's spec.
	/// @return Specification that determines how to configure the texture
	const TextureSpec* getSpec();

	/// Retrieve the texture's char set.
	/// @param loadIfNeeded: If true and the char set is not loaded, loads it
	/// @return Texture's char set
	CharSet getCharSet(uint32 loadIfNeeded);

	/// Increase the usage count.
	void increaseUsageCount();

	/// Decrease the usage count.
	bool decreaseUsageCount();

	/// Retrieve the usage count.
	/// @return Usage count
	int8 getUsageCount();

	/// Set the palette index to use by the graphical data.
	/// @param palette: Palette index to use by the graphical data
	void setPalette(uint8 palette);

	/// Retrieve the palette index used the graphical data.
	/// @return Palette index used by the graphical data
	uint8 getPalette();

	/// Retrieve the number frames specified by the texture's spec.
	/// @return Number frames specified by the texture's spec
	uint32 getNumberOfFrames();

	/// Write to DRAM the graphical data of the map that corresponds to the specified frame.
	/// @param frame: The frame that species the block inside the map array to write to DRAM
	void setFrame(uint16 frame);

	/// Retrieve frame that species the block inside the map array to write to DRAM.
	/// @return The frame that species the block inside the map array to write to DRAM
	uint16 getFrame();

	/// Retrieve the texture's horizontal size in tiles.
	/// @return Horizontal size in tiles
	uint32 getCols();

	/// Retrieve the texture's vertical size in tiles.
	/// @return Vertical size in tiles
	uint32 getRows();

	/// Check if the texture's data is completely writen to DRAM.
	/// @return True if the texture's data is completely writing to DRAM
	bool isWritten();

	/// Check if the texture is a shared one.
	/// @return True if the texture is shared; false otherwise
	bool isShared();

	/// Check if the texture is an animation or not.
	/// @return True if the CharSet has an array of animation frames
	bool isAnimated();

	/// Check if the texture has only one frame.
	/// @return True if the texture has only one frame; false otherwise
	bool isSingleFrame();

	/// Check if the texture is a multiframe texture.
	/// @return True if the texture is multiframe; false otherwise
	bool isMultiframe();

	/// Add the color provided color data to a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR
	void addChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR
	/// __UINT32S_PER_CHARS(n) provides the offset within a a uint32 array of color data.
	void putChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a pixel in the sprite's texture.
	/// @param texturePixel: Coordinate in texture's space of the CHAR to replace
	/// @param charSetPixel: Coordinate in CHAR space of the CHAR to replace
	/// @param newPixelColor: Color data array for the CHAR
	void putPixel(const Point* texturePixel, const Pixel* charSetPixel, uint8 newPixelColor);

	/// Prepare the texture to write its graphical data to DRAM during
	/// the next render cycle.
	void prepare();

	/// Update the texture's underlying graphics.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	bool update(int16 maximumTextureRowsToWrite);

	/// Write graphical data to the allocated DRAM space.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	/// @return True if the texture was written; false if it fails
	virtual uint8 write(int16 maximumTextureRowsToWrite);

	/// Rewrite graphical data to the allocated DRAM space.
	virtual void rewrite();
}

#endif
