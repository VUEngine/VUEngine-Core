/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


// ---------------------------------------------------------------------------------------------------------
// 												INCLUDES
// ---------------------------------------------------------------------------------------------------------

#include <ParamTableManager.h>
#include <HardwareManager.h>

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

#define ParamTableManager_ATTRIBUTES											\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* total size of param table */												\
	u32 size;																	\
																				\
	/* number of used bytes */													\
	u32 used;																	\
																				\
	/* allocated bSprites */													\
	VirtualList bSprites;														\
																				\
	/* removed bSprites' sizes */												\
	VirtualList removedBgmapSpritesSizes;											\
																				\
	/* user for defragmentation */												\
	ParamTableFreeData paramTableFreeData;										\
																				\
	/* user for defragmentation */												\
	BgmapSprite previouslyMovedBgmapSprite;												\
	
__CLASS_DEFINITION(ParamTableManager, Object);


// ---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
// ---------------------------------------------------------------------------------------------------------

void ParamTableManager_constructor(ParamTableManager this);
static int ParamTableManager_calculateSize(ParamTableManager this, BgmapSprite bSprite);

// ---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
// ---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(ParamTableManager);

//class constructor
void ParamTableManager_constructor(ParamTableManager this)
{
	__CONSTRUCT_BASE();

	this->bSprites = __NEW(VirtualList);
	this->removedBgmapSpritesSizes = __NEW(VirtualList);
	this->previouslyMovedBgmapSprite = NULL;
	
	ParamTableManager_reset(this);
}

// class destructor
void ParamTableManager_destructor(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::destructor: null this");

	ParamTableManager_reset(this);
	
	__DELETE(this->bSprites);
	this->bSprites = NULL;
	
	__DELETE(this->removedBgmapSpritesSizes);
	this->removedBgmapSpritesSizes = NULL;
	
	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void ParamTableManager_reset(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::reset: null this");

	VirtualList_clear(this->bSprites);
	
	VirtualNode node = VirtualList_begin(this->removedBgmapSpritesSizes);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__DELETE_BASIC(VirtualNode_getData(node));
	}
	
	VirtualList_clear(this->removedBgmapSpritesSizes);
		
	// set the size of the paramtable
	this->size = __PARAM_TABLE_END - __PARAM_BASE;

	NM_ASSERT(0 < __PARAM_TABLE_END - __PARAM_BASE, "ParamTableManager::reset: param table size is negative");

	// all the memory is free
	this->used = 1;
	
	this->paramTableFreeData.param = 0;
	this->paramTableFreeData.recoveredSize = 0;
	this->previouslyMovedBgmapSprite = NULL;
}

// calculate size of param table
static int ParamTableManager_calculateSize(ParamTableManager this, BgmapSprite bSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bSprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row = sprite's rows * 2 ^ 7
	// add one row as padding to make sure not ovewriting take place
	return (((int)Texture_getRows(Sprite_getTexture(__GET_CAST(Sprite, bSprite))) + __PARAM_TABLE_PADDING) << 7) * __MAXIMUM_SCALE;
}

// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, BgmapSprite bSprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(bSprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	int size = ParamTableManager_calculateSize(this, bSprite);
	
	//if there is space in the param table, allocate
	if(__PARAM_DISPLACEMENT((this->used + size)) < (__PARAM_TABLE_END))
	{
		//set sprite param
		BgmapSprite_setParam(bSprite, this->used);
		
		//record sprite
		VirtualList_pushBack(this->bSprites, bSprite);
		
		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		return true;
	}

	Printing_text(Printing_getInstance(), "Total size: ", 20, 7, NULL);
	Printing_int(Printing_getInstance(), __PARAM_TABLE_END - (int)__PARAM_BASE, 20 + 19, 7, NULL);
	
	NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");

	return false;
}

// deallocate param table space
void ParamTableManager_free(ParamTableManager this, BgmapSprite bSprite)
{
	ASSERT(this, "ParamTableManager::free: null this");
	ASSERT(VirtualList_find(this->bSprites, bSprite), "ParamTableManager::free: sprite not found");

	VirtualList_removeElement(this->bSprites, bSprite);

	if(this->previouslyMovedBgmapSprite == bSprite)
	{
		this->previouslyMovedBgmapSprite = NULL;
	}
	
	// accounted for
	if(this->paramTableFreeData.param && this->paramTableFreeData.param <= BgmapSprite_getParam(bSprite))
	{
		// but increase the space recovered
		this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSize(this, bSprite);

		return;
	}
	
	this->paramTableFreeData.param = BgmapSprite_getParam(bSprite);
	this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSize(this, bSprite);
}

// relocate bSprites
bool ParamTableManager_processRemovedSprites(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::processRemoved: null this");
	
	if(this->paramTableFreeData.param)
	{
		VirtualNode node = VirtualList_begin(this->bSprites);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			BgmapSprite sprite = __GET_CAST(BgmapSprite, VirtualNode_getData(node));
	
			u32 spriteParam = BgmapSprite_getParam(sprite);
			
			// retrieve param
			if(spriteParam > this->paramTableFreeData.param)
			{
				int size = ParamTableManager_calculateSize(this, sprite);

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
	Printing_hex(Printing_getInstance(), __PARAM_BASE, x + xDisplacement, y, NULL);
	Printing_text(Printing_getInstance(), "ParamEnd:           ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, NULL);
}