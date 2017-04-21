/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <ParamTableManager.h>
#include <BgmapTextureManager.h>
#include <VIPManager.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Param Table Free Data
 *
 * @memberof ParamTableManager
 */
typedef struct ParamTableFreeData
{
	u32 param;
	u32 recoveredSize;
} ParamTableFreeData;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ParamTableManager_ATTRIBUTES																	\
		Object_ATTRIBUTES																				\
		/**
		 * @var int 				size
		 * @brief 					total size of param table
		 * @memberof				ParamTableManager
		 */																								\
		int size;																						\
		/**
		 * @var u32 				used
		 * @brief 					number of used bytes
		 * @memberof				ParamTableManager
		 */																								\
		u32 used;																						\
		/**
		 * @var VirtualList 		bgmapSprites
		 * @brief 					allocated bgmapSprites
		 * @memberof				ParamTableManager
		 */																								\
		VirtualList bgmapSprites;																		\
		/**
		 * @var VirtualList 		removedBgmapSpritesSizes
		 * @brief 					removed bgmapSprites' sizes
		 * @memberof				ParamTableManager
		 */																								\
		VirtualList removedBgmapSpritesSizes;															\
		/**
		 * @var ParamTableFreeData 	paramTableFreeData
		 * @brief 					used for defragmentation
		 * @memberof				ParamTableManager
		 */																								\
		ParamTableFreeData paramTableFreeData;															\
		/**
		 * @var BgmapSprite 		previouslyMovedBgmapSprite
		 * @brief 					used for defragmentation
		 * @memberof				ParamTableManager
		 */																								\
		BgmapSprite previouslyMovedBgmapSprite;															\
		/**
		 * @var u32 				paramTableBase
		 * @brief 					used for defragmentation
		 * @memberof				ParamTableManager
		 */																								\
		u32 paramTableBase;																				\

/**
 * @class 	ParamTableManager
 * @extends Object
 * @ingroup graphics-2d-sprites-bgmap
 */
__CLASS_DEFINITION(ParamTableManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void ParamTableManager_constructor(ParamTableManager this);
static int ParamTableManager_calculateSpriteParamTableSize(ParamTableManager this, BgmapSprite bgmapSprite);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ParamTableManager_getInstance()
 * @memberof	ParamTableManager
 * @public
 *
 * @return		ParamTableManager instance
 */
__SINGLETON(ParamTableManager);

/**
 * Class constructor
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 */
void __attribute__ ((noinline)) ParamTableManager_constructor(ParamTableManager this)
{
	__CONSTRUCT_BASE(Object);

	this->bgmapSprites = __NEW(VirtualList);
	this->removedBgmapSpritesSizes = __NEW(VirtualList);
	this->previouslyMovedBgmapSprite = NULL;

	ParamTableManager_reset(this);
}

/**
 * Class denstructor
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 */
void ParamTableManager_destructor(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::destructor: null this");

	ParamTableManager_reset(this);

	__DELETE(this->bgmapSprites);
	this->bgmapSprites = NULL;

	__DELETE(this->removedBgmapSpritesSizes);
	this->removedBgmapSpritesSizes = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Reset management
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 */
void ParamTableManager_reset(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::reset: null this");

	VirtualList_clear(this->bgmapSprites);

	VirtualNode node = this->removedBgmapSpritesSizes->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->data);
	}

	VirtualList_clear(this->removedBgmapSpritesSizes);

	this->paramTableBase = __PARAM_TABLE_END;

	// set the size of the paramtable
	this->size = __PARAM_TABLE_END - this->paramTableBase;

	NM_ASSERT(__PARAM_TABLE_END >= this->paramTableBase, "ParamTableManager::reset: param table size is negative");

	// all the memory is free
	this->used = 1;

	this->paramTableFreeData.param = 0;
	this->paramTableFreeData.recoveredSize = 0;
	this->previouslyMovedBgmapSprite = NULL;
}

/**
 * Calculate the param table's base address
 *
 * @memberof										ParamTableManager
 * @public
 *
 * @param this										Function scope
 * @param availableBgmapSegmentsForParamTable		Number of BGMAP segments for the param tables
 */
void ParamTableManager_calculateParamTableBase(ParamTableManager this, int availableBgmapSegmentsForParamTable)
{
	ASSERT(this, "ParamTableManager::calculateParamTableBase: null this");

	if(!availableBgmapSegmentsForParamTable)
	{
		this->paramTableBase = __PARAM_TABLE_END;
	}
	else
	{
		this->paramTableBase = __PARAM_TABLE_END - __BGMAP_SEGMENT_SIZE * availableBgmapSegmentsForParamTable;
	}

	for(; (this->paramTableBase - (__PRINTABLE_BGMAP_AREA << 1)) % __BGMAP_SEGMENT_SIZE && this->paramTableBase > __BGMAP_SPACE_BASE_ADDRESS; this->paramTableBase--);

	NM_ASSERT(this->paramTableBase <= __PARAM_TABLE_END, "ParamTableManager::calculateParamTableBase: param table size is negative");

	this->size = __PARAM_TABLE_END - this->paramTableBase;

	BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager_getInstance());
}

/**
 * Retrieve the param table's base address
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			The base address of the param table
 */
