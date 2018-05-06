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
__CLASS_DEFINITION(Texture, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Texture_onCharSetRewritten(Texture this, Object eventFirer);
static void Texture_onCharSetDeleted(Texture this, Object eventFirer);
static void Texture_loadCharSet(Texture this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Texture, TextureDefinition* textureDefinition, u16 id)
__CLASS_NEW_END(Texture, textureDefinition, id);

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
void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id)
{
	ASSERT(this, "Texture::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

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
void Texture_destructor(Texture this)
{
	ASSERT(this, "Texture::destructor: null this");

	Texture_releaseCharSet(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base_destructor();
}

/**
 * Load the CharSet defined by the TextureDefinition
 *
 * @memberof	Texture
 * @private
 *
 * @param this	Function scope
 */
static void Texture_loadCharSet(Texture this)
{
	ASSERT(this, "Texture::getCharSet: null this");

	Texture_releaseCharSet(this);

	this->charSet = CharSetManager_getCharSet(CharSetManager_getInstance(), this->textureDefinition->charSetDefinition);
	ASSERT(this->charSet, "Texture::constructor: null charSet");
	// if the char definition is NULL, it must be a text
	Object_addEventListener(__SAFE_CAST(Object, this->charSet), __SAFE_CAST(Object, this), (EventListener)Texture_onCharSetRewritten, kEventCharSetRewritten);
	Object_addEventListener(__SAFE_CAST(Object, this->charSet), __SAFE_CAST(Object, this), (EventListener)Texture_onCharSetDeleted, kEventCharSetDeleted);
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
void Texture_setDefinition(Texture this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "Texture::setDefinition: null this");
	ASSERT(textureDefinition, "Texture::setDefinition: null textureDefinition");

	this->textureDefinition = textureDefinition;

	Texture_releaseCharSet(this);
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
TextureDefinition* Texture_getDefinition(Texture this)
{
	ASSERT(this, "Texture::getDefinition: null this");

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
void Texture_releaseCharSet(Texture this)
{
	ASSERT(this, "Texture::releaseCharSet: null this");

	if(this->charSet)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->charSet), __SAFE_CAST(Object, this), (EventListener)Texture_onCharSetRewritten, kEventCharSetRewritten);
		Object_removeEventListener(__SAFE_CAST(Object, this->charSet), __SAFE_CAST(Object, this), (EventListener)Texture_onCharSetDeleted, kEventCharSetDeleted);

		CharSetManager_releaseCharSet(CharSetManager_getInstance(), this->charSet);

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
void Texture_write(Texture this)
{
	ASSERT(this, "Texture::write: null this");
	ASSERT(this->textureDefinition, "Texture::write: null textureDefinition");
	ASSERT(this->textureDefinition->charSetDefinition, "Texture::write: null charSetDefinition");

	if(!this->charSet)
	{
		Texture_loadCharSet(this);
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
void Texture_rewrite(Texture this)
{
	ASSERT(this, "Texture::rewrite: null this");

	this->written = false;

	 Texture_write(this);
}

/**
 * Write to DRAM in h-bias mode
 *
 * @memberof					Texture
 * @public
 *
 * @param this					Function scope
 */
void Texture_writeHBiasMode(Texture this __attribute__ ((unused)))
{
	ASSERT(this, "Texture::writeHBiasMode: null this");

	// TODO
	/*
	int i;
	//put the this into memory calculation the number of char for each reference
	for(i=0;i<this->textureDefinition->rows;i++)
	{
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureDefinition->mapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet_getCharSet(&this->charSet)<<9)+CharSet_getOffset(&this->charSet)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+64* const this->yOffset+64*i)<<1), this->textureDefinition->mapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->palette<<14)|((CharSet_getCharSet(&this->charSet)<<9)+CharSet_getOffset(&this->charSet)));
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
int Texture_getNumberOfChars(Texture this)
{
	ASSERT(this, "Texture::getNumberOfChars: null this");

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
TextureDefinition* Texture_getTextureDefinition(Texture this)
{
	ASSERT(this, "Texture::getTextureDefinition: null this");

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
u32 Texture_getTotalCols(Texture this)
{
	ASSERT(this, "Texture::getTotalCols: null this");

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
u32 Texture_getTotalRows(Texture this)
{
	ASSERT(this, "Texture::getTotalRows: null this");

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
				return this->textureDefinition->rows + this->textureDefinition->rows * (Texture_getTotalCols(this) >> 6);
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
u32 Texture_getNumberOfFrames(Texture this)
{
	ASSERT(this, "Texture::getNumberOfFrames: null this");

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
CharSet Texture_getCharSet(Texture this, u32 loadIfNeeded)
{
	ASSERT(this, "Texture::getCharSet: null this");

	if(!this->charSet && loadIfNeeded)
	{
		Texture_loadCharSet(this);
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
BYTE* Texture_getMapDefinition(Texture this)
{
	ASSERT(this, "Texture::getBgmapDef: null this");

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
void Texture_setPalette(Texture this, u8 palette)
{
	ASSERT(this, "Texture::setPalette: null this");

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
u8 Texture_getPalette(Texture this)
{
	ASSERT(this, "Texture::getPalette: null this");

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
u32 Texture_getRows(Texture this)
{
	ASSERT(this, "Texture::getRows: null this");
	//ASSERT(this->textureDefinition, "Texture::getRows: 0 rows");

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
u32 Texture_getCols(Texture this)
{
	ASSERT(this, "Texture::getCols: null this");

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
u16 Texture_getId(Texture this)
{
	ASSERT(this, "Texture::getId: null this");

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
static void Texture_onCharSetRewritten(Texture this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Texture::onCharSetRewritten: null this");

	 Texture_rewrite(this);

	// propagate event
	Object_fireEvent(__SAFE_CAST(Object, this), kEventTextureRewritten);
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
static void Texture_onCharSetDeleted(Texture this, Object eventFirer)
{
	ASSERT(this, "Texture::onCharSetRewritten: null this");

	this->charSet = __SAFE_CAST(CharSet, eventFirer) == this->charSet ? NULL : this->charSet;
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
void Texture_putChar(Texture this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Texture::putChar: null this");

	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u32 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureDefinition->mapDefinition[displacement];
		CharSet_putChar(this->charSet, charToReplace, newChar);
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
void Texture_putPixel(Texture this, Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Texture::putPixel: null this");

	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureDefinition->cols && ((unsigned)texturePixel->y) < this->textureDefinition->rows)
	{
		u32 displacement = (this->textureDefinition->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureDefinition->mapDefinition[displacement];
		CharSet_putPixel(this->charSet, charToReplace, charSetPixel, newPixelColor);
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
bool Texture_isWritten(Texture this)
{
	ASSERT(this, "Texture::isWritten: null this");

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
void Texture_setMapDisplacement(Texture this, u32 mapDisplacement)
{
	ASSERT(this, "Texture::setMapDisplacement: null this");

	this->mapDisplacement = mapDisplacement;
}
