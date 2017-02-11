/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <SpriteManager.h>
#include <Game.h>
#include <ObjectSpriteContainerManager.h>
#include <VIPManager.h>
#include <Screen.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_SPRITE_CLASS_NAME_SIZE			10


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * Sprites List
 *
 * @memberof SpriteManager
 */
typedef struct SpritesList
{
	const void* spriteClassVTable;
	VirtualList sprites;

} SpritesList;

#define SpriteManager_ATTRIBUTES																		\
	/* super's attributes */																			\
	Object_ATTRIBUTES																					\
	/* list of sprites to render */																		\
	VirtualList sprites;																				\
	/* sorting nodes */																					\
	VirtualNode node;																					\
	VirtualNode nextNode;																				\
	/* texture writing */																				\
	Texture textureToWrite;																				\
	/* next world layer	*/																				\
	s8 freeLayer;																						\
	/* flag to stop sorting while recovering layers	*/													\
	s8 recoveringLayers;																				\
	/* number of cycles that the texture writing is idle */												\
	s8 cyclesToWaitForTextureWriting;																	\
	/* number of rows to write in texture's writing	*/													\
	s8 texturesMaximumRowsToWrite;																		\
	/* flag to control texture's writing deferring */													\
	bool deferTextureWriting;																				\
	/* number of rows to write in affine transformations */												\
	s8 maximumAffineRowsToComputePerCall;																\
	/* flag to control texture's writing deferring */													\
	s8 deferAffineTransformations;																		\
	/* delay before writing again */																	\
	s8 waitToWrite;																						\

/**
 * @class 	SpriteManager
 * @extends Object
 */
__CLASS_DEFINITION(SpriteManager, Object);
__CLASS_FRIEND_DEFINITION(Sprite);
__CLASS_FRIEND_DEFINITION(Texture);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SpriteManager_constructor(SpriteManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(SpriteManager);

// class's constructor
static void __attribute__ ((noinline)) SpriteManager_constructor(SpriteManager this)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	this->node = NULL;
	this->nextNode = NULL;

	this->sprites = NULL;

	this->recoveringLayers = false;
	this->textureToWrite = NULL;
	this->cyclesToWaitForTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->deferTextureWriting = false;
	this->maximumAffineRowsToComputePerCall = -1;
	this->deferAffineTransformations = false;
	this->waitToWrite = 0;

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
	this->textureToWrite = NULL;
	this->cyclesToWaitForTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->deferTextureWriting = false;
	this->waitToWrite = 0;

	SpriteManager_setLastLayer(this);
}

// sort all layers
void SpriteManager_sortLayers(SpriteManager this)
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
				VBVec2D position = __VIRTUAL_CALL(Sprite, getPosition, sprite);
				VBVec2D nextPosition = __VIRTUAL_CALL(Sprite, getPosition, nextSprite);

				// check if z positions are swapped
				if(nextPosition.z + nextSprite->displacement.z < position.z + sprite->displacement.z)
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
	while(swap);
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
			VBVec2D position = __VIRTUAL_CALL(Sprite, getPosition, sprite);
			VBVec2D nextPosition = __VIRTUAL_CALL(Sprite, getPosition, nextSprite);

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

u8 SpriteManager_getWorldLayer(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::getWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::getWorldLayer: adding no sprite");

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);

	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::getWorldLayer: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
#endif
		// retrieve the next free layer, taking into account
		// if there are layers being freed up by the recovery algorithm
		s8 layer = __TOTAL_LAYERS - 1;

		VirtualNode head = this->sprites->head;

		if(head)
		{
			layer = (__SAFE_CAST(Sprite, head->data))->worldLayer - 1;
		}

		NM_ASSERT(0 < layer, "SpriteManager::getWorldLayer: no more layers");

		// add to the front: last element corresponds to the 31 WORLD
		VirtualList_pushFront(this->sprites, sprite);

		this->node = NULL;
		this->nextNode = NULL;

		return (u8)layer;

#ifdef __DEBUG
	}
#endif

	return 0;
}

// remove sprite
void SpriteManager_relinquishWorldLayer(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::relinquishWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::relinquishWorldLayer: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::relinquishWorldLayer: sprite not found");

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

		// block sorting
		this->recoveringLayers = true;

		for(; node; node = node->previous)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			ASSERT(spriteLayer-- == sprite->worldLayer + 1, "SpriteManager::relinquishWorldLayer: wrong layers");

			// move the sprite to the freed layer
			Sprite_setWorldLayer(sprite, sprite->worldLayer + 1);
		}

		// allow sorting
		this->recoveringLayers = false;

		// sorting needs to restart
		this->node = NULL;
		this->nextNode = NULL;
	}
	else
	{
		ASSERT(false, "SpriteManager::relinquishWorldLayer: sprite not registered");
	}
}

// set free layers off
void SpriteManager_setLastLayer(SpriteManager this)
{
	ASSERT(this, "SpriteManager::setLastLayer: null this");

	ASSERT(0 <= this->freeLayer, "SpriteManager::setLastLayer: no more layers");
	ASSERT(__TOTAL_LAYERS > VirtualList_getSize(this->sprites), "SpriteManager::setLastLayer: no more free layers");

	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing_render(Printing_getInstance(), this->freeLayer);

	if(0 < this->freeLayer)
	{
		_worldAttributesBaseAddress[this->freeLayer - 1].head = __WORLD_END;
	}
}

