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

#include <ParamTableManager.h>
#include <BgmapTextureManager.h>
#include <VIPManager.h>


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

/**
 * @class 	ParamTableManager
 * @extends Object
 * @ingroup graphics-2d-sprites-bgmap
 */
implements ParamTableManager : Object;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void ParamTableManager::constructor(ParamTableManager this);
static u32 ParamTableManager::calculateSpriteParamTableSize(ParamTableManager this, BgmapSprite bgmapSprite);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ParamTableManager::getInstance()
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
void __attribute__ ((noinline)) ParamTableManager::constructor(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::constructor: null this");

	Base::constructor();

	this->bgmapSprites = __NEW(VirtualList);
	this->removedBgmapSpritesSizes = __NEW(VirtualList);
	this->previouslyMovedBgmapSprite = NULL;

	ParamTableManager::reset(this);
}

/**
 * Class denstructor
 *
 * @memberof		ParamTableManager
 * @public
 *
 * @param this		Function scope
 */
void ParamTableManager::destructor(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::destructor: null this");

	ParamTableManager::reset(this);

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
void ParamTableManager::reset(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::reset: null this");

	VirtualList::clear(this->bgmapSprites);

	VirtualNode node = this->removedBgmapSpritesSizes->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->data);
	}

	VirtualList::clear(this->removedBgmapSpritesSizes);

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
void ParamTableManager::calculateParamTableBase(ParamTableManager this, int availableBgmapSegmentsForParamTable)
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

	// find the next address that is a multiple of 8192
	// taking into account the printable area
	for(; (this->paramTableBase - (__PRINTABLE_BGMAP_AREA << 1)) % __BGMAP_SEGMENT_SIZE && this->paramTableBase > __BGMAP_SPACE_BASE_ADDRESS; this->paramTableBase--);

	NM_ASSERT(this->paramTableBase <= __PARAM_TABLE_END, "ParamTableManager::calculateParamTableBase: param table size is negative");

	this->size = __PARAM_TABLE_END - this->paramTableBase;

	BgmapTextureManager::calculateAvailableBgmapSegments(BgmapTextureManager::getInstance());
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
u32 ParamTableManager::getParamTableBase(ParamTableManager this)
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
static u32 ParamTableManager::calculateSpriteParamTableSize(ParamTableManager this __attribute__ ((unused)), BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	u16 spriteHead = Sprite::getHead(__SAFE_CAST(Sprite, bgmapSprite));
	u32 textureRows = Texture::getRows(Sprite::getTexture(__SAFE_CAST(Sprite, bgmapSprite))) + __PARAM_TABLE_PADDING;
	u32 size = 0;

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
u32 ParamTableManager::allocate(ParamTableManager this, BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bgmapSprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	u32 size = ParamTableManager::calculateSpriteParamTableSize(this, bgmapSprite);

	if(0 == size)
	{
		return 0;
	}

	u32 paramAddress = 0;

	//if there is space in the param table, allocate
	if(this->paramTableBase + this->used + size < (__PARAM_TABLE_END))
	{
		//set sprite param
		paramAddress = this->paramTableBase + this->used;

		//record sprite
		VirtualList::pushBack(this->bgmapSprites, bgmapSprite);

		//update the param bytes occupied
		this->size -= size;
		this->used += size;
	}

	if(!paramAddress)
	{
		Printing::text(Printing::getInstance(), "Total size: ", 20, 7, NULL);
		Printing::int(Printing::getInstance(), __PARAM_TABLE_END - this->paramTableBase, 20 + 19, 7, NULL);

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
void ParamTableManager::free(ParamTableManager this, BgmapSprite bgmapSprite)
{
	ASSERT(this, "ParamTableManager::free: null this");

	if(VirtualList::removeElement(this->bgmapSprites, bgmapSprite))
	{
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
bool ParamTableManager::defragmentProgressively(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::processRemoved: null this");

	if(this->paramTableFreeData.param)
	{
		VirtualNode node = this->bgmapSprites->head;

		for(; node; node = node->next)
		{
			BgmapSprite sprite = __SAFE_CAST(BgmapSprite, node->data);

			u32 spriteParam = BgmapSprite::getParam(sprite);

			// retrieve param
			if(spriteParam > this->paramTableFreeData.param)
			{
				int size = ParamTableManager::calculateSpriteParamTableSize(this, sprite);

				// check that the sprite won't override itself
				if(this->paramTableFreeData.param + size > spriteParam)
				{
					break;
				}

				if(__IS_OBJECT_ALIVE(this->previouslyMovedBgmapSprite) && 0 < BgmapSprite::getParamTableRow(this->previouslyMovedBgmapSprite))
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
 * @param x				Camera x coordinate
 * @param y				Camera y coordinate
 */
void ParamTableManager::print(ParamTableManager this, int x, int y)
{
	ASSERT(this, "ParamTableManager::print: null this");

	int xDisplacement = 11;

	Printing::text(Printing::getInstance(), "PARAM TABLE'S STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Size:              ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->size, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Used:              ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->used, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "ParamBase:          ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), this->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printing::text(Printing::getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, 8, NULL);
}
