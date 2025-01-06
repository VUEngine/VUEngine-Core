/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with paramTableManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapSprite.h>
#include <BgmapTextureManager.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "ParamTableManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::reset()
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	VirtualList::clear(paramTableManager->bgmapSprites);

	paramTableManager->paramTableBase = __PARAM_TABLE_END;

	// Set the size of the param table
	paramTableManager->size = __PARAM_TABLE_END - paramTableManager->paramTableBase;

	NM_ASSERT(__PARAM_TABLE_END >= paramTableManager->paramTableBase, "ParamTableManager::reset: param table size is negative");

	// TODO: all param tables should start at a 16bit boundary
	paramTableManager->usedBytes = 1;

	paramTableManager->paramTableFreeData.param = 0;
	paramTableManager->paramTableFreeData.recoveredSize = 0;
	paramTableManager->previouslyMovedBgmapSprite = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::setup(int32 availableBgmapSegmentsForParamTable)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	if(0 == availableBgmapSegmentsForParamTable)
	{
		paramTableManager->paramTableBase = __PARAM_TABLE_END;
	}
	else
	{
		paramTableManager->paramTableBase = __PARAM_TABLE_END - __BGMAP_SEGMENT_SIZE * availableBgmapSegmentsForParamTable;
	}

	// Find the next address that is a multiple of 8192
	// Taking into account the printable area
	for(; 0 != (paramTableManager->paramTableBase % __BGMAP_SEGMENT_SIZE) && paramTableManager->paramTableBase > __BGMAP_SPACE_BASE_ADDRESS; paramTableManager->paramTableBase--);

	NM_ASSERT(paramTableManager->paramTableBase <= __PARAM_TABLE_END, "ParamTableManager::setup: param table size is negative");

	paramTableManager->size = __PARAM_TABLE_END - paramTableManager->paramTableBase;

	BgmapTextureManager::calculateAvailableBgmapSegments();

	// Clean param tables memory
	for(uint8* data = (uint8*)paramTableManager->paramTableBase; data < (uint8*)__PARAM_TABLE_END; data++)
	{
		*data = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 ParamTableManager::allocate(BgmapSprite bgmapSprite)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	Texture texture = Sprite::getTexture(bgmapSprite);

	if(isDeleted(texture))
	{
		return 0;
	}

	if(Texture::isShared(texture))
	{
		if(0 != paramTableManager->paramTableFreeData.param)
		{
			for(VirtualNode node = paramTableManager->bgmapSprites->head; NULL != node; node = node->next)
			{
				BgmapSprite bgmapSpriteHelper = BgmapSprite::safeCast(node->data);

				Texture textureHelper = BgmapSprite::getTexture(bgmapSpriteHelper);

				if(!isDeleted(textureHelper) && Texture::getSpec(texture) == Texture::getSpec(textureHelper))
				{
					if(!isDeleted(textureHelper) && Texture::isShared(texture) == Texture::isShared(textureHelper))
					{
						VirtualList::pushBack(paramTableManager->bgmapSprites, bgmapSprite);

						return BgmapSprite::getParam(bgmapSpriteHelper);
					}
				}
			}
		}	
	}

	//calculate necessary space to allocate
	uint32 size = ParamTableManager::calculateSpriteParamTableSize(bgmapSprite);

	if(0 == size)
	{
		return 0;
	}

	uint32 paramAddress = 0;

	//if there is space in the param table, allocate
	if(paramTableManager->paramTableBase + paramTableManager->usedBytes + size < (__PARAM_TABLE_END))
	{
		//set sprite param
		paramAddress = paramTableManager->paramTableBase + paramTableManager->usedBytes;

		//record sprite
		VirtualList::pushBack(paramTableManager->bgmapSprites, bgmapSprite);

		//update the param bytes occupied
		paramTableManager->size -= size;
		paramTableManager->usedBytes += size;
	}

#ifndef __SHIPPING
	if(0 == paramAddress)
	{
		Printing::text("Total size: ", 20, 7, NULL);
		Printing::int32(__PARAM_TABLE_END - paramTableManager->paramTableBase, 20 + 19, 7, NULL);

		NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");
	}
#endif

	return paramAddress;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::free(BgmapSprite bgmapSprite)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	if(VirtualList::removeData(paramTableManager->bgmapSprites, bgmapSprite))
	{
		uint32 paramToFree = BgmapSprite::getParam(bgmapSprite);
		
		for(VirtualNode node = paramTableManager->bgmapSprites->head; NULL != node; node = node->next)
		{
			BgmapSprite bgmapSpriteHelper = BgmapSprite::safeCast(node->data);

			if(BgmapSprite::getParam(bgmapSpriteHelper) == paramToFree)
			{
				return;
			}
		}

		if(paramTableManager->previouslyMovedBgmapSprite == bgmapSprite)
		{
			paramTableManager->previouslyMovedBgmapSprite = NULL;
		}

		// Accounted for
		if(paramTableManager->paramTableFreeData.param && paramTableManager->paramTableFreeData.param <= BgmapSprite::getParam(bgmapSprite))
		{
			// But increase the space recovered
			paramTableManager->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(bgmapSprite);

			return;
		}

		paramTableManager->paramTableFreeData.param = BgmapSprite::getParam(bgmapSprite);
		paramTableManager->paramTableFreeData.recoveredSize += ParamTableManager::calculateSpriteParamTableSize(bgmapSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::defragment(bool deferred)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	if(0 != paramTableManager->paramTableFreeData.param)
	{
		do
		{
			VirtualNode node = paramTableManager->bgmapSprites->head;
			
			for(; NULL != node; node = node->next)
			{
				BgmapSprite sprite = BgmapSprite::safeCast(node->data);

				uint32 spriteParam = BgmapSprite::getParam(sprite);

				// Retrieve param
				if(spriteParam > paramTableManager->paramTableFreeData.param)
				{
					int32 size = ParamTableManager::calculateSpriteParamTableSize(sprite);

					// Check that the sprite won't override itself
					if(paramTableManager->paramTableFreeData.param + size > spriteParam)
					{
						break;
					}

					if(!isDeleted(paramTableManager->previouslyMovedBgmapSprite) && 0 < BgmapSprite::getParamTableRow(paramTableManager->previouslyMovedBgmapSprite))
					{
						break;
					}

					//move back paramSize bytes
					BgmapSprite::setParam(sprite, paramTableManager->paramTableFreeData.param);

					// Set the new param to move on the next cycle
					paramTableManager->paramTableFreeData.param += size;

					paramTableManager->previouslyMovedBgmapSprite = sprite;

					break;
				}
			}

			if(NULL == node)
			{
				//recover space
				paramTableManager->usedBytes -= paramTableManager->paramTableFreeData.recoveredSize;
				paramTableManager->size += paramTableManager->paramTableFreeData.recoveredSize;

				paramTableManager->paramTableFreeData.param = 0;
				paramTableManager->paramTableFreeData.recoveredSize = 0;

				paramTableManager->previouslyMovedBgmapSprite = NULL;
			}
		}
		while(!deferred && 0 != paramTableManager->paramTableFreeData.param);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 ParamTableManager::getParamTableBase()
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	return paramTableManager->paramTableBase;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ParamTableManager::print(int32 x, int32 y)
{
	ParamTableManager paramTableManager = ParamTableManager::getInstance();

	int32 xDisplacement = 11;

	Printing::text("PARAM TABLE STATUS", x, y++, NULL);
	Printing::text("Size:              ", x, ++y, NULL);
	Printing::int32(paramTableManager->size, x + xDisplacement, y, NULL);

	Printing::text("Used:              ", x, ++y, NULL);
	Printing::int32(paramTableManager->usedBytes - 1, x + xDisplacement, y, NULL);

	Printing::text("ParamBase:          ", x, ++y, NULL);
	Printing::hex(paramTableManager->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printing::text("ParamEnd:           ", x, ++y, NULL);
	Printing::hex(__PARAM_TABLE_END, x + xDisplacement, y, 8, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 ParamTableManager::calculateSpriteParamTableSize(BgmapSprite bgmapSprite)
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

		// Calculate necessary space to allocate
		// Size = sprite's rows * 8 pixels each one * 16 bytes needed by each row = sprite's rows * 2 ^ 7
		// Add one row as padding to make sure not ovewriting take place
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

	ParamTableManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ParamTableManager::destructor()
{
	ParamTableManager::reset();

	delete this->bgmapSprites;
	this ->bgmapSprites = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
