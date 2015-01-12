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
	/* allocated sprites */														\
	VirtualList sprites;														\
																				\
	/* removed sprites' sizes */												\
	VirtualList removedSpritesSizes;											\
																				\
	/* user for defragmentation */												\
	ParamTableFreeData paramTableFreeData;										\

__CLASS_DEFINITION(ParamTableManager);


// ---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
// ---------------------------------------------------------------------------------------------------------

static void ParamTableManager_constructor(ParamTableManager this);
static int ParamTableManager_calculateSize(ParamTableManager this, Sprite sprite);

// ---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
// ---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(ParamTableManager);

//class constructor
static void ParamTableManager_constructor(ParamTableManager this)
{
	__CONSTRUCT_BASE(Object);

	this->sprites = __NEW(VirtualList);
	this->removedSpritesSizes = __NEW(VirtualList);
	
	ParamTableManager_reset(this);
}

// class destructor
void ParamTableManager_destructor(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::destructor: null this");

	ParamTableManager_reset(this);
	
	__DELETE(this->sprites);
	this->sprites = NULL;
	
	__DELETE(this->removedSpritesSizes);
	this->removedSpritesSizes = NULL;

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// reset
void ParamTableManager_reset(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::reset: null this");

	VirtualList_clear(this->sprites);
	
	VirtualNode node = VirtualList_begin(this->removedSpritesSizes);
	
	for (; node; node = VirtualNode_getNext(node))
	{
		__DELETE_BASIC(VirtualNode_getData(node));
	}
	
	VirtualList_clear(this->removedSpritesSizes);
		
	// set the size of the paramtable
	int paramTableBase = (int)VPUManager_getParamBase(VPUManager_getInstance());
	this->size = __PARAM_TABLE_END - paramTableBase;

	NM_ASSERT(0 < __PARAM_TABLE_END - paramTableBase, "ParamTableManager::reset: param table size is negative");

	// all the memory is free
	this->used = 1;
	
	this->paramTableFreeData.param = 0;
	this->paramTableFreeData.recoveredSize = 0;
}

// calculate size of param table
static int ParamTableManager_calculateSize(ParamTableManager this, Sprite sprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(sprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row = sprite's rows * 2 ^ 7
	// add one row as padding to make sure not ovewriting take place
	return (((int)Texture_getTotalRows(Sprite_getTexture(sprite)) + 1) << 7) * __MAXIMUM_SCALE;
}

// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, Sprite sprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	ASSERT(sprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	int size = ParamTableManager_calculateSize(this, sprite);
	
	//if there is space in the param table, allocate
	if (VPUManager_getParamDisplacement(VPUManager_getInstance(), this->used + size) < __PARAM_TABLE_END)
	{
		//set sprite param
		Sprite_setParam(sprite, this->used);
		
		//record sprite
		VirtualList_pushBack(this->sprites, sprite);
		
		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		return true;
	}

	Printing_text(Printing_getInstance(), "Total size: ", 20, 7, NULL);
	Printing_int(Printing_getInstance(), __PARAM_TABLE_END - (int)VPUManager_getParamBase(VPUManager_getInstance()), 20 + 19, 7, NULL);
	
	NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");

	return false;
}

// deallocate param table space
void ParamTableManager_free(ParamTableManager this, Sprite sprite)
{
	ASSERT(this, "ParamTableManager::free: null this");
	ASSERT(VirtualList_find(this->sprites, sprite), "ParamTableManager::free: sprite not found");

	VirtualList_removeElement(this->sprites, sprite);

	// accounted for
	if(this->paramTableFreeData.param && this->paramTableFreeData.param <= Sprite_getParam(sprite))
	{
		// but increase the space recovered
		this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSize(this, sprite);

		return;
	}
	
	this->paramTableFreeData.param = Sprite_getParam(sprite);
	this->paramTableFreeData.recoveredSize += ParamTableManager_calculateSize(this, sprite);
}

// relocate sprites
bool ParamTableManager_processRemovedSprites(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::processRemoved: null this");
	
	if(this->paramTableFreeData.param)
	{
		VirtualNode node = VirtualList_begin(this->sprites);
		
		for (; node; node = VirtualNode_getNext(node))
		{
			Sprite sprite = (Sprite)VirtualNode_getData(node);
	
			u32 spriteParam = Sprite_getParam(sprite);
	
			// retrieve param
			if (spriteParam > this->paramTableFreeData.param)
			{
				//move back paramSize bytes
				Sprite_setParam(sprite, this->paramTableFreeData.param);
	
				//create an independant of software variable to point XPSTTS register
				unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

				//wait for screen to idle
				while (*xpstts & XPBSYR);

				// scale now
				Sprite_scale(sprite);
	
				// render now
				__VIRTUAL_CALL(void, Sprite, render, sprite);

				// set the new param to move on the next cycle
				this->paramTableFreeData.param += ParamTableManager_calculateSize(this, sprite);

				break;
			}
		}
			
		if (!node)
		{
			//recover space
			this->used -= this->paramTableFreeData.recoveredSize;
			this->size += this->paramTableFreeData.recoveredSize;
			
			this->paramTableFreeData.param = 0;
			this->paramTableFreeData.recoveredSize = 0;
			
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
	Printing_text(Printing_getInstance(), "Size:", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->size, x + xDisplacement, y, NULL);
	
	Printing_text(Printing_getInstance(), "Used:", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->used, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "ParamBase:", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), VPUManager_getParamBase(VPUManager_getInstance()), x + xDisplacement, y, NULL);
	Printing_text(Printing_getInstance(), "ParamEnd:", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), __PARAM_TABLE_END, x + xDisplacement, y, NULL);

	VirtualNode node = VirtualList_begin(this->sprites);
	++y;
	Printing_text(Printing_getInstance(), "Param values:", x, y++, NULL);

	int i = 0;
	xDisplacement = 0;
	
	for (; node; node = VirtualNode_getNext(node), i++)
	{
		Printing_hex(Printing_getInstance(), (int)VirtualNode_getData(node), x + xDisplacement, y + i, NULL);

		if(y + i + 1 >= __SCREEN_HEIGHT >> 3)
		{
			i++;
			i = 0;
			xDisplacement += 10;
		}
	}
}