// render sprites
void SpriteManager_render(SpriteManager this)
{
	ASSERT(this, "SpriteManager::render: null this");

	bool textureWasWritten = false;

	if(!this->waitToWrite)
	{
		if(0 < this->texturesMaximumRowsToWrite && this->textureToWrite)
		{
			if(*(u32*)this->textureToWrite)
			{
				__VIRTUAL_CALL(Texture, write, this->textureToWrite);

				this->textureToWrite = !this->textureToWrite->written && this->textureToWrite->textureDefinition? this->textureToWrite : NULL;
				textureWasWritten = true;
				this->waitToWrite = this->cyclesToWaitForTextureWriting;
			}
			else
			{
				this->textureToWrite = NULL;
			}
		}
		else
		{
			VirtualNode node = this->sprites->head;

			for(; node; node = node->next)
			{
				Texture texture = (__SAFE_CAST(Sprite, node->data))->texture;

				if(texture && *(u32*)texture && !texture->written && texture->textureDefinition)
				{
					__VIRTUAL_CALL(Texture, write, texture);

					textureWasWritten = true;

					if(this->deferTextureWriting)
					{
						this->waitToWrite = this->cyclesToWaitForTextureWriting;
						this->textureToWrite = !texture->written? texture : NULL;
						break;
					}
				}
			}
		}
	}
	else
	{
		this->waitToWrite--;
	}

	if(!textureWasWritten && !this->recoveringLayers)
	{
		// z sorting
		SpriteManager_sortLayersProgressively(this);
	}

	VirtualNode node = this->sprites->head;

	this->freeLayer = __TOTAL_LAYERS - 1;

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		// first update
		Sprite_update(__SAFE_CAST(Sprite, sprite));

		if(sprite->hidden || !sprite->visible)
		{
			_worldAttributesBaseAddress[sprite->worldLayer].head = __WORLD_OFF;
		}
		else
		{
			__VIRTUAL_CALL(Sprite, render, sprite);
		}

		// must make sure that no sprite has the end world
		// which can be the case when a new sprite is added
		// and the previous end world is assigned to it
		_worldAttributesBaseAddress[sprite->worldLayer].head &= ~__WORLD_END;

		if(sprite->worldLayer && sprite->worldLayer < this->freeLayer)
		{
			this->freeLayer = sprite->worldLayer;
		}
	}

	this->freeLayer--;

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
			__VIRTUAL_CALL(Sprite, hide, sprite);
		}
		else
		{
			__VIRTUAL_CALL(Sprite, show, sprite);
		}

		// force initialization
		VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, sprite);
		__VIRTUAL_CALL(Sprite, setPosition, sprite, &spritePosition);

		_worldAttributesBaseAddress[sprite->worldLayer].head &= ~__WORLD_END;
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

		__VIRTUAL_CALL(Sprite, show, sprite);

		// force inialization
		VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, sprite);
		__VIRTUAL_CALL(Sprite, setPosition, sprite, &spritePosition);

		_worldAttributesBaseAddress[sprite->worldLayer].head &= ~__WORLD_END;
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

void SpriteManager_deferTextureWriting(SpriteManager this, bool deferTextureWriting)
{
	ASSERT(this, "SpriteManager::print: null this");

	this->waitToWrite = 0;
	this->deferTextureWriting = deferTextureWriting;
}

s8 SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager this)
{
	ASSERT(this, "SpriteManager::getTextureMaximumRowsToWrite: null this");

	return this->deferTextureWriting? this->texturesMaximumRowsToWrite : -1;
}

void SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager this, u8 cyclesToWaitForTextureWriting)
{
	ASSERT(this, "SpriteManager::getTextureMaximumRowsToWrite: null this");

	this->cyclesToWaitForTextureWriting = cyclesToWaitForTextureWriting;
}

void SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager this, u8 texturesMaximumRowsToWrite)
{
	ASSERT(this, "SpriteManager::setMaximumTextureRowsToWrite: null this");

	this->texturesMaximumRowsToWrite = 2 > (s8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

void SpriteManager_deferAffineTransformations(SpriteManager this, bool deferAffineTransformations)
{
	ASSERT(this, "SpriteManager::deferAffineTransformations: null this");

	this->deferAffineTransformations = deferAffineTransformations;
}

int SpriteManager_getMaximumAffineRowsToComputePerCall(SpriteManager this)
{
	ASSERT(this, "SpriteManager::getMaximumAffineRowsPerCall: null this");

	return this->deferAffineTransformations ? this->maximumAffineRowsToComputePerCall : -1;
}

void SpriteManager_setMaximumAffineRowsToComputePerCall(SpriteManager this, int maximumAffineRowsToComputePerCall)
{
	ASSERT(this, "SpriteManager::setMaximumAffineRowsToComputePerCall: null this");

	this->maximumAffineRowsToComputePerCall = maximumAffineRowsToComputePerCall;
}


// print status
void SpriteManager_print(SpriteManager this, int x, int y)
{
	ASSERT(this, "SpriteManager::print: null this");

	Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Last free layer:     ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->freeLayer, x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Free layers:         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __TOTAL_LAYERS - 1 - VirtualList_getSize(this->sprites), x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Sprites' count:      ", x, ++y, NULL);

	int auxY = y + 2;
	int auxX = x;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		strncpy(spriteClassName, __GET_CLASS_NAME_UNSAFE(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printing_int(Printing_getInstance(), Sprite_getWorldLayer(sprite), auxX, auxY, NULL);
		Printing_text(Printing_getInstance(), ": ", auxX + 2, auxY, NULL);
		Printing_text(Printing_getInstance(), spriteClassName, auxX + 4, auxY, NULL);
		Printing_hex(Printing_getInstance(), _worldAttributesBaseAddress[sprite->worldLayer].head, auxX + 14, auxY, 4, NULL);

		if((__SCREEN_HEIGHT >> 3) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}

	Printing_int(Printing_getInstance(), VirtualList_getSize(this->sprites), x + 17, y, NULL);
}
