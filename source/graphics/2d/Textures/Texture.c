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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSetManager.h>
#include <Optics.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	Texture
 * @extends Object
 * @ingroup graphics-2d-textures
 */


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof					Texture
 * @private
 *
 * @param this					Function scope
 * @param textureDefinition		Definition to use
 * @param id					Texture's identification
 */
void Texture::constructor(TextureDefinition* textureDefinition, u16 id)
{
	// construct base object
	Base::constructor();

	// set id
	this->id = id;

	this->mapDisplacement = 0;

	// save the bgmap definition's address
	this->textureDefinition = textureDefinition;
	this->charSet = NULL;
	// set the palette
	this->palette = textureDefinition->palette;
	this->written = false;
}

/**
 * Class destructor
 *
 * @memberof	Texture
 * @public
 *
 * @param this	Function scope
 */
void Texture::destructor()
{
	Texture::releaseCharSet(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Load the CharSet defined by the TextureDefinition
 *
 * @memberof	Texture
 * @private
 *
 * @param this	Function scope
 */
void Texture::loadCharSet()
{
	Texture::releaseCharSet(this);

	this->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), this->textureDefinition->charSetDefinition);
	ASSERT(this->charSet, "Texture::constructor: null charSet");
	// if the char definition is NULL, it must be a text
	Object::addEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture_onCharSetRewritten, kEventCharSetRewritten);
	Object::addEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture_onCharSetDeleted, kEventCharSetDeleted);
}

/**
 * Set the TextureDefinition
 *
 * @memberof					Texture
 * @public
 *
 * @param this					Function scope
 * @param textureDefinition		New TextureDefinition
 */
void Texture::setDefinition(TextureDefinition* textureDefinition)
{
	ASSERT(textureDefinition, "Texture::setDefinition: null textureDefinition");

	this->textureDefinition = textureDefinition;

	Texture::releaseCharSet(this);
}

/**
 * Retrieve the TextureDefinition
 *
 * @memberof			Texture
 * @public
 *
 * @param this			Function scope
 *
 * @return				TextureDefinition
 */
TextureDefinition* Texture::getDefinition()
{
	return this->textureDefinition;
}

/**
 * Release the CharSet
 *
 * @memberof					Texture
 * @public
 *
 * @param this					Function scope
 */
void Texture::releaseCharSet()
{
	if(this->charSet)
	{
		Object::removeEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture_onCharSetRewritten, kEventCharSetRewritten);
		Object::removeEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture_onCharSetDeleted, kEventCharSetDeleted);

		CharSetManager::releaseCharSet(CharSetManager::getInstance(), this->charSet);

		this->charSet = NULL;
	}

	this->written = false;
}

/**
 * Write the map to DRAM
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 */
void Texture::write()
{
	ASSERT(this->textureDefinition, "Texture::write: null textureDefinition");
	ASSERT(this->textureDefinition->charSetDefinition, "Texture::write: null charSetDefinition");

	if(!this->charSet)
	{
		Texture::loadCharSet(this);
	}

	this->written = true;
}

/**
 * Rewrite the map to DRAM
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 */
void Texture::rewrite()
{
	this->written = false;

	Texture::write(this);
}

/**
 * Write to DRAM in h-bias mode
 *
 * @memberof					Texture
 * @public
 *
 * @param this					Function scope
 */
void Texture::writeHBiasMode()
{
	// TODO
	/*
	int i;
	//put the this into memory calculation the number of char for each reference
	for(i=0;i<this->textureDefinition->rows;i++)
	{
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureDefinition->mapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+64* const this->yOffset+64*i)<<1), this->textureDefinition->mapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
	}
	*/
}

/**
 * Retrieve the number of CHARs according to the TextureDefinition's CharDefinition
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of CHARs
 */
int Texture::getNumberOfChars()
{
	return this->textureDefinition->charSetDefinition->numberOfChars;
}

/**
 * Retrieve the TextureDefinition
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			TextureDefinition
 */
TextureDefinition* Texture::getTextureDefinition()
{
	return this->textureDefinition;
}

/**
 * Retrieve map's total column size, accounting for the total frames of animation
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of total columns
 */
u32 Texture::getTotalCols()
{
	// determine the allocation type
	switch(this->textureDefinition->charSetDefinition->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:

			// just return the cols
			return this->textureDefinition->cols;
			break;

		case __ANIMATED_MULTI:
			{
				// return the total number of chars
				int totalCols = this->textureDefinition->numberOfFrames * this->textureDefinition->cols;
				return 64 >= totalCols ? totalCols : 64;
			}
			break;

		case __NOT_ANIMATED:

			// just return the cols
			return this->textureDefinition->cols;
			break;

		default:

			NM_ASSERT(false, "Texture::getTotalRows: animation scheme not handled");
			break;
	}

	return 0;
}

