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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SpriteManager.h>
#include <VPUManager.h>
#include <Screen.h>


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
	VirtualNode otherNode;														\
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

static void SpriteManager_constructor(SpriteManager this);
static void SpriteManager_processFreedLayers(SpriteManager this);

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
	this->otherNode = NULL;

	this->sprites = NULL;
	this->freedLayer = 0;
	this->tempFreedLayer = 0;

	SpriteManager_reset(this);
}

// class's destructor
void SpriteManager_destructor(SpriteManager this)
{
	ASSERT(this, "SpriteManager::destructor: null this");

	if (this->sprites)
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

	if (this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	this->sprites = __NEW(VirtualList);

	this->freeLayer = __TOTAL_LAYERS - 1;
	this->freedLayer = 0;

	this->node = NULL;
	this->otherNode = NULL;

	SpriteManager_setLastLayer(this);
}

// check if any entity must be assigned another world layer
void SpriteManager_spriteChangedPosition(SpriteManager this)
{
	ASSERT(this, "SpriteManager::spriteChangedPosition: null this");
}

// check if any entity must be assigned another world layer
void SpriteManager_sortLayers(SpriteManager this, int progressively)
{
	ASSERT(this, "SpriteManager::sortLayers: null this");

	this->node = progressively && this->node ? this->otherNode ? this->node : VirtualNode_getNext(this->node): VirtualList_begin(this->sprites);

	for (; this->node; this->node = VirtualNode_getNext(this->node))
	{
		Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(this->node));
		DrawSpec drawSpec = Sprite_getDrawSpec(sprite);

		this->otherNode = progressively && this->otherNode ? VirtualNode_getNext(this->otherNode) : VirtualNode_getNext(this->node);

		for (; this->otherNode; this->otherNode = VirtualNode_getNext(this->otherNode))
		{
			Sprite otherSprite = __UPCAST(Sprite, VirtualNode_getData(this->otherNode));
			DrawSpec otherDrawSpec = Sprite_getDrawSpec(otherSprite);

			// check if z positions are swaped
			if (otherDrawSpec.position.z < drawSpec.position.z)
			{
				// get each entity's layer
				u8 worldLayer1 = Sprite_getWorldLayer(sprite);
				u8 worldLayer2 = Sprite_getWorldLayer(otherSprite);

				ASSERT(worldLayer1 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 1");
				ASSERT(worldLayer2 != this->freedLayer, "SpriteManager::sortLayers: wrong layer 2");

				// swap layers
				Sprite_setWorldLayer(sprite, worldLayer2);
				Sprite_setWorldLayer(otherSprite, worldLayer1);

				// swap array entries
				VirtualNode_swapData(this->node, this->otherNode);

				this->node = this->otherNode;

				if (!progressively)
				{
					// make sure sort is complete
					this->node = VirtualList_begin(this->sprites);
					break;
				}
			}
			
			if (progressively)
			{
				return;
			}
		}

		if (progressively && !this->otherNode)
		{
			break;
		}
	}
}

void SpriteManager_addSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::addSprite: null this");
	ASSERT(__UPCAST(Sprite, sprite), "SpriteManager::addSprite: adding no sprite");

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);
	
	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::addSprite: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
#endif

		// retrieve the next free layer, taking into account
		// if there are layers being freed up by the recovery algorithm
		u8 layer = __TOTAL_LAYERS - 1;
		
		if(VirtualList_getSize(this->sprites))
		{
			layer = Sprite_getWorldLayer(__UPCAST(Sprite, VirtualList_front(this->sprites))) - 1;
			
			if(layer == this->tempFreedLayer)
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
		this->otherNode = NULL;
		SpriteManager_sortLayers(this, true);

		if(layer != Sprite_getWorldLayer(sprite))
		{
			layer = Sprite_getWorldLayer(sprite);
	
			Sprite_render(__UPCAST(Sprite, VirtualList_front(this->sprites)));
		}

		if(this->tempFreedLayer > layer)
		{
			ASSERT(this->freeLayer < layer, "SpriteManager::addSprite: 1 this->freeLayer >= layer");

			WORLD_HEAD(layer, 0x0000);
		}

#ifdef __DEBUG		
	}
#endif
}


// remove sprite
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::removeSprite: null this");
	ASSERT(__UPCAST(Sprite, sprite), "SpriteManager::removeSprite: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::removeSprite: sprite not found");

	// check if exists
	if (VirtualList_removeElement(this->sprites, sprite))
	{
		// hide it
		Sprite_hide(sprite);

		// calculate the freed layer
		// if there is already a higher layer being freed
		// don't do anything, the recovery algorithm will take
		// care of this new freed layer
		u8 spriteLayer = Sprite_getWorldLayer(sprite);
		this->freedLayer = this->freedLayer < spriteLayer? spriteLayer: this->freedLayer;
		
		// sorting needs to restart
		this->node = NULL;
		this->otherNode = NULL;
	}
	else 
	{
		ASSERT(false, "SpriteManager::removeSprite: sprite not registered");
	}
}

