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
	Sprite sprites[__OBJECTLISTTAM];			\
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
	
	for ( i = 0; i < __OBJECTLISTTAM; i++){
		
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
	for(i = 0; i < __OBJECTLISTTAM - 1 &&  this->sprites[i + 1]; i++){

		DrawSpec drawSpec = Sprite_getDrawSpec(this->sprites[i]);

		int j = 0;
		for(j = i + 1;j < __OBJECTLISTTAM &&  this->sprites[j]; j++){

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

	if(!this->needSorting){

		return;
	}
	
	int i = 0;

	DrawSpec drawSpec = Sprite_getDrawSpec(this->sprites[0]);

	CACHE_ENABLE;
	for(;i < __OBJECTLISTTAM - 1 &&  this->sprites[i + 1]; i++){

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
			
			// wait for frame before rendering
			//VPUManager_waitForFrame(VPUManager_getInstance());

			Sprite_render(this->sprites[i]);
			Sprite_render(this->sprites[i + 1]);

			// enable interrupts
			//VPUManager_displayOn(VPUManager_getInstance());

			break;
		}
		
		drawSpec = nextDrawSpec;
	}
	CACHE_DISABLE;
	
	if(!(i < __OBJECTLISTTAM - 1 &&  this->sprites[i + 1])){

		this->needSorting = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_addSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::addSprite: null this");

	int i = 0;
	
	// find the last render object's index
	for(; this->sprites[i] && i < __OBJECTLISTTAM; i++);
	
	if(i < __OBJECTLISTTAM){
		
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
	
	CACHE_ENABLE;
	
	// search for the entity to remove
	for(; this->sprites[i] != sprite && i < __OBJECTLISTTAM; i++);

	// if found
	if(i < __OBJECTLISTTAM){
		
		int j = i;
		// must render the whole entities after the entity to be removed twice
		// to avoid flickering
		for(; this->sprites[j + 1] && j < __OBJECTLISTTAM - 1; j++){
			
			this->sprites[j] = this->sprites[j + 1];
			
			// set layer
			Sprite_setWorldLayer(this->sprites[j], __TOTAL_LAYERS - j);
			
			Sprite_render(this->sprites[j]);
		}
		
		// remove object from list
		this->sprites[j] = NULL;
	}
	CACHE_DISABLE;

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

	Printing_render(this->freeLayer);

	WORLD_HEAD((this->freeLayer - 1), WRLD_OFF);

    WORLD_SIZE((this->freeLayer - 1), 0, 0);
	
	WORLD_HEAD((this->freeLayer - 1), WRLD_END);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render sprites
void SpriteManager_render(SpriteManager this){

	ASSERT(this, "SpriteManager::render: null this");

	int i = 0;
	
	for(i = 0; this->sprites[i] && i < __OBJECTLISTTAM; i++){
		
		//render sprite	
		Sprite_render((Sprite)this->sprites[i]);
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve free layer
int SpriteManager_getFreeLayer(SpriteManager this){
	
	ASSERT(this, "SpriteManager::getFreeLayer: null this");

	return this->freeLayer;
}