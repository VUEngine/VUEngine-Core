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

__CLASS_DEFINITION(ParamTableManager);


typedef struct ParamTableFreeData
{
	u32 param;
	u32 size;
	u32 recoveredSize;
} ParamTableFreeData;


// ---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
// ---------------------------------------------------------------------------------------------------------

static void ParamTableManager_constructor(ParamTableManager this);


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
	this->size = __PARAMEND - __PARAMINI;
	
	// all the memory is free
	this->used = 1;
}

// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, Sprite sprite)
{
	ASSERT(this, "ParamTableManager::allocate: null this");
	NM_ASSERT(sprite, "ParamTableManager::allocate: null sprite");

	//calculate necessary space to allocate
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row
	int size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAM_SPACE_FACTOR));

	//if there is space in the param table, allocate
	if (PARAM((this->used + size)) < __PARAMEND)
	{
		//set sprite param
		Sprite_setParam(sprite, this->used);
		
		//record sprite
		VirtualList_pushBack(this->sprites, sprite);
		
		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		NM_ASSERT(PARAMBase + this->used < WAMBase, "ParamTableManager::allocate: exceded memory area");
		
		return true;
	}
	
	NM_ASSERT(false, "ParamTableManager::allocate: memory depleted");

	return false;
}

// deallocate param table space
void ParamTableManager_free(ParamTableManager this, Sprite sprite)
{
	ASSERT(this, "ParamTableManager::free: null this");
	ASSERT(VirtualList_find(this->sprites, sprite), "ParamTableManager::free: sprite not found");

	ParamTableFreeData* paramTableFreeData = __NEW_BASIC(ParamTableFreeData);
	paramTableFreeData->size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAM_SPACE_FACTOR));
	paramTableFreeData->param = Sprite_getParam(sprite);
	paramTableFreeData->recoveredSize = paramTableFreeData->size;
	
	VirtualList_removeElement(this->sprites, sprite);
	VirtualList_pushBack(this->removedSpritesSizes, paramTableFreeData);
}

// relocate sprites
bool ParamTableManager_processRemovedSprites(ParamTableManager this)
{
	ASSERT(this, "ParamTableManager::processRemoved: null this");
	// for each sprite using param table space reassign them their param table starting point.
	VirtualNode node = VirtualList_begin(this->removedSpritesSizes);

	for (; node; node = VirtualNode_getNext(node))
	{
		ParamTableFreeData* paramTableFreeData = (ParamTableFreeData*)VirtualNode_getData(node);

		//calculate necessary space to allocate
		VirtualNode auxNode = VirtualList_begin(this->sprites);

		for (; auxNode; auxNode = VirtualNode_getNext(auxNode))
		{
			Sprite auxSprite = (Sprite)VirtualNode_getData(auxNode);

			u32 auxParam = Sprite_getParam(auxSprite);

			// retrieve param
			if (auxParam > paramTableFreeData->param)
			{
				//move back paramSize bytes
				Sprite_setParam(auxSprite, paramTableFreeData->param);

				//create an independant of software variable to point XPSTTS register
				unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

				//wait for screen to idle
				while (*xpstts & XPBSYR);

				// scale now
				Sprite_scale(auxSprite);

				//wait for screen to idle
				while (*xpstts & XPBSYR);
				// render now
				Sprite_render(auxSprite);

				// set the new param and size to move on the next cycle
				paramTableFreeData->size = (((int)Texture_getTotalRows(Sprite_getTexture(auxSprite))) << (7 + __PARAM_SPACE_FACTOR));
				paramTableFreeData->param += paramTableFreeData->size;
				break;
			}
		}
		
		if (!auxNode)
		{
			//recover space
			this->used -= paramTableFreeData->recoveredSize;
			this->size += paramTableFreeData->recoveredSize;
			
			VirtualList_removeElement(this->removedSpritesSizes, paramTableFreeData);

			__DELETE_BASIC(paramTableFreeData);
			return true;
		}
	}
	
	return false;
}

// print param table's attributes state
void ParamTableManager_print(ParamTableManager this, int x, int y)
{
	ASSERT(this, "ParamTableManager::print: null this");

	int i = 0;
	
	Printing_text(Printing_getInstance(), "PARAM TABLE'S STATUS", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Size:", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->size, x + 6, y, NULL);
	
	Printing_text(Printing_getInstance(), "Used:", x, y + 1, NULL);
	Printing_int(Printing_getInstance(), this->used, x + 6, y + 1, NULL);
	
	VirtualNode node = VirtualList_begin(this->sprites);
	
	for (; node; node = VirtualNode_getNext(node))
	{
		Printing_hex(Printing_getInstance(), (int)VirtualNode_getData(node), x, y + i + 3, NULL);
	}
}