// process removed sprites
static void SpriteManager_processFreedLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::processRemovedSprites: null this");

	// must wait a cycle to settup the printing layer so
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
	
	if (this->freedLayer)
	{
		ASSERT(this->freedLayer < __TOTAL_LAYERS, "SpriteManager::processRemovedSprites: error freedLayer");

		VirtualNode node = VirtualList_end(this->sprites);

		for (; node; node = VirtualNode_getPrevious(node))
		{
			Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(node));
			u8 spriteLayer = Sprite_getWorldLayer(sprite);
			
			// search for the next sprite with the closest 
			// layer to the freed layer
			if (spriteLayer < this->freedLayer)
			{
				ASSERT(this->freeLayer < this->freedLayer, "Sprite::processRemovedSprites:1 this->freeLayer >= this->freedLayer");

				// move the sprite to the freed layer
				Sprite_setWorldLayer(sprite, this->freedLayer);
				
				// prerredner it so is is shown in the next cycle
				//Sprite_preRender(sprite);
				//Sprite_show(sprite);

				// register previous sprite's layer
				// to avoid flicker and gosthing
				this->tempFreedLayer = spriteLayer;
								
				// decrease freed layer
				// so the next time it is checked against it
			    this->freedLayer--;
			    
				ASSERT(this->freedLayer > this->freeLayer, "Sprite::processRemovedSprites:2 this->freedLayer <= this->freeLayer");

			    // don't enter here again if the end has been reached
			    node = VirtualNode_getPrevious(node);
				break;
			}
		}
		
		if (!node)
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

	if (VirtualList_getSize(this->sprites))
	{
		this->freeLayer = Sprite_getWorldLayer(__UPCAST(Sprite, VirtualList_front(this->sprites))) - 1;
		ASSERT(!this->tempFreedLayer || this->freeLayer <= this->tempFreedLayer, "SpriteManager::setLastLayer: this->freeLayer >= this->tempFreedLayer");
	}
	else 
	{
		this->freeLayer = __TOTAL_LAYERS - 1;
	}
	
	NM_ASSERT(0 <= this->freeLayer, "SpriteManager::setLastLayer: no more layers");
//	NM_ASSERT(this->freeLayer < __TOTAL_LAYERS - VirtualList_getSize(this->sprites), "SpriteManager::setLastLayer: no more free layers");
	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing_render(Printing_getInstance(), this->freeLayer);
	
	if (0 < this->freeLayer)
	{
		WA[this->freeLayer - 1].head = WRLD_OFF;
	}
}

// render sprites
void SpriteManager_render(SpriteManager this)
{
	ASSERT(this, "SpriteManager::render: null this");

	// sort layers
	SpriteManager_sortLayers(SpriteManager_getInstance(), true);

	// render from WORLD 31 to the lowes active one
	// reverse this order when a new sprite was added
	// to make effective its visual properties as quick as
	// possible
	VirtualNode node = VirtualList_begin(this->sprites);
/*
	for (; node; node = VirtualNode_getNext(node))
	{
		if(Sprite_getRenderFlag(__UPCAST(Sprite, VirtualNode_getData(node))))
		{
			__VIP_WAIT;
			break;
		}
	}
*/
	for (; node; node = VirtualNode_getNext(node))
	{
		Sprite_render(__UPCAST(Sprite, VirtualNode_getData(node)));
	}

	// recover layers
	SpriteManager_processFreedLayers(this);
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

	for (; node; node = VirtualNode_getPrevious(node))
	{
		if (Sprite_getWorldLayer(__UPCAST(Sprite, VirtualNode_getData(node))) != layer)
		{
			Sprite_hide(__UPCAST(Sprite, VirtualNode_getData(node)));
		}
		else
		{
			Sprite_show(__UPCAST(Sprite, VirtualNode_getData(node)));
		}
	}
}

// show all layers
void SpriteManager_recoverLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	VirtualNode node = VirtualList_end(this->sprites);
	for (; node; node = VirtualNode_getPrevious(node))
	{
		Sprite_show(__UPCAST(Sprite, VirtualNode_getData(node)));
	}
}

// print status
void SpriteManager_print(SpriteManager this, int x, int y)
{
	ASSERT(this, "SpriteManager::print: null this");

	Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Last free layer: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->freeLayer, x + 15, y, NULL);
	Printing_text(Printing_getInstance(), "Free layers: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __TOTAL_LAYERS - 1 - VirtualList_getSize(this->sprites), x + 15, y, NULL);
	Printing_text(Printing_getInstance(), "Sprites count: ", x, ++y, NULL);

	int auxY = y + 2;
	int auxX = x;

	VirtualNode node = VirtualList_begin(this->sprites);

	for (; node; node = VirtualNode_getNext(node))
	{
		Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(node));

		Printing_text(Printing_getInstance(), "Sprite: ", auxX, auxY, NULL);
		Printing_int(Printing_getInstance(), Sprite_getWorldLayer(sprite), auxX + 8, auxY, NULL);
		Printing_text(Printing_getInstance(), __GET_CLASS_NAME(sprite), auxX + 11, auxY, NULL);

		if (28 <= ++auxY)
		{
			auxY = y + 2;
			auxX += 25;
		}
	}

	Printing_int(Printing_getInstance(), VirtualList_getSize(this->sprites), x + 15, y, NULL);
}
