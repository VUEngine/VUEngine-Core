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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSetManager.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param textureSpec		Spec to use
 * @param id					Texture's identification
 */
void Texture::constructor(TextureSpec* textureSpec, u16 id)
{
	// construct base object
	Base::constructor();

	// set id
	this->id = id;

	this->mapDisplacement = 0;
	this->usageCount = 1;

	// save the bgmap spec's address
	this->textureSpec = textureSpec;
	this->charSet = NULL;
	// set the palette
	this->palette = textureSpec->palette;
	this->status = kTexturePendingWriting;
	this->frame = 0;
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
u8 Texture::getUsageCount()
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
	if(this->usageCount)
	{
		this->usageCount--;
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

	this->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), this->textureSpec->charSetSpec);
	ASSERT(this->charSet, "Texture::constructor: null charSet");

	this->status = kTexturePendingWriting;

	Object::addEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture::onCharSetRewritten, kEventCharSetRewritten);
	Object::addEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture::onCharSetDeleted, kEventCharSetDeleted);
}

/**
 * Set the TextureSpec
 *
 * @param textureSpec		New TextureSpec
 */
void Texture::setSpec(TextureSpec* textureSpec)
{
	ASSERT(textureSpec, "Texture::setSpec: null textureSpec");

	if(0 == this->usageCount || NULL == this->textureSpec || isDeleted(this->charSet))
	{
		// Since the texture cannot be used without a spec or a charSet
		// it is safe to assume that it is not being displayed and can be written
		// outside XPEND
		this->status = kTexturePendingWriting;
	}
	else if(this->textureSpec != textureSpec)
	{
		Texture::releaseCharSet(this);

		// Since the texture
		this->status = kTextureSpecChanged;
	}

	this->textureSpec = textureSpec;
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
	if(!isDeleted(this->charSet))
	{
		Object::removeEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture::onCharSetRewritten, kEventCharSetRewritten);
		Object::removeEventListener(this->charSet, Object::safeCast(this), (EventListener)Texture::onCharSetDeleted, kEventCharSetDeleted);

		CharSetManager::releaseCharSet(CharSetManager::getInstance(), this->charSet);

		this->charSet = NULL;
	}

	this->status = kTexturePendingWriting;
}

/**
 * Write the map to DRAM
 */
bool Texture::write()
{
	ASSERT(this->textureSpec, "Texture::write: null textureSpec");
	ASSERT(this->textureSpec->charSetSpec, "Texture::write: null charSetSpec");

	if(isDeleted(this->charSet))
	{
		Texture::loadCharSet(this);
	}

	if(isDeleted(this->charSet))
	{
		this->status = kTextureInvalid;
		return false;
	}

	this->status = kTextureWritten;
	return true;
}

bool Texture::prepare()
{
	switch(this->status)
	{
		case kTexturePendingWriting:

			// Non written textures can be written at any time 
			// since they are not visible yet
			Texture::write(this);
			break;

		case kTexturePendingRewriting:

			SpriteManager::updateTexture(SpriteManager::getInstance(), this);
			return true;
			break;

		case kTextureFrameChanged:

			if(isDeleted(this->charSet))
			{
				this->status = kTextureInvalid;
			}
			else
			{
				return true;
			}

			break;

		case kTextureSpecChanged:

			Texture::loadCharSet(this);
			Texture::update(this);
			break;
	}

	return kTextureWritten == this->status;
}

void Texture::update()
{
	switch(this->status)
	{
		case kTexturePendingRewriting:

			Texture::write(this);

			if(kTextureWritten == this->status)
			{
				Texture::fireEvent(this, kEventTextureRewritten);
			
				NM_ASSERT(!isDeleted(this), "Texture::prepare: deleted this during kEventTextureRewritten");
			}
			return;
			break;

		default:

			if(isDeleted(this->charSet))
			{
				Texture::write(this);
			}
			else
			{
				// write according to the allocation type
				switch(CharSet::getAllocationType(this->charSet))
				{
					case __ANIMATED_SINGLE_OPTIMIZED:

						CharSet::setFrame(this->charSet, this->frame);
						Texture::setMapDisplacement(this, this->textureSpec->cols * this->textureSpec->rows * this->frame);
						Texture::write(this);
						break;

					case __NOT_ANIMATED:
					case __ANIMATED_SINGLE:
					case __ANIMATED_SHARED:
					case __ANIMATED_SHARED_COORDINATED:

						CharSet::setFrame(this->charSet, this->frame);
						this->status = kTextureWritten;
						break;

					case __ANIMATED_MULTI:

						Texture::setFrameAnimatedMulti(this, this->frame);
						this->status = kTextureWritten;
						break;
				}
			}

			break;
	}
}

/**
 * Rewrite the map to DRAM
 */
void Texture::rewrite()
{
	this->status = this->status > kTexturePendingRewriting ? kTexturePendingRewriting : this->status;
}

/**
 * Write to DRAM in h-bias mode
 */
void Texture::writeHBiasMode()
{
	// TODO
	/*
	int i;
	//put the this into memory calculation the number of char for each reference
	for(i=0;i<this->textureSpec->rows;i++)
	{
		//write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureSpec->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureSpec->mapSpec+(i<<7), (this->textureSpec->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureSpec->cols/3+64* const this->yOffset+64*i)<<1), this->textureSpec->mapSpec+(i<<7), (this->textureSpec->cols/3)*2,(this->palette<<14)|((CharSet::getCharSet(&this->charSet)<<9)+CharSet::getOffset(&this->charSet)));
	}
	*/
}

