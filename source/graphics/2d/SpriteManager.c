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
	/* next world layer	*/														\
	int freeLayer;																\
																				\
	/* flag controls END layer	*/												\
	u8 needSorting;																\
																				\
	/* sorting nodes	*/														\
	VirtualNode node;															\
	VirtualNode otherNode;														\

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

	this->freeLayer = __TOTAL_LAYERS;
	this->needSorting = false;
	
	this->node = NULL;
	this->otherNode = NULL;
	
	SpriteManager_setLastLayer(this);
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_spriteChangedPosition(SpriteManager this){

	ASSERT(this, "SpriteManager::spriteChangedPosition: null this");

	this->needSorting = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_sortLayers(SpriteManager this, int progressively){

	ASSERT(this, "SpriteManager::sortLayers: null this");

	/*
	if(!this->needSorting && progressively){

		return;
	}
*/
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
				
				Sprite_render(sprite);
				Sprite_render(otherSprite);

				this->node = this->otherNode;
			}
			
			if(progressively){
				
				break;
			}
		}	

		if(progressively && this->node == this->otherNode){

			break;
		}
	}
	
	//this->needSorting = this->node? true: false;
	
	// TODO: remove
	//this->needSorting = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_addSprite(SpriteManager this, Sprite sprite){
	
	ASSERT(this, "SpriteManager::addSprite: null this");

	VPUManager_disableInterrupt(VPUManager_getInstance());

	u8 layer = __TOTAL_LAYERS - VirtualList_getSize(this->sprites);
	Sprite_setWorldLayer(sprite, layer);
	VirtualList_pushFront(this->sprites, sprite);
	SpriteManager_setLastLayer(this);
	this->needSorting = true;
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

		VPUManager_disableInterrupt(VPUManager_getInstance());
		node = VirtualNode_getPrevious(node);
		VirtualList_removeElement(this->sprites, sprite);
		
		for(node = node? node: VirtualList_begin(this->sprites); node; node = VirtualNode_getPrevious(node)){
			
			Sprite previousSprite = (Sprite)VirtualNode_getData(node);
			u8 layer = Sprite_getWorldLayer(previousSprite);
			Sprite_setWorldLayer(previousSprite, layer + 1);
			Sprite_render(previousSprite);
		}
	}
	
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

	this->freeLayer = __TOTAL_LAYERS - VirtualList_getSize(this->sprites);
	int printingLayer = 0 > this->freeLayer? 1: this->freeLayer;

	Printing_render(printingLayer);

	WORLD_HEAD((printingLayer - 1), WRLD_OFF);

    WORLD_SIZE((printingLayer - 1), 0, 0);
	
	WORLD_HEAD((printingLayer - 1), WRLD_END);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render sprites
void SpriteManager_render(SpriteManager this){

	ASSERT(this, "SpriteManager::render: null this");

	VPUManager_disableInterrupt(VPUManager_getInstance());

	VirtualNode node = VirtualList_end(this->sprites);
	for(; node; node = VirtualNode_getPrevious(node)){

		//render sprite	
		Sprite_render((Sprite)VirtualNode_getData(node));
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