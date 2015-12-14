/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------


#include <string.h>
#include <SpriteManager.h>
#include <Game.h>
#include <ObjectSpriteContainerManager.h>
#include <VPUManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_SPRITE_CLASS_NAME_SIZE	19


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define SpriteManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* list of sprites to render */												\
	VirtualList sprites;														\
																				\
	/* sorting nodes	*/														\
	VirtualNode node;															\
	VirtualNode nextNode;														\
																				\
	/* next world layer	*/														\
	s8 freeLayer;																\
																				\
	/* list of sprites to render */												\
	s8 freedLayer;																\
	s8 tempFreedLayer;															\

__CLASS_DEFINITION(SpriteManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern unsigned int volatile* _xpstts;

static void SpriteManager_constructor(SpriteManager this);
static void SpriteManager_processFreedLayersProgressively(SpriteManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(SpriteManager);

// class's constructor
static void SpriteManager_constructor(SpriteManager this)
{
	// construct base object
	__CONSTRUCT_BASE();

	this->node = NULL;
	this->nextNode = NULL;

	this->sprites = NULL;
	this->freedLayer = 0;
	this->tempFreedLayer = 0;

	SpriteManager_reset(this);
}

// class's destructor
void SpriteManager_destructor(SpriteManager this)
{
	ASSERT(this, "SpriteManager::destructor: null this");

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void SpriteManager_reset(SpriteManager this)
{
	ASSERT(this, "SpriteManager::reset: null this");

	// must reset the ObjectSpriteContainerManager before the SpriteManager!
	ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager_getInstance());

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	this->sprites = __NEW(VirtualList);

	this->freeLayer = __TOTAL_LAYERS - 1;
	this->freedLayer = 0;
	this->tempFreedLayer = 0;

	this->node = NULL;
	this->nextNode = NULL;

	SpriteManager_setLastLayer(this);
}

// check if any entity must be assigned another world layer
void SpriteManager_spriteChangedPosition(SpriteManager this)
{
	ASSERT(this, "SpriteManager::spriteChangedPosition: null this");
}

// sort all layers
void SpriteManager_sortLayers(SpriteManager this, int progressively)
{
	ASSERT(this, "SpriteManager::sortLayers: null this");

	bool swap = false;

	do
	{
		swap = false;
		
		VirtualNode node = VirtualList_begin(this->sprites);
		
		if(node)
		{
			VirtualNode nextNode = VirtualNode_getNext(node);
			
			for(; nextNode; node = VirtualNode_getNext(node), nextNode = VirtualNode_getNext(nextNode))
			{
				Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(node));
				Sprite nextSprite = __GET_CAST(Sprite, VirtualNode_getData(nextNode));
				const VBVec2D* position = __VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, sprite);
				const VBVec2D* nextPosition = __VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, nextSprite);

				// check if z positions are swapped
				if(FIX19_13TOI(nextPosition->z) + Sprite_getDisplacement(nextSprite).z < FIX19_13TOI(position->z) + Sprite_getDisplacement(sprite).z)
				{
					// get each entity's layer
					u8 worldLayer1 = Sprite_getWorldLayer(sprite);
					u8 worldLayer2 = Sprite_getWorldLayer(nextSprite);
		
					ASSERT(worldLayer1 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 1");
					ASSERT(worldLayer2 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 2");
		
					// swap layers
					Sprite_setWorldLayer(sprite, worldLayer2);
					Sprite_setWorldLayer(nextSprite, worldLayer1);
		
					// swap array entries
					VirtualNode_swapData(node, nextNode);
					
					swap = true;
				}
			}
		}
	}
	while (swap);
}

// check if any entity must be assigned another world layer
void SpriteManager_sortLayersProgressively(SpriteManager this)
{
	ASSERT(this, "SpriteManager::sortLayersProgressively: null this");

	this->node = this->node ? this->nextNode ? this->node : VirtualNode_getNext(this->node): VirtualList_begin(this->sprites);

	for(; this->node; this->node = VirtualNode_getNext(this->node))
	{
		this->nextNode = VirtualNode_getNext(this->node);

		if(this->nextNode)
		{
			Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(this->node));
			Sprite nextSprite = __GET_CAST(Sprite, VirtualNode_getData(this->nextNode));
			const VBVec2D* position = __VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, sprite);
			const VBVec2D* nextPosition = __VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, nextSprite);
	
			// check if z positions are swapped
			if(FIX19_13TOI(nextPosition->z) + Sprite_getDisplacement(nextSprite).z < FIX19_13TOI(position->z) + Sprite_getDisplacement(sprite).z)
			{
				// get each entity's layer
				u8 worldLayer1 = Sprite_getWorldLayer(sprite);
				u8 worldLayer2 = Sprite_getWorldLayer(nextSprite);
	
				// swap layers
				ASSERT(worldLayer1 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 1");
				ASSERT(worldLayer2 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 2");
	
				// don't render inmediately, it causes glitches
				Sprite_setWorldLayer(sprite, worldLayer2);
				Sprite_setWorldLayer(nextSprite, worldLayer1);
	
				// swap nodes' data
				VirtualNode_swapData(this->node, this->nextNode);

				this->node = this->nextNode;
				return;
			}
		}
	}
}

