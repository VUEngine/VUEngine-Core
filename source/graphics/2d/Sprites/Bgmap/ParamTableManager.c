/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================


#include <BgmapSprite.h>
#include <BgmapTextureManager.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "ParamTableManager.h"


//=========================================================================================================
// CLASS'S DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS'S PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ParamTableManager::reset()
{
	VirtualList::clear(this->bgmapSprites);

	this->paramTableBase = __PARAM_TABLE_END;

	// set the size of the param table
	this->size = __PARAM_TABLE_END - this->paramTableBase;

	NM_ASSERT(__PARAM_TABLE_END >= this->paramTableBase, "ParamTableManager::reset: param table size is negative");

	// TODO: all param tables should start at a 16bit boundary
	this->usedBytes = 1;

	this->paramTableFreeData.param = 0;
	this->paramTableFreeData.recoveredSize = 0;
	this->previouslyMovedBgmapSprite = NULL;
}
//---------------------------------------------------------------------------------------------------------
void ParamTableManager::setup(int32 availableBgmapSegmentsForParamTable)
{
	if(0 == availableBgmapSegmentsForParamTable)
	{
		this->paramTableBase = __PARAM_TABLE_END;
	}
	else
	{
		this->paramTableBase = __PARAM_TABLE_END - __BGMAP_SEGMENT_SIZE * availableBgmapSegmentsForParamTable;
	}

	// find the next address that is a multiple of 8192
	// taking into account the printable area
	for(; 0 != (this->paramTableBase % __BGMAP_SEGMENT_SIZE) && this->paramTableBase > __BGMAP_SPACE_BASE_ADDRESS; this->paramTableBase--);

	NM_ASSERT(this->paramTableBase <= __PARAM_TABLE_END, "ParamTableManager::setup: param table size is negative");

	this->size = __PARAM_TABLE_END - this->paramTableBase;

	BgmapTextureManager::calculateAvailableBgmapSegments(BgmapTextureManager::getInstance());

	// Clean param tables memory
	for(uint8* data = (uint8*)this->paramTableBase; data < (uint8*)__PARAM_TABLE_END; data++)
	{
		*data = 0;
	}
}
//---------------------------------------------------------------------------------------------------------
uint32 ParamTableManager::allocate(BgmapSprite bgmapSprite)
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
	if(this->paramTableBase + this->usedBytes + size < (__PARAM_TABLE_END))
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
		Printing::text(Printing::getInstance(), "Total size: ", 20, 7, NULL);
		Printing::int32(Printing::getInstance(), __PARAM_TABLE_END - this->paramTableBase, 20 + 19, 7, NULL);

		NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");
	}
#endif

	return paramAddress;
}
//---------------------------------------------------------------------------------------------------------
void ParamTableManager::free(BgmapSprite bgmapSprite)
{
	if(VirtualList::removeData(this->bgmapSprites, bgmapSprite))
	{
		uint32 paramToFree = BgmapSprite::getParam(bgmapSprite);
		
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

		// accounted for
		if(this->paramTableFreeData.param && this->paramTableFreeData.param <= BgmapSprite::getParam(bgmapSprite))
		{
			// but increase the space recovered
			this->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);

			return;
		}

		this->paramTableFreeData.param = BgmapSprite::getParam(bgmapSprite);
		this->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);
	}
}
//---------------------------------------------------------------------------------------------------------
void ParamTableManager::defragment(bool deferred)
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

				// retrieve param
				if(spriteParam > this->paramTableFreeData.param)
				{
					int32 size = ParamTableManager::calculateSpriteParamTableSize(this, sprite);

					// check that the sprite won't override itself
					if(this->paramTableFreeData.param + size > spriteParam)
					{
						break;
					}

					if(!isDeleted(this->previouslyMovedBgmapSprite) && 0 < BgmapSprite::getParamTableRow(this->previouslyMovedBgmapSprite))
					{
						break;
					}

					//move back paramSize bytes
					BgmapSprite::setParam(sprite, this->paramTableFreeData.param);

					// set the new param to move on the next cycle
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
//---------------------------------------------------------------------------------------------------------
uint32 ParamTableManager::getParamTableBase()
{
	return this->paramTableBase;
}
//---------------------------------------------------------------------------------------------------------
void ParamTableManager::print(int32 x, int32 y)
{
	int32 xDisplacement = 11;

	Printing::text(Printing::getInstance(), "PARAM TABLE STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Size:              ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->size, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Used:              ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->usedBytes - 1, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "ParamBase:          ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), this->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printing::text(Printing::getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, 8, NULL);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS'S PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ParamTableManager::constructor()
{
	Base::constructor();

	this->bgmapSprites = new VirtualList();
	this->previouslyMovedBgmapSprite = NULL;

	ParamTableManager::reset(this);
}
//---------------------------------------------------------------------------------------------------------
void ParamTableManager::destructor()
{
	ParamTableManager::reset(this);

	delete this->bgmapSprites;
	this->bgmapSprites = NULL;

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
uint32 ParamTableManager::calculateSpriteParamTableSize(BgmapSprite bgmapSprite)
{
	ASSERT(bgmapSprite, "ParamTableManager::calculateSpriteParamTableSize: null sprite");

	uint16 spriteHead = Sprite::getHead(bgmapSprite);
	uint32 textureRows = Texture::getRows(Sprite::getTexture(bgmapSprite)) + __PARAM_TABLE_PADDING;
	uint32 size = 0;

	if(__WORLD_AFFINE & spriteHead)
	{
		if(64 < textureRows)
		{
			textureRows = 64;
		}

		// calculate necessary space to allocate
		// size = sprite's rows * 8 pixels each one * 16 bytes needed by each row = sprite's rows * 2 ^ 7
		// add one row as padding to make sure not ovewriting take place
		size = (textureRows << 7) * __MAXIMUM_SCALE;
	}
	else if(__WORLD_HBIAS & spriteHead)
	{
		if(28 < textureRows)
		{
			textureRows = 28;
		}

		// size = sprite's rows * 8 pixels each one * 4 bytes needed by each row = sprite's rows * 2 ^ 5
		size = textureRows << 5;
	}

	return size;
}
//---------------------------------------------------------------------------------------------------------
