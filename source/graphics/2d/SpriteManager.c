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

#include <SpriteManager.h>
#include <VPUManager.h>
#include <Screen.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define SpriteManager_ATTRIBUTES				\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* list of sprites to render */				\
	Sprite sprites[__SPRITE_LIST_SIZE];			\
												\
	/* next world layer	*/						\
	int freeLayer;								\
												\
	/* flag controls END layer	*/				\
	u8 needSorting;


__CLASS_DEFINITION(SpriteManager);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void SpriteManager_constructor(SpriteManager this);

// set free layers off
static void SpriteManager_setLastLayer(SpriteManager this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S ATTRIBUTES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__SINGLETON(SpriteManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void SpriteManager_constructor(SpriteManager this){

	// construct base object
	__CONSTRUCT_BASE(Object);

	SpriteManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void SpriteManager_destructor(SpriteManager this){
	
	ASSERT(this, "SpriteManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void SpriteManager_reset(SpriteManager this){

	ASSERT(this, "SpriteManager::reset: null this");

	int i = 0;
	
	for ( i = 0; i < __SPRITE_LIST_SIZE; i++){
		
		this->sprites[i] = NULL;
	}
	
	this->freeLayer = __TOTAL_LAYERS;
	this->needSorting = false;
	
	SpriteManager_setLastLayer(this);
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_sortAllLayers(SpriteManager this){

	ASSERT(this, "SpriteManager::sortAllLayers: null this");

	int i = 0;

	CACHE_ENABLE;
	for(i = 0; i < __SPRITE_LIST_SIZE - 1 &&  this->sprites[i + 1]; i++){

		DrawSpec drawSpec = Sprite_getDrawSpec(this->sprites[i]);

		int j = 0;
		for(j = i + 1; j < __SPRITE_LIST_SIZE && this->sprites[j]; j++){

			DrawSpec nextDrawSpec = Sprite_getDrawSpec(this->sprites[j]);

			// check if z positions are swaped
			if(nextDrawSpec.position.z > drawSpec.position.z){
				
				// get each entity's layer
				int worldLayer1 = Sprite_getWorldLayer(this->sprites[i]);
				int worldLayer2 = Sprite_getWorldLayer(this->sprites[j]);
				
				// swap array entries
				Sprite auxSprite = this->sprites[i];			
				this->sprites[i] = this->sprites[j];			
				this->sprites[j] = auxSprite;
	
				// swap layers
				Sprite_setWorldLayer(this->sprites[i], worldLayer1);
				Sprite_setWorldLayer(this->sprites[j], worldLayer2);
			}
			
			drawSpec = nextDrawSpec;
		}
	}
	CACHE_DISABLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_spriteChangedPosition(SpriteManager this){

	ASSERT(this, "SpriteManager::spriteChangedPosition: null this");

	this->needSorting = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_sortLayersProgressively(SpriteManager this){

	ASSERT(this, "SpriteManager::sortLayersProgressively: null this");
	//ASSERT(this->sprites[0], "SpriteManager::sortLayersProgressively: null this->sprites[0]");

	if(!this->needSorting || !this->sprites[0]){

		return;
	}
	
	int i = 0;

	DrawSpec drawSpec = Sprite_getDrawSpec(this->sprites[0]);

	CACHE_ENABLE;
	for(;i < __SPRITE_LIST_SIZE - 1; i++){

		if(!this->sprites[i] || !this->sprites[i + 1]) {
			
			continue;
		}
		
		DrawSpec nextDrawSpec = Sprite_getDrawSpec(this->sprites[i + 1]);

		// check if z positions are swaped
		if(nextDrawSpec.position.z > drawSpec.position.z){
			
			// get each entity's layer
			int worldLayer1 = Sprite_getWorldLayer(this->sprites[i]);
			int worldLayer2 = Sprite_getWorldLayer(this->sprites[i + 1]);
			
			// swap array entries
			Sprite auxSprite = this->sprites[i];			
			this->sprites[i] = this->sprites[i + 1];			
			this->sprites[i + 1] = auxSprite;

			// swap layers
			Sprite_setWorldLayer(this->sprites[i], worldLayer1);
			Sprite_setWorldLayer(this->sprites[i + 1], worldLayer2);
			
			Sprite_render(this->sprites[i]);
			Sprite_render(this->sprites[i + 1]);
			break;
		}
		
		drawSpec = nextDrawSpec;
	}
	CACHE_DISABLE;
	
	if(!(i < __SPRITE_LIST_SIZE - 1 &&  this->sprites[i + 1])){

		this->needSorting = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_addSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::addSprite: null this");

	VPUManager_disableInterrupt(VPUManager_getInstance());

	int i = 0;
	// find the last render object's index
	for(; this->sprites[i] && i < __SPRITE_LIST_SIZE; i++);
	
	if(i < __SPRITE_LIST_SIZE){
		
		//SpriteManager_alignSameLayerSprites(this, sprite);

		// set entity into slot
		this->sprites[i] = sprite;

		// set layer
		Sprite_setWorldLayer(this->sprites[i], __TOTAL_LAYERS - i);
		
	    WORLD_SIZE((this->freeLayer), 0, 0);
		
		// don't allow flickering in the next render cycly
		this->freeLayer = __TOTAL_LAYERS - i - 1;

		SpriteManager_setLastLayer(this);

		ASSERT(this->freeLayer, "SpriteManager::addSprite: no more free layers" );

		this->needSorting = true;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::removeSprite: null this");

	int i = 0;

	VPUManager_disableInterrupt(VPUManager_getInstance());

	// search for the entity to remove
	for(; this->sprites[i] != sprite && i < __SPRITE_LIST_SIZE; i++);

	// if found
	if(i < __SPRITE_LIST_SIZE){
		
		int j = i;
		
		// must render the whole entities after the entity to be removed twice
		// to avoid flickering
		for(; this->sprites[j + 1] && j < __SPRITE_LIST_SIZE - 1; j++){
			
			this->sprites[j] = this->sprites[j + 1];

			// set layer
			Sprite_setWorldLayer(this->sprites[j], __TOTAL_LAYERS - j);
			
			Sprite_render(this->sprites[j]);
		}
		
		// remove object from list
		this->sprites[j] = NULL;
	}

	this->freeLayer++;
	
	ASSERT(__TOTAL_LAYERS >= this->freeLayer, "SpriteManager::removeSprite: more free layers than really available" );

	this->needSorting = true;
	
	SpriteManager_setLastLayer(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set free layers off
static void SpriteManager_setLastLayer(SpriteManager this){

	ASSERT(this, "SpriteManager::setLastLayer: null this");

	//create an independant of software variable to point XPSTTS register
	unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

	//wait for screen to idle
	while (*xpstts & XPBSYR);

	int printingLayer = __TOTAL_LAYERS == this->freeLayer? this->freeLayer - 1: this->freeLayer;
	printingLayer = 0 > printingLayer? 1: printingLayer;

	Printing_render(printingLayer);

	WORLD_HEAD((printingLayer - 1), WRLD_OFF);

    WORLD_SIZE((printingLayer - 1), 0, 0);
	
	WORLD_HEAD((printingLayer - 1), WRLD_END);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render sprites
void SpriteManager_render(SpriteManager this){

	ASSERT(this, "SpriteManager::render: null this");

	int i = 0;
	
	VPUManager_disableInterrupt(VPUManager_getInstance());
	
	for(i = 0; this->sprites[i] && i < __SPRITE_LIST_SIZE; i++){
		
		//render sprite	
		Sprite_render((Sprite)this->sprites[i]);
	}	
	
	VPUManager_enableInterrupt(VPUManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve free layer
int SpriteManager_getFreeLayer(SpriteManager this){
	
	ASSERT(this, "SpriteManager::getFreeLayer: null this");

	return this->freeLayer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show a given layer
void SpriteManager_showLayer(SpriteManager this, int layer) {
	
	ASSERT(this, "SpriteManager::showLayer: null this");

	int i = 0;
	for(; this->sprites[i] && i < __SPRITE_LIST_SIZE; i++){
		
		if(Sprite_getWorldLayer(this->sprites[i]) != layer) {
			
			Sprite_hide(this->sprites[i]);
		}
		else {
			
			Sprite_show(this->sprites[i]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show all layers
void SpriteManager_recoverLayers(SpriteManager this) {
	
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	int i = 0;
	for(; this->sprites[i] && i < __SPRITE_LIST_SIZE; i++){
		
		Sprite_show(this->sprites[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print status
void SpriteManager_print(SpriteManager this, int x, int y){
	
	ASSERT(this, "SpriteManager::print: null this");

	int spritesCount = 0;
	Printing_text("SPRITES' USAGE", x, y++);
	Printing_text("Free layers: ", x, ++y);
	Printing_int(this->freeLayer, x + 15, y);
	Printing_text("Sprites count: ", x, ++y);

	int i = 0;
	int auxY = y + 2;
	int auxX = x;
	for(; i < __SPRITE_LIST_SIZE; i++){
	
		if(this->sprites[i]){
			
			spritesCount++;
		}
		
		Printing_text("Sprite: ", auxX, auxY);
		Printing_int(i, auxX + 8, auxY);
		Printing_hex((int)this->sprites[i], auxX + 11, auxY);
		
		if(28 <= ++auxY) {
			
			auxY = y + 2;
			auxX += 25;
		}
	}

	Printing_int(spritesCount, x + 15, y);
}