u32 ParamTableManager_getParamTableBase(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::getParamTableBase: null this");

	return this->paramTableBase;
}

/**
 * Calculate the param table'size for the given Sprite
 *
 * @memberof			ParamTableManager
 * @public
 *
 * @param this			Function scope
 * @param bgmapSprite	Sprite to base the calculation on
 *
 * @return				Param table's size for the Sprite
 */
static int ParamTableManager_calculateSpriteParamTableSize(ParamTableManager this __attribute__ ((unused)), BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	// calculate necessary space to allocate
	// size = sprite's rows * 8 pixels each on * 16 bytes needed by each row = sprite's rows * 2 ^ 7
	// add one row as padding to make sure not ovewriting take place
	return (((int)Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, bgmapSprite))) + __PARAM_TABLE_PADDING) << 7) * __MAXIMUM_SCALE;
}

/**
 * Allocate param table space for the given Sprite
 *
 * @memberof			ParamTableManager
 * @public
 *
 * @param this			Function scope
 * @param bgmapSprite	Sprite for which the param table space will be allocated
 *
 * @return				True if param table space was allocated
 */
u32 ParamTableManager_allocate(ParamTableManager this, BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	int size = ParamTableManager_calculateSpriteParamTableSize(this, bgmapSprite);

	u32 paramAddress = 0;

	//if there is space in the param table, allocate
	if(this->paramTableBase + this->used + size < (__PARAM_TABLE_END))
	{
		//set sprite param
		paramAddress = this->paramTableBase + this->used;

		//record sprite
		VirtualList_pushBack(this->bgmapSprites, bgmapSprite);

		//update the param bytes ocupied
		this->size -= size;
		this->used += size;
	}

	if(!paramAddress)
	{
		Printing_text(Printing_getInstance(), "Total size: ", 20, 7, NULL);
		Printing_int(Printing_getInstance(), __PARAM_TABLE_END - this->paramTableBase, 20 + 19, 7, NULL);

		NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");
	}

	return paramAddress;
}

/**
 * Free the param table space used by the Sprite
 *
 * @memberof			ParamTableManager
 * @public
 *
 * @param this			Function scope
 * @param bgmapSprite	Sprite of which param table space will be freed
 */
void ParamTableManager_free(ParamTableManager this, BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::free: null this");
	ASSERT(VirtualList_find(this->bgmapSprites, bgmapSprite), "ParamTableManager::free: sprite not found");

	VirtualList_removeElement(this->bgmapSprites, bgmapSprite);

	if(this->previouslyMovedBgmapSprite == bgmapSprite)
	{
		this->previouslyMovedBgmapSprite = NULL;
	}

	// accounted for
	if(this->paramTableFreeData.param && this->paramTableFreeData.param <= BgmapSprite_getParam(bgmapSprite))
	{
		// but increase the space recovered
		this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSpriteParamTableSize(this, bgmapSprite);

		return;
	}

	this->paramTableFreeData.param = BgmapSprite_getParam(bgmapSprite);
	this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSpriteParamTableSize(this, bgmapSprite);
}

/**
 * Defragment the param table space
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 *
 * @return 			True if defragmentation took place
 */
bool ParamTableManager_defragmentProgressively(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::processRemoved: null this");

	if(this->paramTableFreeData.param)
	{
		VirtualNode node = this->bgmapSprites->head;

		for(; node; node = node->next)
		{
			BgmapSprite sprite = __SAFE_CAST(BgmapSprite, node->data);

			u32 spriteParam = BgmapSprite_getParam(sprite);

			// retrieve param
			if(spriteParam > this->paramTableFreeData.param)
			{
				int size = ParamTableManager_calculateSpriteParamTableSize(this, sprite);

				// check that the sprite won't override itself
				if(this->paramTableFreeData.param + size > spriteParam)
				{
					break;
				}

				if(this->previouslyMovedBgmapSprite && this->previouslyMovedBgmapSprite && 0 < BgmapSprite_getParamTableRow(this->previouslyMovedBgmapSprite))
				{
					break;
				}

				//move back paramSize bytes
				BgmapSprite_setParam(sprite, this->paramTableFreeData.param);

				// set the new param to move on the next cycle
				this->paramTableFreeData.param += size;

				this->previouslyMovedBgmapSprite = sprite;

				break;
			}
		}

		if(!node)
		{
			//recover space
			this->used -= this->paramTableFreeData.recoveredSize;
			this->size += this->paramTableFreeData.recoveredSize;

			this->paramTableFreeData.param = 0;
			this->paramTableFreeData.recoveredSize = 0;

			this->previouslyMovedBgmapSprite = NULL;

			return true;
		}
	}

	return false;
}

/**
 * Print the manager's state
 *
 * @memberof			ParamTableManager
 * @public
 *
 * @param this			Function scope
 * @param x				Screen x coordinate
 * @param y				Screen y coordinate
 */
void ParamTableManager_print(ParamTableManager this, int x, int y)
{
	ASSERT(this, "ParamTableManager::print: null this");

	int xDisplacement = 11;

	Printing_text(Printing_getInstance(), "PARAM TABLE'S STATUS", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Size:              ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->size, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Used:              ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->used, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "ParamBase:          ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), this->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printing_text(Printing_getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, 8, NULL);
}
