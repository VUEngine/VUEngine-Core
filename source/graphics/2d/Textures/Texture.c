/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSetManager.h>
#include <DebugUtilities.h>
#include <Optics.h>
#include <SpriteManager.h>

#include "Texture.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

friend class CharSet;


/**
 * Class constructor
 *
 * @private
 * @param textureSpec		Spec to use
 * @param id					Texture's identification
 */
void Texture::constructor(TextureSpec* textureSpec, uint16 id)
{
	// construct base object
	Base::constructor();

	// set id
	this->id = id;

	this->doUpdate = NULL;

	this->mapDisplacement = 0;
	this->usageCount = 1;

	// save the bgmap spec's address
	this->textureSpec = textureSpec;
	this->charSet = NULL;
	// set the palette
	this->palette = textureSpec->palette;
	this->status = kTexturePendingWriting;
	this->frame = 0;
	this->update = false;
}

/**
 * Class destructor
 */
void Texture::destructor()
{
	// make sure that I'm not destroyed again
	this->usageCount = 0;

	Texture::releaseCharSet(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Retrieve the count usage for this Texture
 *
 * @return				Texture's count usage
 */
uint8 Texture::getUsageCount()
{
	return this->usageCount;
}

/**
 * Increase the count usage for this Texture
 */
void Texture::increaseUsageCount()
{
	this->usageCount++;
}

/**
 * Decrease the count usage for this Texture
 *
 * @return				True if count usage reached zero
 */
bool Texture::decreaseUsageCount()
{
	if(0 >= --this->usageCount)
	{
		this->usageCount = 0;
	}

	return 0 == this->usageCount;
}

/**
 * Load the CharSet defined by the TextureSpec
 *
 * @private
 */
void Texture::loadCharSet()
{
	Texture::releaseCharSet(this);

	if(NULL == this->textureSpec->charSetSpec)
	{
		return;
	}

	this->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), this->textureSpec->charSetSpec);

	if(isDeleted(this->charSet))
	{
		return;
	}

	this->status = kTexturePendingWriting;

	Texture::setupUpdateFunction(this);

	CharSet::addEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetRewritten, kEventCharSetRewritten);
	CharSet::addEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetDeleted, kEventCharSetDeleted);
}

/**
 * Set the TextureSpec
 *
 * @param textureSpec		New TextureSpec
 */
void Texture::setSpec(TextureSpec* textureSpec)
{
	ASSERT(textureSpec, "Texture::setSpec: null textureSpec");

	if(NULL == textureSpec)
	{
		return;
	}

	if(this->textureSpec != textureSpec)
	{
		if(NULL != this->charSet && textureSpec->charSetSpec != CharSet::getCharSetSpec(this->charSet))
		{
			Texture::releaseCharSet(this);
		}

		this->textureSpec = textureSpec;
		this->frame = 0;
		this->mapDisplacement = 0;
		this->palette = this->textureSpec->palette;
		this->status = kTexturePendingWriting;
	}
}

/**
 * Retrieve the TextureSpec
 *
 * @return				TextureSpec
 */
TextureSpec* Texture::getSpec()
{
	return this->textureSpec;
}

/**
 * Release the CharSet
 */
void Texture::releaseCharSet()
{
	this->update = false;
	this->status = kTexturePendingWriting;

	if(!isDeleted(this->charSet))
	{
		CharSet::removeEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetRewritten, kEventCharSetRewritten);
		CharSet::removeEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetDeleted, kEventCharSetDeleted);

		CharSetManager::releaseCharSet(CharSetManager::getInstance(), this->charSet);

		this->charSet = NULL;
	}

	this->frame = 0;
	this->mapDisplacement = 0;
	this->palette = this->textureSpec->palette;
}

bool Texture::isReady()
{
	return kTextureWritten == this->status;
}

/**
 * Write the map to DRAM
 */
