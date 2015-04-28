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

__CLASS_DEFINITION(Texture, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Texture_onCharSetRewritten(Texture this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Texture, TextureDefinition* textureDefinition, u16 id)
__CLASS_NEW_END(Texture, textureDefinition, id);

// class's constructor
void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE();

	// set id
	this->id = id;

	// save the bgmap definition's address
	this->textureDefinition = textureDefinition;

	// if the char definition is NULL, it must be a text	
	this->charSet = CharSetManager_get(CharSetManager_getInstance(), (CharSetDefinition*)&this->textureDefinition->charSetDefinition);
	ASSERT(this->charSet, "Texture::destructor: null charSet");
	Object_addEventListener(__UPCAST(Object, this->charSet), __UPCAST(Object, this), (void (*)(Object, Object))Texture_onCharSetRewritten, __EVENT_CHARSET_REWRITTEN);
	
	// set the palette
	this->palette = textureDefinition->palette;
}

// class's destructor
void Texture_destructor(Texture this)
{
	ASSERT(this, "Texture::destructor: null this");

	Texture_freeCharMemory(this);

	// destroy the super object
	__DESTROY_BASE;
}

// write an animated map
void Texture_setDefinition(Texture this, TextureDefinition* textureDefinition)
{
	this->textureDefinition = textureDefinition;
}

TextureDefinition* Texture_getDefinition(Texture this)
{
	return this->textureDefinition;
}

// free char memory
void Texture_freeCharMemory(Texture this)
{
	ASSERT(this, "Texture::freeCharMemory: null this");

	if (this->charSet)
	{
		Object_removeEventListener(__UPCAST(Object, this->charSet), __UPCAST(Object, this), (void (*)(Object, Object))Texture_onCharSetRewritten, __EVENT_CHARSET_REWRITTEN);
		CharSetManager_free(CharSetManager_getInstance(), this->charSet);
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
		this->charSet = CharSetManager_get(CharSetManager_getInstance(), (CharSetDefinition*)&this->textureDefinition->charSetDefinition);

		if(this->charSet)
		{
			Object_addEventListener(__UPCAST(Object, this->charSet), __UPCAST(Object, this), (void (*)(Object, Object))Texture_onCharSetRewritten, __EVENT_CHARSET_REWRITTEN);
		}
	}
}

// write into memory the chars and this
void Texture_rewrite(Texture this)
{
	ASSERT(this, "Texture::rewrite: null this");

	CharSet_rewrite(this->charSet);
	
	__VIRTUAL_CALL(void, Texture, write, this);
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
		case __ANIMATED_SINGLE:

			// just return the cols
			return this->textureDefinition->cols;
			break;

		case __ANIMATED_MULTI:
		case __ANIMATED_SHARED:
			{
				// return the total number of chars
				int totalCols = this->textureDefinition->numberOfFrames * this->textureDefinition->cols;
				return 64 >= totalCols? totalCols: 64;
			}
			break;

		case __NOT_ANIMATED:

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
		case __ANIMATED_SINGLE:

			// just return the cols
			return this->textureDefinition->rows;
			break;

		case __ANIMATED_MULTI:
		case __ANIMATED_SHARED:
			{
				// return the total number of chars
				int totalCols = Texture_getTotalCols(this);
				return this->textureDefinition->rows + this->textureDefinition->rows * ((int)(totalCols / 64));
			}
			break;

		case __NOT_ANIMATED:

			// just return the cols
			return this->textureDefinition->rows;
			break;
	}

	return 0;
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
BYTE* Texture_getBgmapDefinition(Texture this)
{
	ASSERT(this, "Texture::getBgmapDef: null this");

	return this->textureDefinition? this->textureDefinition->bgmapDefinition: NULL;
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

// process event
static void Texture_onCharSetRewritten(Texture this, Object eventFirer)
{
	__VIRTUAL_CALL(void, Texture, write, this);

	// propagate event
	Object_fireEvent(__UPCAST(Object, this), __EVENT_TEXTURE_REWRITTEN);
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

