/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

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
	BYTE* mapSpec;

	/// x size, 1 column represents 8 pixels
	u32 cols;

	/// y size, 1 row represents 8 pixels
	u32 rows;

	/// padding for affine/hbias transformations (cols, rows)
	TexturePadding padding;

	/// number of frames
	u32 numberOfFrames;

	/// palette index to use
	u32 palette;

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
abstract class Texture : Object
{
	// Char group to use int this texture
	CharSet charSet;
	// Pointer to ROM spec
	TextureSpec* textureSpec;
	// Array spec of the map
	u32 mapDisplacement;
	// Texture's id
	u16 id;
	// Color palette
	u8 palette;
	// Written flag
	u8 written;

	/// @publicsection
	void constructor(TextureSpec* textureSpec, u16 id);
	void setSpec(TextureSpec* textureSpec);
	TextureSpec* getSpec();
	void releaseCharSet();
	void writeHBiasMode();
	int getNumberOfChars();
	TextureSpec* getTextureSpec();
	u32 getTotalCols();
	u32 getTotalRows();
	u32 getNumberOfFrames();
	CharSet getCharSet(u32 loadIfNeeded);
	BYTE* getMapSpec();
	void setPalette(u8 palette);
	u8 getPalette();
	u32 getRows();
	u32 getCols();
	u16 getId();
	void putChar(Point* texturePixel, BYTE* newChar);
	void putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	bool isWritten();
	void setMapDisplacement(u32 mapDisplacement);
	void setFrame(u16 frame);
	virtual void write();
	virtual void rewrite();
	virtual void setFrameAnimatedMulti(u16 frame);
}


#endif