bool Texture::write(int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	ASSERT(this->textureSpec, "Texture::write: null textureSpec");
	ASSERT(this->textureSpec->charSetSpec, "Texture::write: null charSetSpec");

	if(isDeleted(this->charSet))
	{
		Texture::loadCharSet(this);

		if(isDeleted(this->charSet))
		{
			this->status = kTextureInvalid;
			return false;
		}
	}

	CharSet::setFrame(this->charSet, this->frame);

	this->status = kTextureWritten;
	return true;
}

void Texture::prepare()
{
	switch(this->status)
	{
		case kTexturePendingWriting:

			this->update = true;
			break;

		case kTexturePendingRewriting:

			this->update = true;
			break;

		case kTextureFrameChanged:
		case kTextureMapDisplacementChanged:

			if(isDeleted(this->charSet))
			{
				Texture::loadCharSet(this);
			}

			if(isDeleted(this->charSet))
			{
				this->status = kTextureInvalid;
			}
			else
			{
				this->update = true;
			}

			break;
	}
}

bool Texture::update(int16 maximumTextureRowsToWrite)
{
	switch(this->status)
	{
		case kTexturePendingWriting:

			Texture::write(this, maximumTextureRowsToWrite);
			break;

		case kTexturePendingRewriting:

			Texture::write(this, maximumTextureRowsToWrite);

			if(kTextureWritten == this->status)
			{
				Texture::fireEvent(this, kEventTextureRewritten);
			
				NM_ASSERT(!isDeleted(this), "Texture::prepare: deleted this during kEventTextureRewritten");
			}

			break;

		case kTextureMapDisplacementChanged:

			Texture::write(this, maximumTextureRowsToWrite);

			if(kTextureWritten != this->status)
			{
				break;
			}

			Texture::fireEvent(this, kEventTextureRewritten);			
			NM_ASSERT(!isDeleted(this), "Texture::prepare: deleted this during kEventTextureRewritten");

			// Intended fall through

		default:

			if(isDeleted(this->charSet))
			{
				Texture::write(this, maximumTextureRowsToWrite);
			}
			else if(NULL != this->doUpdate)
			{
				this->doUpdate(this, maximumTextureRowsToWrite);
			}
			else
			{
				this->status = kTextureWritten;
			}

			break;
	}

	return kTextureWritten == this->status;
}

static void Texture::updateOptimized(Texture texture, int16 maximumTextureRowsToWrite)
{
	if(isDeleted(texture->charSet))
	{
		return;
	}

	CharSet::setFrame(texture->charSet, texture->frame);

	Texture::write(texture, maximumTextureRowsToWrite);
}

static void Texture::updateDefault(Texture texture, int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	if(isDeleted(texture->charSet))
	{
		return;
	}

	CharSet::setFrame(texture->charSet, texture->frame);

	texture->status = kTextureWritten;
}

static void Texture::updateMulti(Texture texture, int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	texture->status = kTextureWritten;
}

bool Texture::isShared()
{
	if(!isDeleted(this->charSet))
	{
		return CharSet::isShared(this->charSet);
	}

	if(NULL != this->textureSpec->charSetSpec)
	{
		return this->textureSpec->charSetSpec->shared;
	}

	return true;
}

static bool Texture::isSpecSingleFrame(const TextureSpec* textureSpec)
{
	if(NULL == textureSpec)
	{
		return false;		
	}

	return 1 == textureSpec->numberOfFrames;
}

bool Texture::isSingleFrame()
{
	return Texture::isSpecSingleFrame(this->textureSpec);
}

bool Texture::isMultiframe()
{
	return !Texture::isSpecSingleFrame(this->textureSpec);
}

/**
 * Rewrite the map to DRAM
 */
void Texture::rewrite()
{
	bool statusChanged = kTexturePendingRewriting != this->status || !this->update;

	this->status = this->status > kTexturePendingRewriting ? kTexturePendingRewriting : this->status;

	if(statusChanged && kTexturePendingRewriting == this->status)
	{
		// Prepare the texture right away just in case the call initiates
		// at a defragmentation process
		Texture::prepare(this);
	}
}

/**
 * Write to DRAM in h-bias mode
 */