/**
 * Retrieve map's total row size, accounting for the total frames of animation
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of total rows
 */
u32 Texture::getTotalRows()
{
	// determine the allocation type
	switch(this->textureDefinition->charSetDefinition->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:

			// just return the cols
			return this->textureDefinition->rows;
			break;

		case __ANIMATED_MULTI:
			{
				// return the total number of chars
				return this->textureDefinition->rows + this->textureDefinition->rows * (Texture::getTotalCols(this) >> 6);
			}
			break;

		case __NOT_ANIMATED:

			// just return the cols
			return this->textureDefinition->rows;
			break;

		default:

			NM_ASSERT(false, "Texture::getTotalRows: animation scheme not handled");
			break;
	}

	return 0;
}

/**
 * Retrieve number of frames for animation
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of frames for animation
 */
u32 Texture::getNumberOfFrames()
{
	return this->textureDefinition->numberOfFrames;
}

/**
 * Retrieve CharSet
 *
 * @memberof				Texture
 * @public
 *
 * @param this				Function scope
 * @param loadIfNeeded		Flag to force loading if CharSet is NULL
 *
 * @return					CharSet
 */
CharSet Texture::getCharSet(u32 loadIfNeeded)
{
	if(!this->charSet && loadIfNeeded)
	{
		Texture::loadCharSet(this);
	}

	return this->charSet;
}

/**
 * Retrieve map definition
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Pointer to the map definition
 */
BYTE* Texture::getMapDefinition()
{
	return this->textureDefinition ? this->textureDefinition->mapDefinition : NULL;
}

/**
 * Set palette
 *
 * @memberof			Texture
 * @public
 *
 * @param this			Function scope
 * @param palette		New palette
 */
void Texture::setPalette(u8 palette)
{
	this->palette = palette;
}

/**
 * Retrieve palette
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Palette
 */
u8 Texture::getPalette()
{
	return this->palette;
}

/**
 * Retrieve map's row size
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of rows
 */
u32 Texture::getRows()
{	//ASSERT(this->textureDefinition, "Texture::getRows: 0 rows");

	return this->textureDefinition->rows;
}

/**
 * Retrieve map's column size
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of columns
 */
u32 Texture::getCols()
{
	return this->textureDefinition->cols;
}

/**
 * Retrieve identification
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			Identification number
 */
u16 Texture::getId()
{
	return this->id;
}

/**
 * Event listener for CharSet re-writing to DRAM
 *
 * @memberof				Texture
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		CharSet
 */
void Texture::onCharSetRewritten(Object eventFirer __attribute__ ((unused)))
{
	 Texture::rewrite(this);

	// propagate event
	Object::fireEvent(this, kEventTextureRewritten);
}

/**
 * Event listener for CharSet deletion
 *
 * @memberof				Texture
 * @private
 *
 * @param this				Function scope
 * @param eventFirer		CharSet
 */
void Texture::onCharSetDeleted(Object eventFirer)
{
	this->charSet = CharSet::safeCast(eventFirer) == this->charSet ? NULL : this->charSet;
}

/**
 * Write a single CHAR to DRAM
 *
 * @memberof				Texture
 * @public
 *
 * @param this				Function scope
 * @param texturePixel		Coordinates within the map definition to write
 * @param newChar			CHAR data to write
 */
void Texture::putChar(Point* texturePixel, BYTE* newChar)
{
	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u32 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureDefinition->mapDefinition[displacement];
		CharSet::putChar(this->charSet, charToReplace, newChar);
	}
}

/**
 * Write a single pixel to DRAM
 *
 * @memberof				Texture
 * @public
 *
 * @param this				Function scope
 * @param texturePixel		Point that defines the position of the char in the Sprite's texture
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void Texture::putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor)
{
	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u32 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureDefinition->mapDefinition[displacement];
		CharSet::putPixel(this->charSet, charToReplace, charSetPixel, newPixelColor);
	}
}

/**
 * Check if writing to DRAM is done
 *
 * @memberof		Texture
 * @public
 *
 * @param this		Function scope
 *
 * @return			True if completely written to DRAM
 */
bool Texture::isWritten()
{
	return this->written;
}

/**
 * Set displacement to add to the offset within the BGMAP memory
 *
 * @memberof								Texture
 * @public
 *
 * @param this								Function scope
 * @param mapDefinitionDisplacement			Displacement
 */
void Texture::setMapDisplacement(u32 mapDisplacement)
{
	this->mapDisplacement = mapDisplacement;
}
