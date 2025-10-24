/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSetManager.h>
#include <BgmapTextureManager.h>
#include <ObjectTextureManager.h>
#include <Optics.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "Texture.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class CharSet;
friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::reset()
{
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());
	ObjectTextureManager::reset(ObjectTextureManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Texture Texture::get
(
	ClassPointer textureClass, const TextureSpec* textureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	if(NULL == textureSpec)
	{
		return NULL;
	}

	if(typeofclass(BgmapTexture) == textureClass)
	{
		return 
			Texture::safeCast
			(
				BgmapTextureManager::getTexture
				(
					BgmapTextureManager::getInstance(), (BgmapTextureSpec*)textureSpec, minimumSegment, mustLiveAtEvenSegment, scValue
				)
			);
	}
	else if(typeofclass(ObjectTexture) == textureClass)
	{
		return Texture::safeCast(ObjectTextureManager::getTexture(ObjectTextureManager::getInstance(), (ObjectTextureSpec*)textureSpec));
	}

	return NULL;	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::release(Texture texture)
{
	if(isDeleted(texture))
	{
		return;
	}

	if(__IS_INSTANCE_OF(BgmapTexture, texture))
	{
		BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), BgmapTexture::safeCast(texture));
	}
	else if(__IS_INSTANCE_OF(ObjectTexture, texture))
	{
		ObjectTextureManager::releaseTexture(ObjectTextureManager::getInstance(), ObjectTexture::safeCast(texture));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::updateTextures(int16 maximumTextureRowsToWrite, bool defer)
{
	BgmapTextureManager::updateTextures(BgmapTextureManager::getInstance(), maximumTextureRowsToWrite, defer);
	ObjectTextureManager::updateTextures(ObjectTextureManager::getInstance(), maximumTextureRowsToWrite, defer);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 Texture::getTotalRows(TextureSpec* textureSpec)
{
	if(NULL == textureSpec->charSetSpec)
	{
		return 0;
	}

	if(!Texture::isSpecSingleFrame(textureSpec))
	{
		int16 allocableFrames = 64 / textureSpec->cols;
		int16 neededRows = 
			__FIXED_TO_I(__FIXED_DIV(__I_TO_FIXED(textureSpec->numberOfFrames), __I_TO_FIXED(allocableFrames)) + __05F_FIXED) - 1;

		// Return the total number of chars
		return textureSpec->rows + textureSpec->rows * (0 < neededRows ? neededRows : 0);
	}

	return textureSpec->rows;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool Texture::isSpecSingleFrame(const TextureSpec* textureSpec)
{
	if(NULL == textureSpec)
	{
		return false;		
	}

	return 1 == textureSpec->numberOfFrames;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::updateOptimized(Texture texture, int16 maximumTextureRowsToWrite)
{
	Texture::write(texture, maximumTextureRowsToWrite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::updateDefault(Texture texture, int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	if(isDeleted(texture->charSet))
	{
		return;
	}

	texture->generation = CharSet::write(texture->charSet);

	texture->status = kTextureWritten;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Texture::updateMulti(Texture texture, int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	texture->status = kTextureWritten;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::constructor(const TextureSpec* textureSpec, uint16 id)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Set id
	this->id = id;
	this->generation = 0;

	this->doUpdate = NULL;

	this->mapDisplacement = 0;
	this->usageCount = 1;

	// Save the bgmap spec's address
	this->textureSpec = textureSpec;
	this->charSet = NULL;
	// Set the palette
	this->palette = textureSpec->palette;
	this->status = kTextureNoCharSet;
	this->frame = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::destructor()
{
	this->status = kTextureInvalid;

	// Make sure that I'm not destroyed again
	this->usageCount = 0;

	Texture::releaseCharSet(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Texture::getId()
{
	return this->id;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::setSpec(TextureSpec* textureSpec)
{
	ASSERT(textureSpec, "Texture::setSpec: null textureSpec");

	if(NULL == textureSpec)
	{
		return;
	}

	if(this->textureSpec != textureSpec || kTextureWritten != this->status)
	{
		if(NULL != this->charSet && textureSpec->charSetSpec != CharSet::getSpec(this->charSet))
		{
			Texture::releaseCharSet(this);
		}

		this->textureSpec = textureSpec;
		this->frame = 0;
		this->mapDisplacement = 0;
		this->palette = this->textureSpec->palette;
		this->status = kTexturePendingWriting;
		Texture::rewrite(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const TextureSpec* Texture::getSpec()
{
	return this->textureSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CharSet Texture::getCharSet(uint32 loadIfNeeded)
{
	if(isDeleted(this->charSet) && loadIfNeeded)
	{
		Texture::loadCharSet(this);
	}

	return this->charSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::increaseUsageCount()
{
	this->usageCount++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::decreaseUsageCount()
{
	if(0 >= --this->usageCount)
	{
		this->usageCount = 0;
	}

	// The commented code saves on lookups for a charset in Texture::setSpec, but can cause
	// heavy char memory defragmentation
	if(0 == this->usageCount)// && !this->textureSpec->recyclable)
	{
		Texture::releaseCharSet(this);
	}

	return 0 == this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 Texture::getUsageCount()
{
	return this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::setPalette(uint8 palette)
{
	this->palette = palette;

	Texture::rewrite(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 Texture::getPalette()
{
	return this->palette;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Texture::getNumberOfFrames()
{
	return this->textureSpec->numberOfFrames;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::setFrame(uint16 frame)
{
	if(this->frame != frame || kTextureWritten != this->status)
	{
		this->frame = frame;
		Texture::setStatus(this, kTextureFrameChanged);	

		if(!isDeleted(this->charSet))
		{
			CharSet::setFrame(this->charSet, this->frame);		
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Texture::getFrame()
{
	return this->frame;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Texture::getCols()
{
	return this->textureSpec->cols;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Texture::getRows()
{
	return this->textureSpec->rows;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::isWritten()
{
	return kTextureWritten == this->status;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::isAnimated()
{
	return !isDeleted(this->charSet) && CharSet::hasMultipleFrames(this->charSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::isSingleFrame()
{
	return Texture::isSpecSingleFrame(this->textureSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::isMultiframe()
{
	return !Texture::isSpecSingleFrame(this->textureSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::addChar(const Point* textureChar, const uint32* newChar)
{
	if
	(
		NULL != this->charSet && NULL != textureChar && ((unsigned)textureChar->x) < this->textureSpec->cols && 
		((unsigned)textureChar->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * textureChar->y + textureChar->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;

		CharSet::addChar(this->charSet, charToReplace, newChar);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::putChar(const Point* textureChar, const uint32* newChar)
{
	if
	(
		NULL != this->charSet && NULL != textureChar && ((unsigned)textureChar->x) < this->textureSpec->cols && 
		((unsigned)textureChar->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * textureChar->y + textureChar->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;

		CharSet::putChar(this->charSet, charToReplace, newChar);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::putPixel(const Point* texturePixel, const Pixel* charSetPixel, uint8 newPixelColor)
{
	if
	(
		this->charSet && texturePixel && ((unsigned)texturePixel->x) < this->textureSpec->cols && 
		((unsigned)texturePixel->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * texturePixel->y + texturePixel->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;
		CharSet::putPixel(this->charSet, charToReplace, charSetPixel, newPixelColor);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::prepare()
{
	if(isDeleted(this->charSet))
	{
		this->status = kTextureNoCharSet;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 Texture::update(int16 maximumTextureRowsToWrite)
{
#ifndef __RELEASE		
	if(NULL != this->charSet)
	{
		this->status = this->generation != CharSet::getGeneration(this->charSet)? kTexturePendingRewriting : this->status;
	}
#endif

	switch(this->status)
	{
		case kTextureNoCharSet:
		{
			Texture::loadCharSet(this);
			break;
		}
		
		case kTexturePendingWriting:
		case kTexturePendingRewriting:
		{
			Texture::write(this, maximumTextureRowsToWrite);
			break;
		}

		case kTextureFrameChanged:
		{
			if(NULL != this->doUpdate)
			{
				this->doUpdate(this, maximumTextureRowsToWrite);
			}
			else
			{
				this->status = kTextureWritten;
			}

			break;
		}
	}

	return this->status;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::write(int16 maximumTextureRowsToWrite __attribute__((unused)))
{
	ASSERT(this->textureSpec, "Texture::write: null textureSpec");
	ASSERT(this->textureSpec->charSetSpec, "Texture::write: null charSetSpec");

	if(isDeleted(this->charSet))
	{
		return false;
	}

	if(CharSet::isShared(this->charSet))
	{
		this->frame = CharSet::getFrame(this->charSet);
	}

	CharSet::setFrame(this->charSet, this->frame);		

	this->generation = CharSet::write(this->charSet);

	if(CharSet::isOptimized(this->charSet))
	{
		this->mapDisplacement = this->textureSpec->cols * this->textureSpec->rows * this->frame;
	}

	this->status = kTextureWritten;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::rewrite()
{
	Texture::setStatus(this, kTexturePendingWriting);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::setStatus(uint8 status)
{
	if(kTextureNoCharSet == this->status)
	{
		return;
	}
	
	this->status = this->status > status ? status : this->status;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::loadCharSet()
{
	if(!isDeleted(this->charSet))
	{
		return;
	}

	if(NULL == this->textureSpec || NULL == this->textureSpec->charSetSpec)
	{
		this->status = kTextureInvalid;
		return;
	}

	this->charSet = CharSet::get(this->textureSpec->charSetSpec);

	if(isDeleted(this->charSet))
	{
		this->status = kTextureNoCharSet;
		return;
	}

	this->generation = CharSet::getGeneration(this->charSet);

	this->status = kTexturePendingWriting;

	Texture::setupUpdateFunction(this);

	if(CharSet::isShared(this->charSet))
	{
		if(1 == CharSet::getUsageCount(this->charSet))
		{
			CharSet::setFrame(this->charSet, this->frame);			
		}
		else
		{		
			this->frame = CharSet::getFrame(this->charSet);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::releaseCharSet()
{
	this->status = kTextureInvalid;

	if(!isDeleted(this->charSet))
	{
		CharSet::release(this->charSet);

		this->charSet = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::setMapDisplacement(uint32 mapDisplacement)
{
	if(this->mapDisplacement != mapDisplacement || kTextureWritten != this->status)
	{
		this->mapDisplacement = mapDisplacement;
		Texture::setStatus(this, kTexturePendingRewriting);		
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
	}
	else
	{			
		this->doUpdate = Texture::updateDefault;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
