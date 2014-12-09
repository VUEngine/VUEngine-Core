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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <ParamTableManager.h>
#include <HardwareManager.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define ParamTableManager_ATTRIBUTES											\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* total size of param table */												\
	int size;																	\
																				\
	/* number of used bytes */													\
	int used;																	\
																				\
	/* allocated sprites */														\
	VirtualList sprites;														\

__CLASS_DEFINITION(ParamTableManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class constructor 
static void ParamTableManager_constructor(ParamTableManager this);

// register a sprite
static void ParamTableManager_registerSprite(ParamTableManager this, Sprite sprite);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a singleton
__SINGLETON(ParamTableManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class constructor 
static void ParamTableManager_constructor(ParamTableManager this){

	__CONSTRUCT_BASE(Object);

	this->sprites = NULL;
	

	ParamTableManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class destructor
void ParamTableManager_destructor(ParamTableManager this){

	ASSERT(this, "ParamTableManager::destructor: null this");

	if(this->sprites) {
		
		__DELETE(this->sprites);
		
		this->sprites = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void ParamTableManager_reset(ParamTableManager this){

	ASSERT(this, "ParamTableManager::reset: null this");

	if(this->sprites) {
		
		__DELETE(this->sprites);
		
		this->sprites = NULL;
	}
	
	this->sprites = __NEW(VirtualList);
		
	// set the size of the paramtable
	this->size = __PARAMEND - __PARAMINI;
	
	// all the memory is free
	this->used = 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, Sprite sprite){
	
	ASSERT(this, "ParamTableManager::allocate: null this");

	int size = 0;

	//calculate necesary space to allocate	
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row
	size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAM_SPACE_FACTOR));

	//if there is space in the param table, allocate
	if(PARAM((this->used + size)) < __PARAMEND){
		
		//set sprite param
		Sprite_setParam(sprite, this->used);
		
		//record sprite
		ParamTableManager_registerSprite(this, sprite);
		
		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		ASSERT(PARAMBase + this->used < WAMBase, "ParamTableManager::allocate: exceded memory area");
		
		return true;
	}
	
	ASSERT(false, "ParamTableManager::allocate: memory depleted");

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// record allocated sprite
static void ParamTableManager_registerSprite(ParamTableManager this, Sprite sprite){
	
	ASSERT(this, "ParamTableManager::setObject: null this");
	ASSERT(sprite, "ParamTableManager::setObject: null sprite");

	if(sprite) {

		VirtualList_pushBack(this->sprites, sprite);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// deallocate param table space
void ParamTableManager_free(ParamTableManager this, Sprite sprite){
	
	ASSERT(this, "ParamTableManager::free: null this");

	int i = 0;
	u32 auxParam = 0;
	u32 size = 0;
	u32 param = Sprite_getParam(sprite);
	
	//calculate necesary space to allocate	
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row
	size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAM_SPACE_FACTOR));

	//recover space
	this->used -= size;
	this->size += size;

	/* for each sprite using param table space
	 * reasign them their param table start
	 * point.
	 */
	VirtualNode node = VirtualList_find(this->sprites, sprite);

	ASSERT(node, "ParamTableManager::free: null node");
	
	node = VirtualNode_getNext(node);
	
	VirtualList_removeElement(this->sprites, sprite);
	
	for(; node; node = VirtualNode_getNext(node)){

		Sprite auxSprite = (Sprite)VirtualNode_getData(node);

		// retrieve param
		auxParam = Sprite_getParam(auxSprite);

		//move back paramSize bytes
		Sprite_setParam(auxSprite, auxParam - size);
							
		// and force render
		Sprite_render(auxSprite);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print param table's attributes state
void ParamTableManager_print(ParamTableManager this, int x, int y){
	
	ASSERT(this, "ParamTableManager::print: null this");

	int i = 0;
	
	Printing_text("PARAM TABLE'S STATUS", x, y++);
	Printing_text("Size:", x, ++y);
	Printing_int(this->size, x + 6, y);
	
	Printing_text("Used:", x, y + 1);
	Printing_int(this->used, x + 6, y + 1);
	
	VirtualNode node = VirtualList_begin(this->sprites);
	
	for(; node; node = VirtualNode_getNext(node)){

		Printing_hex((int)VirtualNode_getData(node), x, y + i + 3);
		//printInt((int)this->sprites[i]->param,x+10,y+i+3);
	}
}

