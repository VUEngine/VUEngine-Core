/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTexture.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(BgmapTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BgmapTexture_constructor(BgmapTexture this, BgmapTextureDefinition* bgmapTextureDefinition, u16 id);
static void BgmapTexture_writeAnimated(BgmapTexture this);
static void BgmapTexture_writeNoAnimated(BgmapTexture this);
static void BgmapTexture_writeNoAnimated(BgmapTexture this);
static void BgmapTexture_writeAnimatedShared(BgmapTexture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapTexture, BgmapTextureDefinition* bgmapTextureDefinition, u16 id)
__CLASS_NEW_END(BgmapTexture, bgmapTextureDefinition, id);

// class's constructor
static void BgmapTexture_constructor(BgmapTexture this, BgmapTextureDefinition* bgmapTextureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE((TextureDefinition*)bgmapTextureDefinition, id);
}

// class's destructor
void BgmapTexture_destructor(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write into memory the chars and this
void BgmapTexture_write(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::write: null this");

	Texture_write(__UPCAST(Texture, this));
	
	//determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// write the definition to graphic memory
			BgmapTexture_writeAnimated(this);
			break;

		case __ANIMATED_SHARED:

			// write the definition to graphic memory
			BgmapTexture_writeAnimatedShared(this);
			break;

		case __NO_ANIMATED:

			// write the definition to graphic memory
			BgmapTexture_writeNoAnimated(this);
			break;

		default:

			ASSERT(false, "Texture::write: no allocation type");
	}
}

// write an animated map
static void BgmapTexture_writeAnimated(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeAnimated: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if (0 > xOffset || 0 > yOffset)
	{
		return;
	}

	//put the map into memory calculating the number of char for each reference
	for (; i--;)
	{
		Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (i << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));

	}
}

// write an inanimated map
static void BgmapTexture_writeNoAnimated(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeNoAnimated: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if (0 > xOffset || 0 > yOffset)
	{
		return;
	}
	
	//put the map into memory calculating the number of char for each reference
	for (; i--;)
	{
		//specifying the char displacement inside the char mem
		Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (i << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));
	}
}

// write an animated and shared map
static void BgmapTexture_writeAnimatedShared(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeAnimatedShared: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int frames = CharSet_getNumberOfChars(this->charSet) / area;

	int i = this->textureDefinition->rows;

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if (0 > xOffset || 0 > yOffset)
	{
		return;
	}

	//put the map into memory calculating the number of char for each reference
	for (; i--;)
	{
		int j = 1;
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		for (; j <= frames; j++)
		{
			Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (this->textureDefinition->cols * (j - 1)) + (yOffset << 6) + (i << 6)) << 1),
					(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
					this->textureDefinition->cols,
					(palette) | (charLocation + area * (j - 1)));
		}
	}
}

// get texture's x offset within bgmap mem
u8 BgmapTexture_getXOffset(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getXOffset: null this");

	return abs(BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id));
}

// get texture's y offset within bgmap mem
u8 BgmapTexture_getYOffset(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getYOffset: null this");

	return abs(BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id));
}

//get texture's bgmap segment
u8 BgmapTexture_getBgmapSegment(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getBgmapSegment: null this");

	return BgmapTextureManager_getBgmapSegment(BgmapTextureManager_getInstance(), this->id);
}
