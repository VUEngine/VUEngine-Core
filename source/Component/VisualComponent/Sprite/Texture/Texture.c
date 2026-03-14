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

#include <TileSetManager.h>
#include <TextureManager.h>
#include <Optics.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "Texture.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class TileSet;
friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 Texture::getTotalCols(TextureSpec* textureSpec)
{
	if(NULL == textureSpec->tileSetSpec)
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
	if(NULL == textureSpec->tileSetSpec)
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
	if(isDeleted(texture->tileSet))
	{
		return;
	}

	texture->generation = TileSet::write(texture->tileSet);

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
	this->tileSet = NULL;
	// Set the palette
	this->palette = textureSpec->palette;
	this->status = kTextureNoTileSet;
	this->frame = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::destructor()
{
	this->status = kTextureInvalid;

	// Make sure that I'm not destroyed again
	this->usageCount = 0;

	Texture::releaseTileSet(this);

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
		if(NULL != this->tileSet && textureSpec->tileSetSpec != TileSet::getSpec(this->tileSet))
		{
			Texture::releaseTileSet(this);
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

TileSet Texture::getTileSet(uint32 loadIfNeeded)
{
	if(isDeleted(this->tileSet) && loadIfNeeded)
	{
		Texture::loadTileSet(this);
	}

	return this->tileSet;
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

	// The commented code saves on lookups for a tileSet in Texture::setSpec, but can cause
	// heavy char memory defragmentation
	if(0 == this->usageCount)// && !this->textureSpec->recyclable)
	{
		Texture::releaseTileSet(this);
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

		if(!isDeleted(this->tileSet))
		{
			TileSet::setFrame(this->tileSet, this->frame);		
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
	if(!isDeleted(this->tileSet))
	{
		return TileSet::isShared(this->tileSet);
	}

	if(NULL != this->textureSpec->tileSetSpec)
	{
		return this->textureSpec->tileSetSpec->shared;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Texture::isAnimated()
{
	return !isDeleted(this->tileSet) && TileSet::hasMultipleFrames(this->tileSet);
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
		NULL != this->tileSet && NULL != textureChar && ((unsigned)textureChar->x) < this->textureSpec->cols && 
		((unsigned)textureChar->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * textureChar->y + textureChar->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;

		TileSet::addChar(this->tileSet, charToReplace, newChar);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::putChar(const Point* textureChar, const uint32* newChar)
{
	if
	(
		NULL != this->tileSet && NULL != textureChar && ((unsigned)textureChar->x) < this->textureSpec->cols && 
		((unsigned)textureChar->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * textureChar->y + textureChar->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;

		TileSet::putChar(this->tileSet, charToReplace, newChar);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::putPixel(const Point* texturePixel, const Pixel* tileSetPixel, uint8 newPixelColor)
{
	if
	(
		this->tileSet && texturePixel && ((unsigned)texturePixel->x) < this->textureSpec->cols && 
		((unsigned)texturePixel->y) < this->textureSpec->rows
	)
	{
		uint32 displacement = this->textureSpec->cols * texturePixel->y + texturePixel->x;
		uint32 charToReplace = this->textureSpec->map[displacement] & 0x7FF;
		TileSet::putPixel(this->tileSet, charToReplace, tileSetPixel, newPixelColor);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::prepare()
{
	if(isDeleted(this->tileSet))
	{
		this->status = kTextureNoTileSet;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 Texture::update(int16 maximumTextureRowsToWrite)
{
#ifndef __RELEASE		
	if(NULL != this->tileSet)
	{
		this->status = this->generation != TileSet::getGeneration(this->tileSet)? kTexturePendingRewriting : this->status;
	}
#endif

	switch(this->status)
	{
		case kTextureNoTileSet:
		{
			Texture::loadTileSet(this);
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
	ASSERT(this->textureSpec->tileSetSpec, "Texture::write: null tileSetSpec");

	if(isDeleted(this->tileSet))
	{
		return false;
	}

	if(TileSet::isShared(this->tileSet))
	{
		this->frame = TileSet::getFrame(this->tileSet);
	}

	TileSet::setFrame(this->tileSet, this->frame);		

	this->generation = TileSet::write(this->tileSet);

	if(TileSet::isOptimized(this->tileSet))
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
	if(kTextureNoTileSet == this->status)
	{
		return;
	}
	
	this->status = this->status > status ? status : this->status;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::loadTileSet()
{
	if(!isDeleted(this->tileSet))
	{
		return;
	}

	if(NULL == this->textureSpec || NULL == this->textureSpec->tileSetSpec)
	{
		this->status = kTextureInvalid;
		return;
	}

	this->tileSet = TileSet::get(this->textureSpec->tileSetSpec);

	if(isDeleted(this->tileSet))
	{
		this->status = kTextureNoTileSet;
		return;
	}

	this->generation = TileSet::getGeneration(this->tileSet);

	this->status = kTexturePendingWriting;

	Texture::setupUpdateFunction(this);

	if(TileSet::isShared(this->tileSet))
	{
		if(1 == TileSet::getUsageCount(this->tileSet))
		{
			TileSet::setFrame(this->tileSet, this->frame);			
		}
		else
		{		
			this->frame = TileSet::getFrame(this->tileSet);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Texture::releaseTileSet()
{
	this->status = kTextureInvalid;

	if(!isDeleted(this->tileSet))
	{
		TileSet::release(this->tileSet);

		this->tileSet = NULL;
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
	else if(TileSet::isOptimized(this->tileSet))
	{
		this->doUpdate = Texture::updateOptimized;
	}
	else
	{			
		this->doUpdate = Texture::updateDefault;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
