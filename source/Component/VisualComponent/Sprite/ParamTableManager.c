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

#include <BgmapSprite.h>
#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <Printer.h>
#include <Singleton.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "ParamTableManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __PARAM_TABLE_PADDING		1

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::print(int32 x, int32 y)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	int32 xDisplacement = 11;

	Printer::text("PARAM TABLE STATUS", x, y++, NULL);
	Printer::text("Size:              ", x, ++y, NULL);
	Printer::int32(paramTableManager->paramTableEnd - paramTableManager->paramTableBase, x + xDisplacement, y, NULL);

	Printer::text("Available:              ", x, ++y, NULL);
	Printer::int32(paramTableManager->size, x + xDisplacement, y, NULL);

	Printer::text("Used:              ", x, ++y, NULL);
	Printer::int32(paramTableManager->usedBytes - 1, x + xDisplacement, y, NULL);

	Printer::text("ParamBase:          ", x, ++y, NULL);
	Printer::hex(paramTableManager->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printer::text("ParamEnd:           ", x, ++y, NULL);
	Printer::hex(paramTableManager->paramTableEnd, x + xDisplacement, y, 8, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ParamTableManager::reset()
{
	if(!isDeleted(this->bgmapSprites))
	{
		VirtualList::clear(this->bgmapSprites);
	}

	this->paramTableBase = this->paramTableEnd;

	// Set the size of the param table
	this->size = this->paramTableEnd - this->paramTableBase;

	NM_ASSERT(this->paramTableEnd >= this->paramTableBase, "ParamTableManager::reset: param table size is negative");

	// TODO: all param tables should start at a 16bit boundary
	this->usedBytes = 1;

	this->paramTableFreeData.param = 0;
	this->paramTableFreeData.recoveredSize = 0;
	this->previouslyMovedBgmapSprite = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure uint32 ParamTableManager::configure(int32 availableBgmapSegmentsForParamTable)
{
	if(0 == availableBgmapSegmentsForParamTable)
	{
		this->paramTableBase = this->paramTableEnd;
	}
	else
	{
		this->paramTableBase = this->paramTableEnd - __BGMAP_SEGMENT_SIZE * availableBgmapSegmentsForParamTable;
	}

	// Find the next address that is a multiple of 8192
	// Taking into account the printable area
	for(; 0 != (this->paramTableBase % __BGMAP_SEGMENT_SIZE) && this->paramTableBase > __BGMAP_SPACE_BASE_ADDRESS; this->paramTableBase--);

	NM_ASSERT(this->paramTableBase <= this->paramTableEnd, "ParamTableManager::setup: param table size is negative");

	this->size = this->paramTableEnd - this->paramTableBase;

	// Clean param tables memory
	for(uint8* data = (uint8*)this->paramTableBase; data < (uint8*)this->paramTableEnd; data++)
	{
		*data = 0;
	}

	return this->paramTableBase;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure uint32 ParamTableManager::allocate(BgmapSprite bgmapSprite)
{
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	Texture texture = Sprite::getTexture(bgmapSprite);

	if(isDeleted(texture))
	{
		return 0;
	}

	if(Texture::isShared(texture))
	{
		if(0 != this->paramTableFreeData.param)
		{
			for(VirtualNode node = this->bgmapSprites->head; NULL != node; node = node->next)
			{
				BgmapSprite bgmapSpriteHelper = BgmapSprite::safeCast(node->data);

				Texture textureHelper = BgmapSprite::getTexture(bgmapSpriteHelper);

				if(!isDeleted(textureHelper) && Texture::getSpec(texture) == Texture::getSpec(textureHelper))
				{
					if(!isDeleted(textureHelper) && Texture::isShared(texture) == Texture::isShared(textureHelper))
					{
						VirtualList::pushBack(this->bgmapSprites, bgmapSprite);

						return BgmapSprite::getParam(bgmapSpriteHelper);
					}
				}
			}
		}	
	}

	//calculate necessary space to allocate
	uint32 size = ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);

	if(0 == size)
	{
		return 0;
	}

	uint32 paramAddress = 0;

	//if there is space in the param table, allocate
	if(this->paramTableBase + this->usedBytes + size < (this->paramTableEnd))
	{
		//set sprite param
		paramAddress = this->paramTableBase + this->usedBytes;

		//record sprite
		VirtualList::pushBack(this->bgmapSprites, bgmapSprite);

		//update the param bytes occupied
		this->size -= size;
		this->usedBytes += size;
	}

#ifndef __SHIPPING
	if(0 == paramAddress)
	{
		Printer::text("Total size: ", 20, 7, NULL);
		Printer::int32(this->paramTableEnd - this->paramTableBase, 20 + 19, 7, NULL);

		NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");
	}
#endif

	return paramAddress + (__PARAM_TABLE_PADDING << 2) * 16;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ParamTableManager::free(BgmapSprite bgmapSprite)
{
	if(VirtualList::removeData(this->bgmapSprites, bgmapSprite))
	{
		uint32 paramToFree = BgmapSprite::getParam(bgmapSprite) - (__PARAM_TABLE_PADDING << 2) * 16;
		
		for(VirtualNode node = this->bgmapSprites->head; NULL != node; node = node->next)
		{
			BgmapSprite bgmapSpriteHelper = BgmapSprite::safeCast(node->data);

			if(BgmapSprite::getParam(bgmapSpriteHelper) == paramToFree)
			{
				return;
			}
		}

		if(this->previouslyMovedBgmapSprite == bgmapSprite)
		{
			this->previouslyMovedBgmapSprite = NULL;
		}

		// Accounted for
		if(this->paramTableFreeData.param && this->paramTableFreeData.param <= BgmapSprite::getParam(bgmapSprite))
		{
			// But increase the space recovered
			this->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);

			return;
		}

		this->paramTableFreeData.param = BgmapSprite::getParam(bgmapSprite);
		this->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ParamTableManager::defragment(bool deferred)
{
	if(0 != this->paramTableFreeData.param)
	{
		do
		{
			VirtualNode node = this->bgmapSprites->head;
			
			for(; NULL != node; node = node->next)
			{
				BgmapSprite sprite = BgmapSprite::safeCast(node->data);

				uint32 spriteParam = BgmapSprite::getParam(sprite);

				// Retrieve param
				if(spriteParam > this->paramTableFreeData.param)
				{
					int32 size = ParamTableManager::calculateSpriteParamTableSize(this, sprite);

					if(!isDeleted(this->previouslyMovedBgmapSprite) && 0 < BgmapSprite::getParamTableRow(this->previouslyMovedBgmapSprite))
					{
						break;
					}

					//move back paramSize bytes
					BgmapSprite::setParam(sprite, this->paramTableFreeData.param);

					// Set the new param to move on the next cycle
					this->paramTableFreeData.param += size;

					this->previouslyMovedBgmapSprite = sprite;

					break;
				}
			}

			if(NULL == node)
			{
				//recover space
				this->usedBytes -= this->paramTableFreeData.recoveredSize;
				this->size += this->paramTableFreeData.recoveredSize;

				this->paramTableFreeData.param = 0;
				this->paramTableFreeData.recoveredSize = 0;

				this->previouslyMovedBgmapSprite = NULL;
			}
		}
		while(!deferred && 0 != this->paramTableFreeData.param);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ParamTableManager::getParamTableEnd()
{
	return this->paramTableEnd;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParamTableManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->bgmapSprites = new VirtualList();
	this->previouslyMovedBgmapSprite = NULL;

	extern uint32 _dramDirtyStart;
	
	this->paramTableEnd = (uint32) & _dramDirtyStart;
	this->paramTableBase = this->paramTableEnd;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParamTableManager::destructor()
{
	if(!isDeleted(this->bgmapSprites))
	{
		delete this->bgmapSprites;
	}

	this ->bgmapSprites = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ParamTableManager::calculateSpriteParamTableSize(BgmapSprite bgmapSprite)
{
	ASSERT(bgmapSprite, "ParamTableManager::calculateSpriteParamTableSize: null sprite");

	if(NULL == Sprite::getSpec(bgmapSprite))
	{
		return 0;
	}

	uint16 spriteHead = Sprite::getHead(bgmapSprite);
	uint32 textureRows = ((SpriteSpec*)Sprite::getSpec(bgmapSprite))->textureSpec->rows + __PARAM_TABLE_PADDING;
	uint32 size = 0;

	if(__WORLD_AFFINE & spriteHead)
	{
		if(64 < textureRows)
		{
			textureRows = 64;
		}

		// Calculate necessary space to allocate
		// Size = sprite's rows * 8 pixels each one * 16 bytes needed by each row = sprite's rows * 2 ^ 7
		// Add one row as padding to make sure not ovewriting takes place
		size = (textureRows << 7) * __MAXIMUM_SCALE;
	}
	else if(__WORLD_HBIAS & spriteHead)
	{
		if(28 < textureRows)
		{
			textureRows = 28;
		}

		// Size = sprite's rows * 8 pixels each one * 4 bytes needed by each row = sprite's rows * 2 ^ 5
		size = textureRows << 5;
	}

	return size;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
