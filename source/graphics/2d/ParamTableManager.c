/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define ParamTableManager_ATTRIBUTES			\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* total size of param table */				\
	int size;									\
												\
	/* number of used bytes */					\
	int used;									\
												\
	/* allocated objects */						\
	Sprite sprites[__TOTALPARAMOBJECTS];
	
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
static void ParamTableManager_setObject(ParamTableManager this, Sprite sprite);


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

	ParamTableManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class destructor
void ParamTableManager_destructor(ParamTableManager this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void ParamTableManager_reset(ParamTableManager this){

	int i = 0;

	// clear the usage
	for(;i < __TOTALPARAMOBJECTS; i++){

		this->sprites[i] = NULL;
	}
	// set the size of the paramtable
	this->size = __PARAMEND - __PARAMINI;
	
	// all the memory is free
	this->used = 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate param table space for sprite
int ParamTableManager_allocate(ParamTableManager this, Sprite sprite){
	
	int size = 0;
	
	//calculate necesary space to allocate	
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row
	size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAMSPACEFACTOR));

	//if there is space in the param table, allocate
	if(PARAM((this->used + size)) < __PARAMEND){
		
		//set sprite param
		Sprite_setParam(sprite, this->used);
		
		//record sprite
		ParamTableManager_setObject(this, sprite);
		
		//update the param bytes ocupied
		this->size -= size;
		this->used += size;

		ASSERT(PARAMBase + this->used < WAMBase);
		
		return true;
	}
	
	ASSERT(false, PT_MEM_ERR);

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// record allocated sprite
static void ParamTableManager_setObject(ParamTableManager this, Sprite sprite){
	
	int i = 0;
	
	// search for and empty slot
	for(; i < __TOTALPARAMOBJECTS && this->sprites[i]; i++);
	
	ASSERT(i < __TOTALPARAMOBJECTS, PT_OBJDEP_ERR);

	// record sprite
	this->sprites[i] = sprite;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// deallocate param table space
void ParamTableManager_free(ParamTableManager this, Sprite sprite){
	
	int i = 0;
	u32 auxParam = 0;
	u32 size = 0;
	u32 param = Sprite_getParam(sprite);
	
	//calculate necesary space to allocate	
	//size = sprite's rows * 8 pixels each on * 16 bytes needed by each row
	size = (((int)Texture_getTotalRows(Sprite_getTexture(sprite))) << (7 + __PARAMSPACEFACTOR));

	//recover space
	this->used -= size;
	this->size += size;

	/* for each sprite using param table space
	 * reasign them their param table start
	 * point.
	 */

	for(i = 0; i < __TOTALPARAMOBJECTS; i++){
		
		// if there is a defined sprite
		if(this->sprites[i]){
			
			// if it is the sprite being removed, 
			if(this->sprites[i] == sprite){
			
				// free the sprite entry in the param table
				this->sprites[i] = NULL;
				
				continue;
			}

			// retrieve param
			auxParam = Sprite_getParam(this->sprites[i]);

			// if the sprite has a greater param table start point
			if(auxParam >= param){
			
				//move back paramSize bytes
				Sprite_setParam(this->sprites[i], auxParam - size);
				
				// render the sprite inmediately to update the WORLD
				Sprite_render(this->sprites[i]);
			}
		}	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print param table's attributes state
void ParamTableManager_print(ParamTableManager this,int x, int y){
	
	int i = 0;
	
	vbjPrintText("Size:", x, y);
	
	vbjPrintInt(this->size, x + 6,y);
	
	vbjPrintText("Used:", x, y + 1);
	
	vbjPrintInt(this->used, x + 6, y + 1);
	
	for(i = 0; i < 10; i++){
		vbjPrintHex((int)this->sprites[i], x, y + i + 3);
		//printInt((int)this->sprites[i]->param,x+10,y+i+3);
	}
}