void Texture::writeHBiasMode()
{
	// TODO
	/*
	int32 i;
	//put the this into memory calculation the number of char for each reference
	for(i=0;i<this->textureSpec->rows;i++)
	{
		//write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureSpec->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureSpec->map<7), (this->textureSpec->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureSpec->cols/3+64* const this->yOffset+64*i)<<1), this->textureSpec->map<7), (this->textureSpec->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
	}
	*/
}

/**
 * Retrieve the number of CHARs according to the TextureSpec's CharSpec
 *
 * @return	Number of CHARs
 */
int32 Texture::getNumberOfChars()
{
	return this->textureSpec->charSetSpec->numberOfChars;
}

/**
 * Retrieve the TextureSpec
 *
 * @return	TextureSpec
 */
TextureSpec* Texture::getTextureSpec()
{
	return this->textureSpec;
}

/**
 * Set displacement to add to the offset within the BGMAP memory
 *
 * @param mapDisplacement	Displacement
 */
void Texture::setMapDisplacement(uint32 mapDisplacement)
{
	bool statusChanged = kTextureMapDisplacementChanged != this->status || !this->update;

	this->status = this->mapDisplacement != mapDisplacement && this->status > kTextureMapDisplacementChanged ? kTextureMapDisplacementChanged : this->status;

	this->mapDisplacement = mapDisplacement;

	if(statusChanged && kTextureMapDisplacementChanged == this->status)
	{
		Texture::prepare(this);
	}
}

/**
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void Texture::setFrame(uint16 frame)
{
	if(frame == this->frame && this->status < kTextureFrameChanged)
	{
		return;
	}

	this->frame = frame;

	bool statusChanged = kTextureFrameChanged != this->status || !this->update;

	this->status = this->status > kTextureFrameChanged ? kTextureFrameChanged : this->status;

	if(!isDeleted(this->charSet))
	{
		if(statusChanged && kTextureFrameChanged == this->status)
		{
			if(CharSet::isOptimized(this->charSet))
			{
				this->mapDisplacement = this->textureSpec->cols * this->textureSpec->rows * this->frame;
			}

			Texture::prepare(this);
		}
	}
}

/**
 * Get Texture's frame
 *
 * @return 	Texture's frame to display
 */
uint16 Texture::getFrame()
{
	return this->frame;
}

/**
 * Retrieve map's total column size, accounting for the total frames of animation
 *
 * @return	Number of total columns
 */
static uint32 Texture::getTotalCols(TextureSpec* textureSpec)
{
	if(NULL == textureSpec->charSetSpec)
	{
		return 0;
	}

	if(!Texture::isSpecSingleFrame(textureSpec))
	{
		uint32 maximumNumberOfFrames = 64 / textureSpec->cols;

		if(maximumNumberOfFrames > textureSpec->numberOfFrames)
		{
			return textureSpec->numberOfFrames * textureSpec->cols;
		}
		
		return maximumNumberOfFrames * textureSpec->cols;	
	}

	return textureSpec->cols;
}

/**
 * Retrieve map's total row size, accounting for the total frames of animation
 *
 * @return	Number of total rows
 */
static uint32 Texture::getTotalRows(TextureSpec* textureSpec)
{
	if(NULL == textureSpec->charSetSpec)
	{
		return 0;
	}

	if(!Texture::isSpecSingleFrame(textureSpec))
	{
		uint32 allocableCols = Texture::getTotalCols(textureSpec);
		int32 remainingCols = textureSpec->numberOfFrames * textureSpec->cols - allocableCols;
		
		// return the total number of chars
		return textureSpec->rows + textureSpec->rows * (remainingCols / 64);
	}

	return textureSpec->rows;
}

/**
 * Retrieve number of frames for animation
 *
 * @return	Number of frames for animation
 */
uint32 Texture::getNumberOfFrames()
{
	return this->textureSpec->numberOfFrames;
}

/**
 * Retrieve CharSet
 *
 * @param loadIfNeeded	Flag to force loading if CharSet is NULL
 * @return				CharSet
 */
CharSet Texture::getCharSet(uint32 loadIfNeeded)
{
	if(isDeleted(this->charSet) && loadIfNeeded)
	{
		Texture::loadCharSet(this);
	}

	return this->charSet;
}

