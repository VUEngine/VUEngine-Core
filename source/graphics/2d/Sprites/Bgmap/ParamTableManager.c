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

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ParamTableManager::getInstance()
 * @memberof	ParamTableManager
 * @public
 * @return		ParamTableManager instance
 */


/**
 * Class constructor
 */
void ParamTableManager::constructor()
{
	Base::constructor();

	this->bgmapSprites = new VirtualList();
	this->removedBgmapSpritesSizes = new VirtualList();
	this->previouslyMovedBgmapSprite = NULL;

	ParamTableManager::reset(this);
}

/**
 * Class denstructor
 */
void ParamTableManager::destructor()
{
	ParamTableManager::reset(this);

	delete this->bgmapSprites;
	this->bgmapSprites = NULL;

	delete this->removedBgmapSpritesSizes;
	this->removedBgmapSpritesSizes = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset management
 */
void ParamTableManager::reset()
{
	VirtualList::clear(this->bgmapSprites);

	VirtualNode node = this->removedBgmapSpritesSizes->head;

	for(; node; node = node->next)
	{
		delete node->data;
	}

	VirtualList::clear(this->removedBgmapSpritesSizes);

	this->paramTableBase = __PARAM_TABLE_END;

	// set the size of the param table
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
 * @param availableBgmapSegmentsForParamTable	Number of BGMAP segments for the param tables
 */
void ParamTableManager::calculateParamTableBase(int availableBgmapSegmentsForParamTable)
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

	NM_ASSERT(this->paramTableBase <= __PARAM_TABLE_END, "ParamTableManager::calculateParamTableBase: param table size is negative");

	this->size = __PARAM_TABLE_END - this->paramTableBase;

	BgmapTextureManager::calculateAvailableBgmapSegments(BgmapTextureManager::getInstance());

	// Clean param tables memory
	for(u8* data = (u8*)this->paramTableBase; data < (u8*)__PARAM_TABLE_END; data++)
	{
		*data = 0;
	}
}

/**
 * Retrieve the param table's base address
 *
 * @return	The base address of the param table
 */
u32 ParamTableManager::getParamTableBase()
{
	return this->paramTableBase;
}

/**
 * Calculate the param table'size for the given Sprite
 *
 * @param bgmapSprite	Sprite to base the calculation on
 * @return				Param table's size for the Sprite
 */
u32 ParamTableManager::calculateSpriteParamTableSize(BgmapSprite bgmapSprite)
{
	ASSERT(bgmapSprite, "ParamTableManager::calculateSpriteParamTableSize: null sprite");

	u16 spriteHead = Sprite::getHead(bgmapSprite);
	u32 textureRows = Texture::getRows(Sprite::getTexture(bgmapSprite)) + __PARAM_TABLE_PADDING;
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
 * @param bgmapSprite	Sprite for which the param table space will be allocated
 * @return				True if param table space was allocated
 */
u32 ParamTableManager::allocate(BgmapSprite bgmapSprite)
{
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
 * @param bgmapSprite	Sprite of which param table space will be freed
 */
void ParamTableManager::free(BgmapSprite bgmapSprite)
{
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
 * @return 	True if defragmentation took place
 */
bool ParamTableManager::defragmentProgressively()
{
	if(this->paramTableFreeData.param)
	{
		VirtualNode node = this->bgmapSprites->head;

		for(; node; node = node->next)
		{
			BgmapSprite sprite = BgmapSprite::safeCast(node->data);

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
 * @param x				Camera x coordinate
 * @param y				Camera y coordinate
 */
void ParamTableManager::print(int x, int y)
{
	int xDisplacement = 11;

	Printing::text(Printing::getInstance(), "PARAM TABLE STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Size:              ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->size, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Used:              ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->used - 1, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "ParamBase:          ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), this->paramTableBase, x + xDisplacement, y, 8, NULL);
	Printing::text(Printing::getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, 8, NULL);
}
