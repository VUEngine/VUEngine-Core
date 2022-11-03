/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

enum TextureStatus
{
	kTexturePendingWriting = 1,
	kTextureSpecChanged,
	kTexturePendingRewriting,
	kTextureMapDisplacementChanged,
	kTextureFrameChanged,
	kTextureWritten,
	kTextureInvalid
};

/**
 * A Texture spec
 *
 * @memberof Texture
 */
typedef struct TextureSpec
{
	/// pointer to the char spec
	CharSetSpec* charSetSpec;

	/// pointer to the bgtexture spec in ROM
	uint16* map;

	/// x size, 1 column represents 8 pixels
	uint8 cols;

	/// y size, 1 row represents 8 pixels
	uint8 rows;

	/// padding for affine/hbias transformations (cols, rows)
	TexturePadding padding;

	/// number of frames
	uint8 numberOfFrames;

	/// palette index to use
	uint8 palette;

	/// recyclable
	bool recyclable;

	// vertical flip
	bool verticalFlip;

	// horizontal flip
	bool horizontalFlip;

} TextureSpec;

/**
 * A Texture spec that is stored in ROM
 *
 * @memberof Texture
 */
typedef const TextureSpec TextureROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// A texture which has the logic to be allocated in graphic memory
/// @ingroup graphics-2d-textures
abstract class Texture : ListenerObject
{
	// Char group to use int32 this texture
	CharSet charSet;
	// Pointer to ROM spec
	TextureSpec* textureSpec;
	// Array spec of the map
	uint32 mapDisplacement;
	// Texture's id
	uint16 id;
	uint16 frame;
	// Color palette
	uint8 palette;
	// Status flag
	uint8 status;
	// How many sprites are using me
	uint8 usageCount;
	// update flag
	bool update;

	/// @publicsection
	void constructor(TextureSpec* textureSpec, uint16 id);
	void setSpec(TextureSpec* textureSpec);
	TextureSpec* getSpec();
	void releaseCharSet();
	void writeHBiasMode();
	int32 getNumberOfChars();
	TextureSpec* getTextureSpec();
	uint32 getTotalCols();
	uint32 getTotalRows();
	uint32 getNumberOfFrames();
	CharSet getCharSet(uint32 loadIfNeeded);
	void setCharSet(CharSet charSet);
	uint16* getMap();
	void setPalette(uint8 palette);
	uint8 getPalette();
	uint32 getRows();
	uint32 getCols();
	uint16 getId();
	uint8 getUsageCount();
	void increaseUsageCount();
	bool decreaseUsageCount();
	void putChar(Point* texturePixel, uint32* newChar);
	void putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	bool isWritten();
	void setMapDisplacement(uint32 mapDisplacement);
	void setFrame(uint16 frame);
	uint16 getFrame();
	bool prepare();
	bool update();
	uint8 getAllocationType();
	virtual bool write();
	virtual void rewrite();
	virtual void setFrameAnimatedMulti(uint16 frame);
}


#endif
