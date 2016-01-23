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
#include <debugUtilities.h>


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

__CLASS_DEFINITION(SpriteManager, Object);

__CLASS_FRIEND_DEFINITION(Sprite);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


static void SpriteManager_constructor(SpriteManager this);


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
		
		VirtualNode node = this->sprites->head;
		
		if(node)
		{
			VirtualNode nextNode = node->next;
			
			for(; nextNode; node = node->next, nextNode = nextNode->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, node->data);
				Sprite nextSprite = __SAFE_CAST(Sprite, nextNode->data);
				VBVec2D position = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
				VBVec2D nextPosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, nextSprite);

				// check if z positions are swapped
				if(FIX19_13TOI(nextPosition.z) + nextSprite->displacement.z < FIX19_13TOI(position.z) + sprite->displacement.z)
				{
					// get each entity's layer
					u8 worldLayer1 = sprite->worldLayer;
					u8 worldLayer2 = nextSprite->worldLayer;
		
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

	this->node = this->node ? this->nextNode ? this->node : this->node->next: this->sprites->head;

	for(; this->node; this->node = this->node->next)
	{
		this->nextNode = this->node->next;

		if(this->nextNode)
		{
			Sprite sprite = __SAFE_CAST(Sprite, this->node->data);
			Sprite nextSprite = __SAFE_CAST(Sprite, this->nextNode->data);
			VBVec2D position = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
			VBVec2D nextPosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, nextSprite);
	
			// check if z positions are swapped
			if(nextPosition.z + nextSprite->displacement.z < position.z + sprite->displacement.z)
			{
				// get each entity's layer
				u8 worldLayer1 = sprite->worldLayer;
				u8 worldLayer2 = nextSprite->worldLayer;
	
				// don't render inmediately, it causes glitches
				Sprite_setWorldLayer(nextSprite, worldLayer1);
				Sprite_setWorldLayer(sprite, worldLayer2);

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
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::addSprite: adding no sprite");

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);
	
	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::addSprite: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
#endif
		// retrieve the next free layer, taking into account
		// if there are layers being freed up by the recovery algorithm
		u8 layer = __TOTAL_LAYERS - 1;
		
		if(this->sprites->head)
		{
			layer = (__SAFE_CAST(Sprite, VirtualList_front(this->sprites)))->worldLayer - 1;
		}
		
		// add to the front: last element corresponds to the 31 WORLD
		VirtualList_pushFront(this->sprites, sprite);

		Sprite_setWorldLayer(sprite, layer);

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
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::removeSprite: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::removeSprite: sprite not found");

	// check if exists
	if(VirtualList_removeElement(this->sprites, sprite))
	{
		u8 spriteLayer = sprite->worldLayer;

		VirtualNode node = this->sprites->head;

		for(; node; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);
			
			// search for the next sprite with the closest layer to the freed layer
			if(spriteLayer < sprite->worldLayer)
			{
				node = node->previous;
				break;
			}
		}

		for(; node; node = node->previous)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);
			
			ASSERT(spriteLayer == sprite->worldLayer + 1, "SpriteManager::removeSprite: wrong layers");

			spriteLayer--;
			
			// move the sprite to the freed layer
			Sprite_setWorldLayer(sprite, sprite->worldLayer + 1);
		}
		
		// sorting needs to restart
		this->node = NULL;
		this->nextNode = NULL;
	}
	else 
	{
		ASSERT(false, "SpriteManager::removeSprite: sprite not registered");
	}
}

// set free layers off
void SpriteManager_setLastLayer(SpriteManager this)
{
	ASSERT(this, "SpriteManager::setLastLayer: null this");

	if(this->sprites->head)
	{
		this->freeLayer = (__SAFE_CAST(Sprite, VirtualList_front(this->sprites)))->worldLayer - 1;
	}
	else 
	{
		this->freeLayer = __TOTAL_LAYERS - 1;
	}
	
	NM_ASSERT(0 <= this->freeLayer, "SpriteManager::setLastLayer: no more layers");
	NM_ASSERT(__TOTAL_LAYERS > VirtualList_getSize(this->sprites), "SpriteManager::setLastLayer: no more free layers");
	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing_render(Printing_getInstance(), this->freeLayer);
	
	if(0 < this->freeLayer)
	{
		WA[this->freeLayer - 1].head = WRLD_END;
	}
}

// render sprites
void SpriteManager_render(SpriteManager this)
{
	ASSERT(this, "SpriteManager::render: null this");

	// z sorting
	SpriteManager_sortLayersProgressively(this);
	
	// render from WORLD 31 to the lowest active one
	VirtualNode node = this->sprites->tail;
	
	for(; node; node = node->previous)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);
		__VIRTUAL_CALL(void, Sprite, render, sprite);
		
		// must make sure that no sprite has the end world
		// which can be the case when a new sprite is added
		// and the previous end world is assigned to it
		WA[sprite->worldLayer].head &= ~WRLD_END;
	}

	// configure printing layer and shutdown unused layers
	SpriteManager_setLastLayer(this);
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

	VirtualNode node = this->sprites->tail;

	for(; node; node = node->previous)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);
		
		if(sprite->worldLayer != layer)
		{
			__VIRTUAL_CALL(void, Sprite, hide, sprite);
		}
		else
		{
			__VIRTUAL_CALL(void, Sprite, show, sprite);
		}
		
		// force inialization
		VBVec2D spritePosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
		__VIRTUAL_CALL(void, Sprite, setPosition, sprite, &spritePosition);

		WA[sprite->worldLayer].head &= ~WRLD_END;
	}
}

// show all layers
void SpriteManager_recoverLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	VirtualNode node = this->sprites->tail;
	for(; node; node = node->previous)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		__VIRTUAL_CALL(void, Sprite, show, sprite);
	
		// force inialization
		VBVec2D spritePosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
		__VIRTUAL_CALL(void, Sprite, setPosition, sprite, &spritePosition);

		WA[sprite->worldLayer].head &= ~WRLD_END;
	}
	
	SpriteManager_setLastLayer(this);
}

Sprite SpriteManager_getSpriteAtLayer(SpriteManager this, u8 layer)
{
	ASSERT(this, "SpriteManager::getSpriteAtLayer: null this");
	ASSERT((unsigned)layer < __TOTAL_LAYERS, "SpriteManager::getSpriteAtLayer: invalid layer");
	
	VirtualNode node = this->sprites->head;
	
	for(; node; node = node->next)
	{
		if((__SAFE_CAST(Sprite, node->data))->worldLayer == layer)
		{
			return __SAFE_CAST(Sprite, node->data);
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

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

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