void SpriteManager_addSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::addSprite: null this");
	ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::addSprite: adding no sprite");

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);
	
	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::addSprite: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
#endif
		// retrieve the next free layer, taking into account
		// if there are layers being freed up by the recovery algorithm
		u8 layer = __TOTAL_LAYERS - 1;
		
		if(VirtualList_begin(this->sprites))
		{
			layer = Sprite_getWorldLayer(__GET_CAST(Sprite, VirtualList_front(this->sprites))) - 1;
			
			if(this->tempFreedLayer && layer == this->tempFreedLayer)
			{
				layer--;
			}
		}
		
		// add to the front: last element corresponde to the 31 WORLD
		VirtualList_pushFront(this->sprites, sprite);

		Sprite_setWorldLayer(sprite, layer);

		// configure printing layer
		// and shutdown unused layers
		SpriteManager_setLastLayer(this);
		ASSERT(this->freeLayer < layer, "SpriteManager::addSprite: this->freeLayer >= layer");

		this->node = NULL;
		this->nextNode = NULL;

#ifdef __DEBUG		
	}
#endif
}


// remove sprite
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::removeSprite: null this");
	ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::removeSprite: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::removeSprite: sprite not found");

	// check if exists
	if(VirtualList_removeElement(this->sprites, sprite))
	{
		// hide it
		__VIRTUAL_CALL(void, Sprite, hide, sprite);

		// calculate the freed layer
		// if there is already a higher layer being freed
		// don't do anything, the recovery algorithm will take
		// care of this new freed layer
		u8 spriteLayer = Sprite_getWorldLayer(sprite);
		this->freedLayer = this->freedLayer < spriteLayer? spriteLayer: this->freedLayer;
		
		// sorting needs to restart
		this->node = NULL;
		this->nextNode = NULL;
	}
	else 
	{
		ASSERT(false, "SpriteManager::removeSprite: sprite not registered");
	}
}

// process sprites
void SpriteManager_processLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::processLayers: null this");

	SpriteManager_processFreedLayersProgressively(SpriteManager_getInstance());

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
	SpriteManager_sortLayersProgressively(SpriteManager_getInstance());
}

void SpriteManager_processFreedLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::processRemovedSprites: null this");

	while(this->freedLayer)
	{
		SpriteManager_processFreedLayersProgressively(this);
	}
}

// process removed sprites
static void SpriteManager_processFreedLayersProgressively(SpriteManager this)
{
	ASSERT(this, "SpriteManager::processFreedLayersProgressively: null this");

	// must wait a cycle to setup the printing layer so
	// we allow for the last sprite to be redraw in the previous layer
	// before reclaiming it back
	static bool previouslyRecoveredAllLayers = false;
	
	if(this->tempFreedLayer)
	{
		u8 tempFreedLayer = this->tempFreedLayer;
		this->tempFreedLayer = 0;

		if(previouslyRecoveredAllLayers)
		{
			previouslyRecoveredAllLayers = false;
			
			SpriteManager_setLastLayer(this);
		}

		// if not the free layer yet
		// turn off the layer
		if(this->freeLayer < tempFreedLayer)
		{
			WORLD_HEAD(tempFreedLayer, 0x0000);
		}		
	}
	
	if(this->freedLayer)
	{
		ASSERT(this->freedLayer < __TOTAL_LAYERS, "SpriteManager::processFreedLayersProgressively: error freedLayer");

		VirtualNode node = VirtualList_end(this->sprites);

		for(; node; node = VirtualNode_getPrevious(node))
		{
			Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(node));
			u8 spriteLayer = Sprite_getWorldLayer(sprite);
			
			// search for the next sprite with the closest 
			// layer to the freed layer
			if(spriteLayer < this->freedLayer)
			{
				ASSERT(this->freeLayer < this->freedLayer, "Sprite::processFreedLayersProgressively:1 this->freeLayer >= this->freedLayer");

				// render last position before using new layer
				__VIRTUAL_CALL(void, Sprite, render, sprite);

				// move the sprite to the freed layer
				Sprite_setWorldLayer(sprite, this->freedLayer);

				// render last position before using new layer
				__VIRTUAL_CALL(void, Sprite, render, sprite);

				// register previous sprite's layer
				// to avoid flicker and gosthing
				this->tempFreedLayer = spriteLayer;
								
				// decrease freed layer
				// so the next time it is checked against it
			    this->freedLayer--;
			    
				ASSERT(this->freedLayer > this->freeLayer, "Sprite::processFreedLayersProgressively:2 this->freedLayer <= this->freeLayer");

			    // don't enter here again if the end has been reached
			    node = VirtualNode_getPrevious(node);
				break;
			}
		}
		
		if(!node)
		{
			this->freedLayer = 0;
			previouslyRecoveredAllLayers = true;
		}
	}
}