/**
 * Set CharSet
 *
 * @param charset	CharSet
 */
void Texture::setCharSet(CharSet charSet)
{
	Texture::releaseCharSet(this);

	this->charSet = charSet;

	if(NULL == this->charSet)
	{
		return;
	}	

	this->status = kTexturePendingWriting;

	CharSet::addEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetRewritten, kEventCharSetRewritten);
	CharSet::addEventListener(this->charSet, ListenerObject::safeCast(this), (EventListener)Texture::onCharSetDeleted, kEventCharSetDeleted);

	Texture::setupUpdateFunction(this);

	Texture::rewrite(this);
}

void Texture::setupUpdateFunction()
{
	if(Texture::isMultiframe(this))
	{
		this->doUpdate = NULL;
		//this->doUpdate = Texture::updateMulti;
	}
	else if(CharSet::isOptimized(this->charSet))
	{
		this->doUpdate = Texture::updateOptimized;
		CharSet::setFrame(this->charSet, this->frame);
	}
	else
	{			
		this->doUpdate = Texture::updateDefault;
		CharSet::setFrame(this->charSet, this->frame);
	}
}

/**
 * Retrieve map spec
 *
 * @return	Pointer to the map spec
 */
uint16* Texture::getMap()
{
	return this->textureSpec ? this->textureSpec->map : NULL;
}

/**
 * Set palette
 *
 * @param palette	New palette
 */
void Texture::setPalette(uint8 palette)
{
	this->palette = palette;

	Texture::rewrite(this);
}

/**
 * Retrieve palette
 *
 * @return	Palette
 */
uint8 Texture::getPalette()
{
	return this->palette;
}

/**
 * Retrieve map's row size
 *
 * @return	Number of rows
 */
uint32 Texture::getRows()
{
	//ASSERT(this->textureSpec, "Texture::getRows: 0 rows");

	return this->textureSpec->rows;
}

/**
 * Retrieve map's column size
 *
 * @return	Number of columns
 */
uint32 Texture::getCols()
{
	return this->textureSpec->cols;
}

/**
 * Retrieve identification
 *
 * @return	Identification number
 */
uint16 Texture::getId()
{
	return this->id;
}

/**
 * Event listener for CharSet re-writing to DRAM
 *
 * @private
 * @param eventFirer	CharSet
 */
void Texture::onCharSetRewritten(ListenerObject eventFirer __attribute__ ((unused)))
{
	Texture::rewrite(this);
}

/**
 * Event listener for CharSet deletion
 *
 * @private
 * @param eventFirer	CharSet
 */
void Texture::onCharSetDeleted(ListenerObject eventFirer)
{
	this->charSet = CharSet::safeCast(eventFirer) == this->charSet ? NULL : this->charSet;
}

/**
 * Write a single CHAR to DRAM
 *
 * @param texturePixel	Coordinates within the map spec to write
 * @param newChar		CHAR data to write
 */
void Texture::putChar(Point* texturePixel, uint32* newChar)
{
	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureSpec->cols && ((unsigned)texturePixel->y) < this->textureSpec->rows)
	{
		uint32 displacement = this->textureSpec->cols * texturePixel->y + texturePixel->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;

		CharSet::putChar(this->charSet, charToReplace, newChar);
	}
}

/**
 * Write a single pixel to DRAM
 *
 * @param texturePixel	Point that defines the position of the char in the Sprite's texture
 * @param charSetPixel	Pixel data
 * @param newPixelColor	Color value of pixel
 */
void Texture::putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor)
{
	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureSpec->cols && ((unsigned)texturePixel->y) < this->textureSpec->rows)
	{
		uint32 displacement = this->textureSpec->cols * texturePixel->y + texturePixel->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;
		CharSet::putPixel(this->charSet, charToReplace, charSetPixel, newPixelColor);
	}
}

/**
 * Check if writing to DRAM is done
 *
 * @return	True if completely written to DRAM
 */
bool Texture::isWritten()
{
	return this->status;
}
