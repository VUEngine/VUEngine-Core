/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a background in ROM memory
typedef struct TextureDefinition
{
	// pointer to the char definition
	CharSetDefinition* charSetDefinition;

	// pointer to the bgtexture definition in ROM
	BYTE* mapDefinition;

	// x size, 1 column represents 8 pixels
	u32 cols;

	// y size, 1 row represents 8 pixels
	u32 rows;

	// padding for affine/hbias transformations (cols, rows)
	TexturePadding padding;

	// number of frames
	u32 numberOfFrames;

	// palette index to use
	u32 palette;

	// recyclable
	bool recyclable;

} TextureDefinition;

typedef const TextureDefinition TextureROMDef;


abstract class Texture : Object
{
	/**
	* @var CharSet				charSet
	* @brief					Char group to use int this texture
	* @memberof				Texture
	*/
	CharSet charSet;
	/**
	* @var TextureDefinition*	textureDefinition
	* @brief					Pointer to ROM definition
	* @memberof				Texture
	*/
	TextureDefinition* textureDefinition;
	/**
	* @var u32					mapDisplacement
	* @brief					Array definition of the map
	* @memberof				Texture
	*/
	u32 mapDisplacement;
	/**
	* @var u16					id
	* @brief					Texture's id
	* @memberof				Texture
	*/
	u16 id;
	/**
	* @var u8					palette
	* @brief					Color palette
	* @memberof				Texture
	*/
	u8 palette;
	/**
	* @var u8					written
	* @brief					Written flag
	* @memberof				Texture
	*/
	u8 written;

	// A texture which has the logic to be allocated in graphic memory
	void constructor(Texture this, TextureDefinition* textureDefinition, u16 id);
	void setDefinition(Texture this, TextureDefinition* textureDefinition);
	TextureDefinition* getDefinition(Texture this);
	void releaseCharSet(Texture this);
	void writeHBiasMode(Texture this);
	int getNumberOfChars(Texture this);
	TextureDefinition* getTextureDefinition(Texture this);
	u32 getTotalCols(Texture this);
	u32 getTotalRows(Texture this);
	u32 getNumberOfFrames(Texture this);
	CharSet getCharSet(Texture this, u32 loadIfNeeded);
	BYTE* getMapDefinition(Texture this);
	void setPalette(Texture this, u8 palette);
	u8 getPalette(Texture this);
	u32 getRows(Texture this);
	u32 getCols(Texture this);
	u16 getId(Texture this);
	void putChar(Texture this, Point* texturePixel, BYTE* newChar);
	void putPixel(Texture this, Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	bool isWritten(Texture this);
	void setMapDisplacement(Texture this, u32 mapDisplacement);
	virtual void write(Texture this);
	virtual void rewrite(Texture this);
}


#endif
