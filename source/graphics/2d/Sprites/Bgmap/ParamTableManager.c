/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


// ---------------------------------------------------------------------------------------------------------
// 												INCLUDES
// ---------------------------------------------------------------------------------------------------------

#include <ParamTableManager.h>
#include <BgmapTextureManager.h>
#include <VIPManager.h>
#include <Printing.h>

// ---------------------------------------------------------------------------------------------------------
// 											  DECLARATIONS
// ---------------------------------------------------------------------------------------------------------

typedef struct ParamTableFreeData
{
	u32 param;
	u32 recoveredSize;
} ParamTableFreeData;

// ---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
// ---------------------------------------------------------------------------------------------------------

#define ParamTableManager_ATTRIBUTES                                                                    \
        /* super's attributes */                                                                        \
        Object_ATTRIBUTES                                                                              \
        /* total size of param table */                                                                 \
        int size;                                                                                       \
        /* number of used bytes */                                                                      \
        u32 used;                                                                                       \
        /* allocated bgmapSprites */                                                                        \
        VirtualList bgmapSprites;                                                                           \
        /* removed bgmapSprites' sizes */                                                                   \
        VirtualList removedBgmapSpritesSizes;                                                           \
        /* user for defragmentation */                                                                  \
        ParamTableFreeData paramTableFreeData;                                                          \
        /* user for defragmentation */                                                                  \
        BgmapSprite previouslyMovedBgmapSprite;                                                         \
        /* user for defragmentation */                                                                  \
        u32 paramTableBase;                                                                             \

__CLASS_DEFINITION(ParamTableManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


// ---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
// ---------------------------------------------------------------------------------------------------------

void ParamTableManager_constructor(ParamTableManager this);
static int ParamTableManager_calculateSpriteParamTableSize(ParamTableManager this, BgmapSprite bgmapSprite);


// ---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
// ---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(ParamTableManager);

//class constructor
void __attribute__ ((noinline)) ParamTableManager_constructor(ParamTableManager this)
{
	__CONSTRUCT_BASE(Object);

	this->bgmapSprites = __NEW(VirtualList);
	this->removedBgmapSpritesSizes = __NEW(VirtualList);
	this->previouslyMovedBgmapSprite = NULL;

	ParamTableManager_reset(this);
}

// class destructor
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

// reset
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

u32 ParamTableManager_getParamTableBase(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::getParamTableBase: null this");

    return this->paramTableBase;
}

// calculate size of param table
static int ParamTableManager_calculateSpriteParamTableSize(ParamTableManager this __attribute__ ((unused)), BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	// calculate necessary space to allocate
	// size = sprite's rows * 8 pixels each on * 16 bytes needed by each row = sprite's rows * 2 ^ 7
	// add one row as padding to make sure not ovewriting take place
	return (((int)Texture_getRows(Sprite_getTexture(__SAFE_CAST(Sprite, bgmapSprite))) + __PARAM_TABLE_PADDING) << 7) * __MAXIMUM_SCALE;
}

// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	int size = ParamTableManager_calculateSpriteParamTableSize(this, bgmapSprite);

	//if there is space in the param table, allocate
	if(this->paramTableBase + this->used + size < (__PARAM_TABLE_END))
	{
		//set sprite param
		BgmapSprite_setParam(bgmapSprite, this->paramTableBase + this->used);

		//record sprite
		VirtualList_pushBack(this->bgmapSprites, bgmapSprite);

		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		return true;
	}

	Printing_text(Printing_getInstance(), "Total size: ", 20, 7, NULL);
	Printing_int(Printing_getInstance(), __PARAM_TABLE_END - this->paramTableBase, 20 + 19, 7, NULL);

	NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");

	return false;
}

// deallocate param table space
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

// relocate bgmapSprites
bool ParamTableManager_processRemovedSprites(ParamTableManager this)
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

// print param table's attributes state
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
	Printing_hex(Printing_getInstance(), this->paramTableBase, x + xDisplacement, y, NULL);
	Printing_text(Printing_getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, NULL);
}
