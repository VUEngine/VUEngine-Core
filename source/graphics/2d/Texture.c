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

#include <Texture.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id);
static void Texture_writeAnimated(Texture this);
static void Texture_writeNoAnimated(Texture this);
static void Texture_writeAnimatedShared(Texture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Texture, __PARAMETERS(TextureDefinition* textureDefinition, u16 id))
__CLASS_NEW_END(Texture, __ARGUMENTS(textureDefinition, id));

// class's constructor
static void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	// set id
	this->id = id;

	// save the bgmap definition's address
	this->textureDefinition = textureDefinition;

	// if the char definition is NULL, it must be a text
	this->charSet = __NEW(CharSet, __ARGUMENTS((CharSetDefinition*)&this->textureDefinition->charSetDefinition, (Object)this));

	// set the palette
	this->palette = textureDefinition->palette;
}

// class's destructor
void Texture_destructor(Texture this)
{
	ASSERT(this, "Texture::destructor: null this");

	Texture_freeCharMemory(this);

	// destroy the super object
	__DESTROY_BASE(Object);
}
//extern void addmem1 (u8* dest, const u8* src, u16 num, u16 offset);

// write an animated map
static void Texture_writeAnimated(Texture this)
{
	ASSERT(this, "Texture::writeAnimated: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int palette = Texture_getPallet(this) << 14;

	int charLocation = (CharSet_getCharSet(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = (int)TextureManager_getYOffset(TextureManager_getInstance(), this->id);

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
static void Texture_writeNoAnimated(Texture this)
{
	ASSERT(this, "Texture::writeNoAnimated: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int palette = Texture_getPallet(this) << 14;

	int charLocation = (CharSet_getCharSet(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = (int)TextureManager_getYOffset(TextureManager_getInstance(), this->id);

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
static void Texture_writeAnimatedShared(Texture this)
{
	ASSERT(this, "Texture::writeAnimatedShared: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int palette = Texture_getPallet(this) << 14;

	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (CharSet_getCharSet(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int frames = CharSet_getNumberOfChars(this->charSet) / area;

	int i = this->textureDefinition->rows;

	int xOffset = (int)TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = (int)TextureManager_getYOffset(TextureManager_getInstance(), this->id);

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

// free char memory
void Texture_freeCharMemory(Texture this)
{
	ASSERT(this, "Texture::freeCharMemory: null this");

	if (this->charSet)
	{
		//destroy the charset
		__DELETE(this->charSet);

		this->charSet = NULL;
	}
}

// write into memory the chars and this
void Texture_write(Texture this)
{
	ASSERT(this, "Texture::write: null this");

	if (!this->charSet)
	{
		// if the char definition is NULL, it must be a text
		this->charSet = __NEW(CharSet, __ARGUMENTS((CharSetDefinition*)&this->textureDefinition->charSetDefinition, (Object)this));
	}

	//write char group
	CharSet_write(this->charSet);

	//determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// write the definition to graphic memory
			Texture_writeAnimated(this);

			break;

		case __ANIMATED_SHARED:

			// write the definition to graphic memory
			Texture_writeAnimatedShared(this);

			break;

		case __NO_ANIMATED:

			// write the definition to graphic memory
			Texture_writeNoAnimated(this);

			break;

		default:

			ASSERT(false, "Texture::write: no allocation type");
	}
}

// write into memory the chars and this
void Texture_rewrite(Texture this)
{
	ASSERT(this, "Texture::rewrite: null this");

	CharSet_rewrite(this->charSet);
	
	Texture_write(this);
}

// write map in hbias mode
void Texture_writeHBiasMode(Texture this)
{
	ASSERT(this, "Texture::writeHBiasMode: null this");

	// TODO
	/*
	int i;
	//put the this into memory calculation the number of char for each reference
	for (i=0;i<this->textureDefinition->rows;i++)
	{
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureDefinition->bgmapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet_getCharSet(&this->charSet)<<9)+CharSet_getOffset(&this->charSet)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+64* const this->yOffset+64*i)<<1), this->textureDefinition->bgmapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet_getCharSet(&this->charSet)<<9)+CharSet_getOffset(&this->charSet)));
	}
	*/
}

// get texture's number of chars
int Texture_getNumberOfChars(Texture this)
{
	ASSERT(this, "Texture::getNumberOfChars: null this");
	ASSERT(this->charSet, "Texture::getNumberOfChars: null charset");

	return CharSet_getNumberOfChars(this->charSet);
}

// get texture's x offset within bgmap mem
u8 Texture_getXOffset(Texture this)
{
	ASSERT(this, "Texture::getXOffset: null this");

	return abs(TextureManager_getXOffset(TextureManager_getInstance(), this->id));
}

// get texture's y offset within bgmap mem
u8 Texture_getYOffset(Texture this)
{
	ASSERT(this, "Texture::getYOffset: null this");

	return abs(TextureManager_getYOffset(TextureManager_getInstance(), this->id));
}

// get texture's definition
TextureDefinition* Texture_getTextureDefinition(Texture this)
{
	ASSERT(this, "Texture::getTextureDefinition: null this");

	return this->textureDefinition;
}

// get texture's cols
u8 Texture_getTotalCols(Texture this)
{
	ASSERT(this, "Texture::getTotalCols: null this");

	// determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// just return the cols
			return this->textureDefinition->cols;
			break;

		case __ANIMATED_SHARED:
			{
				// return the total number of chars
				int totalCols = this->textureDefinition->numberOfFrames * this->textureDefinition->cols;
				return 64 >= totalCols? totalCols: 64;
			}
			break;

		case __NO_ANIMATED:

			// just return the cols
			return this->textureDefinition->cols;
			break;
	}

	return 0;
}

//get texture's rows
u8 Texture_getTotalRows(Texture this)
{
	ASSERT(this, "Texture::getTotalRows: null this");

	// determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// just return the cols
			return this->textureDefinition->rows;
			break;

		case __ANIMATED_SHARED:
			{
				// return the total number of chars
				int totalCols = Texture_getTotalCols(this);
				return this->textureDefinition->rows + this->textureDefinition->rows * ((int)(totalCols / 64));
			}
			break;

		case __NO_ANIMATED:

			// just return the cols
			return this->textureDefinition->rows;
			break;
	}

	return 0;
}

//get texture's bgmap segment
u8 Texture_getBgmapSegment(Texture this)
{
	ASSERT(this, "Texture::getBgmapSegment: null this");

	return TextureManager_getBgmapSegment(TextureManager_getInstance(), this->id);
}

// get number of frames of animation
u8 Texture_getNumberOfFrames(Texture this)
{
	ASSERT(this, "Texture::getNumberOfFrames: null this");

	return this->textureDefinition->numberOfFrames;
}

//get texture's charset
CharSet Texture_getCharSet(Texture this)
{
	ASSERT(this, "Texture::getCharSet: null this");

	return this->charSet;
}

//get texture's bgmap definition
BYTE* Texture_getBgmapDef(Texture this)
{
	ASSERT(this, "Texture::getBgmapDef: null this");

	return this->textureDefinition->bgmapDefinition;
}

// set the palette
void Texture_setPallet(Texture this, u8 palette)
{
	ASSERT(this, "Texture::setPallet: null this");

	this->palette = palette;
}

u8 Texture_getPallet(Texture this)
{
	ASSERT(this, "Texture::getPallet: null this");

	return this->palette;
}

// retrieve texture's rows
u8 Texture_getRows(Texture this)
{
	ASSERT(this, "Texture::getRows: null this");
	//ASSERT(this->textureDefinition, "Texture::getRows: 0 rows");

	return this->textureDefinition->rows;
}

// retrieve texture's cols
u8 Texture_getCols(Texture this)
{
	ASSERT(this, "Texture::getCols: null this");

	return this->textureDefinition->cols;
}

// retrieve texture's id
u16 Texture_getId(Texture this)
{
	ASSERT(this, "Texture::getId: null this");

	return this->id;
}

// process a telegram
bool Texture_handleMessage(Texture this, Telegram telegram)
{
	ASSERT(this, "Texture::handleMessage: null this");

	switch (Telegram_getMessage(telegram))
	{
		case kCharSetRewritten:

			Texture_write(this);
			return true;
			break;
	}

	return false;
}

// write directly to texture
void Texture_putChar(Texture this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Texture::putChar: null this");

	if(texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u16 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u16 charToReplace = this->textureDefinition->bgmapDefinition[displacement];
		CharSet_putChar(this->charSet, charToReplace, newChar);
	}
}


// write directly to texture
void Texture_putPixel(Texture this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Texture::putPixel: null this");

	if(texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u16 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u16 charToReplace = this->textureDefinition->bgmapDefinition[displacement];
		CharSet_putPixel(this->charSet, charToReplace, charSetPixel, newPixelColor);
	}
}

