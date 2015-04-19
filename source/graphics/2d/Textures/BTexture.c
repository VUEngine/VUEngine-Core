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

#include <BTexture.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(BTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BTexture_constructor(BTexture this, BTextureDefinition* bTextureDefinition, u16 id);
static void BTexture_writeAnimated(BTexture this);
static void BTexture_writeNoAnimated(BTexture this);
static void BTexture_writeNoAnimated(BTexture this);
static void BTexture_writeAnimatedShared(BTexture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BTexture, BTextureDefinition* bTextureDefinition, u16 id)
__CLASS_NEW_END(BTexture, bTextureDefinition, id);

// class's constructor
static void BTexture_constructor(BTexture this, BTextureDefinition* bTextureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE((TextureDefinition*)bTextureDefinition, id);
}

// class's destructor
void BTexture_destructor(BTexture this)
{
	ASSERT(this, "BTexture::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write into memory the chars and this
void BTexture_write(BTexture this)
{
	ASSERT(this, "BTexture::write: null this");

	Texture_write(__UPCAST(Texture, this));
	
	//determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// write the definition to graphic memory
			BTexture_writeAnimated(this);
			break;

		case __ANIMATED_SHARED:

			// write the definition to graphic memory
			BTexture_writeAnimatedShared(this);
			break;

		case __NO_ANIMATED:

			// write the definition to graphic memory
			BTexture_writeNoAnimated(this);
			break;

		default:

			ASSERT(false, "Texture::write: no allocation type");
	}
}

// write an animated map
static void BTexture_writeAnimated(BTexture this)
{
	ASSERT(this, "BTexture::writeAnimated: null this");

	int bgmapSegment = BTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)BTextureManager_getXOffset(BTextureManager_getInstance(), this->id);
	int yOffset = (int)BTextureManager_getYOffset(BTextureManager_getInstance(), this->id);

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
static void BTexture_writeNoAnimated(BTexture this)
{
	ASSERT(this, "BTexture::writeNoAnimated: null this");

	int bgmapSegment = BTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)BTextureManager_getXOffset(BTextureManager_getInstance(), this->id);
	int yOffset = (int)BTextureManager_getYOffset(BTextureManager_getInstance(), this->id);

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
static void BTexture_writeAnimatedShared(BTexture this)
{
	ASSERT(this, "BTexture::writeAnimatedShared: null this");

	int bgmapSegment = BTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int frames = CharSet_getNumberOfChars(this->charSet) / area;

	int i = this->textureDefinition->rows;

	int xOffset = (int)BTextureManager_getXOffset(BTextureManager_getInstance(), this->id);
	int yOffset = (int)BTextureManager_getYOffset(BTextureManager_getInstance(), this->id);

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
u8 BTexture_getXOffset(BTexture this)
{
	ASSERT(this, "BTexture::getXOffset: null this");

	return abs(BTextureManager_getXOffset(BTextureManager_getInstance(), this->id));
}

// get texture's y offset within bgmap mem
u8 BTexture_getYOffset(BTexture this)
{
	ASSERT(this, "BTexture::getYOffset: null this");

	return abs(BTextureManager_getYOffset(BTextureManager_getInstance(), this->id));
}

//get texture's bgmap segment
u8 BTexture_getBgmapSegment(BTexture this)
{
	ASSERT(this, "BTexture::getBgmapSegment: null this");

	return BTextureManager_getBgmapSegment(BTextureManager_getInstance(), this->id);
}