// set free layers off
void SpriteManager_setLastLayer(SpriteManager this)
{
	ASSERT(this, "SpriteManager::setLastLayer: null this");

	if(VirtualList_begin(this->sprites))
	{
		this->freeLayer = Sprite_getWorldLayer(__GET_CAST(Sprite, VirtualList_front(this->sprites))) - 1;
		ASSERT(!this->tempFreedLayer || this->freeLayer <= this->tempFreedLayer, "SpriteManager::setLastLayer: this->freeLayer >= this->tempFreedLayer");
	}
	else 
	{
		this->freeLayer = __TOTAL_LAYERS - 1;
	}
	
	ASSERT(0 <= this->freeLayer, "SpriteManager::setLastLayer: no more layers");
//	NM_ASSERT(this->freeLayer < __TOTAL_LAYERS - VirtualList_getSize(this->sprites), "SpriteManager::setLastLayer: no more free layers");
	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	while (*_xpstts & XPBSYR);
	
	Printing_render(Printing_getInstance(), this->freeLayer);
	
	if(0 < this->freeLayer)
	{
		WA[this->freeLayer - 1].head = WRLD_OFF;
	}
}

// render sprites
void SpriteManager_render(SpriteManager this)
{
	ASSERT(this, "SpriteManager::render: null this");

	VPUManager_disableInterrupt(VPUManager_getInstance());

	SpriteManager_processLayers(SpriteManager_getInstance());

	// render from WORLD 31 to the lowest active one
	VirtualNode node = VirtualList_end(this->sprites);

	for(; node; node = VirtualNode_getPrevious(node))
	{
		__VIRTUAL_CALL(void, Sprite, render, __GET_CAST(Sprite, VirtualNode_getData(node)));
	}

	VPUManager_enableInterrupt(VPUManager_getInstance());
}

// retrieve free layer
int SpriteManager_getFreeLayer(SpriteManager this)
{
	ASSERT(this, "SpriteManager::getFreeLayer: null this");

	return this->freeLayer;
}

// show a given layer
void SpriteManager_showLayer(SpriteManager this, u8 layer)
{
	ASSERT(this, "SpriteManager::showLayer: null this");

	VirtualNode node = VirtualList_end(this->sprites);

	for(; node; node = VirtualNode_getPrevious(node))
	{
		if(Sprite_getWorldLayer(__GET_CAST(Sprite, VirtualNode_getData(node))) != layer)
		{
			__VIRTUAL_CALL(void, Sprite, hide, __GET_CAST(Sprite, VirtualNode_getData(node)));
		}
		else
		{
			__VIRTUAL_CALL(void, Sprite, show, __GET_CAST(Sprite, VirtualNode_getData(node)));
		}
	}
}

// show all layers
void SpriteManager_recoverLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	VirtualNode node = VirtualList_end(this->sprites);
	for(; node; node = VirtualNode_getPrevious(node))
	{
		__VIRTUAL_CALL(void, Sprite, show, __GET_CAST(Sprite, VirtualNode_getData(node)));
	}
}

Sprite SpriteManager_getSpriteAtLayer(SpriteManager this, u8 layer)
{
	ASSERT(this, "SpriteManager::getSpriteAtLayer: null this");
	ASSERT((unsigned)layer < __TOTAL_LAYERS, "SpriteManager::getSpriteAtLayer: invalid layer");
	
	VirtualNode node = VirtualList_begin(this->sprites);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		if(Sprite_getWorldLayer(__GET_CAST(Sprite, VirtualNode_getData(node))) == layer)
		{
			return __GET_CAST(Sprite, VirtualNode_getData(node));
		}
	}
	
	return NULL;
}

// print status
void SpriteManager_print(SpriteManager this, int x, int y)
{
	ASSERT(this, "SpriteManager::print: null this");

	Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Last free layer: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->freeLayer, x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Free layers: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __TOTAL_LAYERS - 1 - VirtualList_getSize(this->sprites), x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Sprites' count: ", x, ++y, NULL);

	int auxY = y + 2;
	int auxX = x;

	VirtualNode node = VirtualList_begin(this->sprites);

	for(; node; node = VirtualNode_getNext(node))
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(node));

		strncpy(spriteClassName, __GET_CLASS_NAME(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';
		
		Printing_int(Printing_getInstance(), Sprite_getWorldLayer(sprite), auxX, auxY, NULL);
		Printing_text(Printing_getInstance(), ": ", auxX + 2, auxY, NULL);
		Printing_text(Printing_getInstance(), spriteClassName, auxX + 4, auxY, NULL);

		if(__SCREEN_HEIGHT / 8 - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 5;
		}
	}

	Printing_int(Printing_getInstance(), VirtualList_getSize(this->sprites), x + 17, y, NULL);
}
