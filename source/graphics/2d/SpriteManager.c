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

#define SpriteManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* list of sprites to render */												\
	VirtualList sprites;														\
																				\
	/* list of sprites to render */												\
	VirtualList removedSprites;													\
																				\
	/* sorting nodes	*/														\
	VirtualNode node;															\
	VirtualNode otherNode;														\
																				\
	/* next world layer	*/														\
	s8 freeLayer;																\
																				\
	/* flag controls END layer	*/												\
	u8 reverseRendering;																\

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

	this->node = NULL;
	this->otherNode = NULL;
	
	this->sprites = NULL;
	this->removedSprites = NULL;
	
	SpriteManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void SpriteManager_destructor(SpriteManager this){
	
	ASSERT(this, "SpriteManager::destructor: null this");

	if(this->sprites){
		
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	if(this->removedSprites){
		
		__DELETE(this->removedSprites );
		this->removedSprites  = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void SpriteManager_reset(SpriteManager this){

	ASSERT(this, "SpriteManager::reset: null this");

	if(this->sprites){
		
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	if(this->removedSprites ){
		
		__DELETE(this->removedSprites );
		this->removedSprites  = NULL;
	}

	this->sprites = __NEW(VirtualList);
	this->removedSprites = __NEW(VirtualList);

	this->freeLayer = __TOTAL_LAYERS - 1;
	this->reverseRendering = false;
	
	this->node = NULL;
	this->otherNode = NULL;
	
	SpriteManager_setLastLayer(this);
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_spriteChangedPosition(SpriteManager this){

	ASSERT(this, "SpriteManager::spriteChangedPosition: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_sortLayers(SpriteManager this, int progressively){

	ASSERT(this, "SpriteManager::sortLayers: null this");

	CACHE_ENABLE;
	this->node = progressively && this->node? this->otherNode? this->node: VirtualNode_getNext(this->node): VirtualList_begin(this->sprites);

	for(; this->node; this->node = VirtualNode_getNext(this->node)) {
		
		Sprite sprite = (Sprite)VirtualNode_getData(this->node);
		DrawSpec drawSpec = Sprite_getDrawSpec(sprite);

		this->otherNode = progressively && this->otherNode? VirtualNode_getNext(this->otherNode): VirtualNode_getNext(this->node);

		for(; this->otherNode; this->otherNode = VirtualNode_getNext(this->otherNode)) {

			Sprite otherSprite = (Sprite)VirtualNode_getData(this->otherNode);
			DrawSpec otherDrawSpec = Sprite_getDrawSpec(otherSprite);
	
			// check if z positions are swaped
			if(otherDrawSpec.position.z < drawSpec.position.z){
				
				// get each entity's layer
				u8 worldLayer1 = Sprite_getWorldLayer(sprite);
				u8 worldLayer2 = Sprite_getWorldLayer(otherSprite);
	
				// swap layers
				Sprite_setWorldLayer(sprite, worldLayer2);
				Sprite_setWorldLayer(otherSprite, worldLayer1);
	
				// swap array entries
				VirtualNode_swapData(this->node, this->otherNode);
				
				this->node = this->otherNode;

				if(!progressively){
					
					// make sure sort is complete
					this->node = VirtualList_begin(this->sprites);
					break;
				}
			}
		}	
		
		if(progressively && !this->otherNode){

			break;
		}
	}
	
	CACHE_DISABLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_addSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::addSprite: null this");

	// add to the front: last element corresponde to the 31 WORLD
	VirtualList_pushFront(this->sprites, sprite);
	u8 layer = __TOTAL_LAYERS - VirtualList_getSize(this->sprites);
	Sprite_setWorldLayer(sprite, layer);
	SpriteManager_setLastLayer(this);
	
	// render from the front of the list so the
	// first object to be updated is the new one
	this->reverseRendering = true;
	
	// this will force the sorting algoritm to take care
	// first of the new sprite
	this->node = NULL;
	this->otherNode = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::removeSprite: null this");

	VirtualNode node = VirtualList_find(this->sprites, sprite);

	ASSERT(node, "SpriteManager::removeSprite: sprite not found");

	if(node) {
	
		Sprite_hide(sprite);

		node = VirtualNode_getPrevious(node);
		VirtualList_removeElement(this->sprites, sprite);
		
		CACHE_ENABLE;
		for(; node; node = VirtualNode_getPrevious(node)){
			
			Sprite previousSprite = (Sprite)VirtualNode_getData(node);
			u8 layer = Sprite_getWorldLayer(previousSprite);
			Sprite_setWorldLayer(previousSprite, layer + 1);
			//Sprite_render(previousSprite);
			this->reverseRendering = false;
		}
		CACHE_DISABLE;
		
		// force printing WORLD to be
		// setup in the next rendering cycle
		this->freeLayer = -1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set free layers off
static void SpriteManager_setLastLayer(SpriteManager this){

	ASSERT(this, "SpriteManager::setLastLayer: null this");

	this->freeLayer = (__TOTAL_LAYERS - 1) - VirtualList_getSize(this->sprites);
	this->freeLayer = 0 <= this->freeLayer? this->freeLayer: 1;
	
	Printing_render(this->freeLayer);

	if(0 < this->freeLayer) {
		
		//create an independant of software variable to point XPSTTS register
		unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

		//wait for screen to idle
		while (*xpstts & XPBSYR);

		WORLD_HEAD((this->freeLayer - 1), WRLD_OFF);
	
	    WORLD_SIZE((this->freeLayer - 1), 0, 0);
		
		WORLD_HEAD((this->freeLayer - 1), WRLD_END);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render sprites
void SpriteManager_render(SpriteManager this){

	ASSERT(this, "SpriteManager::render: null this");

	// sort sprites
	SpriteManager_sortLayers(this, true);

	// render from WORLD 31 to the lowes active one
	// reverse this order when a new sprite was added
	// to make effective its visual properties as quick as 
	// possible
	if(this->reverseRendering) {
		
		this->reverseRendering = false;
		
		VirtualNode node = VirtualList_begin(this->sprites);
		
		for(; node; node = VirtualNode_getNext(node)){
	
			Sprite_render((Sprite)VirtualNode_getData(node));
		}
	}
	else {
		
		VirtualNode node = VirtualList_end(this->sprites);
		
		for(; node; node = VirtualNode_getPrevious(node)){

			Sprite_render((Sprite)VirtualNode_getData(node));
		}	
	}
	
	if(0 > this->freeLayer){
	
		SpriteManager_setLastLayer(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve free layer
int SpriteManager_getFreeLayer(SpriteManager this){
	
	ASSERT(this, "SpriteManager::getFreeLayer: null this");

	return this->freeLayer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show a given layer
void SpriteManager_showLayer(SpriteManager this, u8 layer) {
	
	ASSERT(this, "SpriteManager::showLayer: null this");
	
	VirtualNode node = VirtualList_end(this->sprites);
	
	for(; node; node = VirtualNode_getPrevious(node)){

		if(Sprite_getWorldLayer((Sprite)VirtualNode_getData(node)) != layer){
			
			Sprite_hide((Sprite)VirtualNode_getData(node));
		}
		else {
			
			Sprite_show((Sprite)VirtualNode_getData(node));
		}
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show all layers
void SpriteManager_recoverLayers(SpriteManager this) {
	
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	VirtualNode node = VirtualList_end(this->sprites);
	for(; node; node = VirtualNode_getPrevious(node)){

		Sprite_show((Sprite)VirtualNode_getData(node));
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print status
void SpriteManager_print(SpriteManager this, int x, int y){
	
	ASSERT(this, "SpriteManager::print: null this");

	Printing_text("SPRITES' USAGE", x, y++);
	Printing_text("Free layers: ", x, ++y);
	Printing_int(this->freeLayer, x + 15, y);
	Printing_text("Sprites count: ", x, ++y);

	int auxY = y + 2;
	int auxX = x;
	
	VirtualNode node = VirtualList_begin(this->sprites);
	
	for(; node; node = VirtualNode_getNext(node)){
	
		Sprite sprite = (Sprite)VirtualNode_getData(node);
		
		Printing_text("Sprite: ", auxX, auxY);
		Printing_int(Sprite_getWorldLayer(sprite), auxX + 8, auxY);
		Printing_text(__GET_CLASS_NAME(sprite), auxX + 11, auxY);
		
		if(28 <= ++auxY) {
			
			auxY = y + 2;
			auxX += 25;
		}
	}

	Printing_int(VirtualList_getSize(this->sprites), x + 15, y);
}