/**
 * Retrieve the number of CHARs according to the TextureSpec's CharSpec
 *
 * @return	Number of CHARs
 */
int Texture::getNumberOfChars()
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
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void Texture::setFrame(u16 frame)
{
	if(frame == this->frame)
	{
		return;
	}

	this->frame = frame;
	this->status = this->status > kTextureFrameChanged ? kTextureFrameChanged : this->status;

	if(kTextureFrameChanged == this->status)
	{
		SpriteManager::updateTexture(SpriteManager::getInstance(), this);
	}
}


/**
 * Get Texture's frame
 *
 * @return 	Texture's frame to display
 */
u16 Texture::getFrame()
{
	return this->frame;
}

/**
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void Texture::setFrameAnimatedMulti(u16 frame __attribute__ ((unused)))
{
}

/**
 * Retrieve map's total column size, accounting for the total frames of animation
 *
 * @return	Number of total columns
 */
u32 Texture::getTotalCols()
{
	// determine the allocation type
	switch(this->textureSpec->charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:

			// just return the cols
			return this->textureSpec->cols;
			break;

		case __ANIMATED_MULTI:
			{
				// return the total number of chars
				int totalCols = this->textureSpec->numberOfFrames * this->textureSpec->cols;
				return 64 >= totalCols ? totalCols : 64;
			}
			break;

		case __NOT_ANIMATED:

			// just return the cols
			return this->textureSpec->cols;
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
 * @return	Number of total rows
 */
u32 Texture::getTotalRows()
{
	// determine the allocation type
	switch(this->textureSpec->charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:

			// just return the cols
			return this->textureSpec->rows;
			break;

		case __ANIMATED_MULTI:
			{
				// return the total number of chars
				return this->textureSpec->rows + this->textureSpec->rows * (Texture::getTotalCols(this) >> 6);
			}
			break;

		case __NOT_ANIMATED:

			// just return the cols
			return this->textureSpec->rows;
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
 * @return	Number of frames for animation
 */
u32 Texture::getNumberOfFrames()
{
	return this->textureSpec->numberOfFrames;
}

/**
 * Retrieve CharSet
 *
 * @param loadIfNeeded	Flag to force loading if CharSet is NULL
 * @return				CharSet
 */
CharSet Texture::getCharSet(u32 loadIfNeeded)
{
	if(isDeleted(this->charSet) && loadIfNeeded)
	{
		Texture::loadCharSet(this);
	}

	return this->charSet;
}

/**
 * Retrieve map spec
 *
 * @return	Pointer to the map spec
 */
BYTE* Texture::getMapSpec()
{
	return this->textureSpec ? this->textureSpec->mapSpec : NULL;
}

/**
 * Set palette
 *
 * @param palette	New palette
 */
void Texture::setPalette(u8 palette)
{
	this->palette = palette;
}

/**
 * Retrieve palette
 *
 * @return	Palette
 */
u8 Texture::getPalette()
{
	return this->palette;
}

/**
 * Retrieve map's row size
 *
 * @return	Number of rows
 */
u32 Texture::getRows()
{
	//ASSERT(this->textureSpec, "Texture::getRows: 0 rows");

	return this->textureSpec->rows;
}

/**
 * Retrieve map's column size
 *
 * @return	Number of columns
 */
u32 Texture::getCols()
{
	return this->textureSpec->cols;
}

/**
 * Retrieve identification
 *
 * @return	Identification number
 */
u16 Texture::getId()
{
	return this->id;
}

/**
 * Event listener for CharSet re-writing to DRAM
 *
 * @private
 * @param eventFirer	CharSet
 */
void Texture::onCharSetRewritten(Object eventFirer __attribute__ ((unused)))
{
	Texture::rewrite(this);
}

/**
 * Event listener for CharSet deletion
 *
 * @private
 * @param eventFirer	CharSet
 */
void Texture::onCharSetDeleted(Object eventFirer)
{
	this->charSet = CharSet::safeCast(eventFirer) == this->charSet ? NULL : this->charSet;
}

/**
 * Write a single CHAR to DRAM
 *
 * @param texturePixel	Coordinates within the map spec to write
 * @param newChar		CHAR data to write
 */
void Texture::putChar(Point* texturePixel, BYTE* newChar)
{
	if(this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureSpec->cols && ((unsigned)texturePixel->y) < this->textureSpec->rows)
	{
		u32 displacement = (this->textureSpec->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureSpec->mapSpec[displacement];
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
		u32 displacement = (this->textureSpec->cols * texturePixel->y + texturePixel->x) << 1;
		u32 charToReplace = this->textureSpec->mapSpec[displacement];
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

/**
 * Set displacement to add to the offset within the BGMAP memory
 *
 * @param mapSpecDisplacement	Displacement
 */
void Texture::setMapDisplacement(u32 mapDisplacement)
{
	this->status = this->mapDisplacement != mapDisplacement ? kTexturePendingWriting : this->status;

	this->mapDisplacement = mapDisplacement;
}
