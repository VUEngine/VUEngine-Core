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
#include <ParamTableManager.h>
#include <CharSetManager.h>
#include <Screen.h>
#include <Printing.h>
#include <TimerManager.h>
#include <debugConfig.h>


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
	Object_ATTRIBUTES																					\
	/**
	 * @var VirtualList	sprites
	 * @brief 			list of sprites to render
	 * @memberof		SpriteManager
	 */																									\
	VirtualList sprites;																				\
	/**
	 * @var VirtualList	spritesToDispose
	 * @brief 			list of sprites to delete
	 * @memberof		SpriteManager
	 */																									\
	VirtualList spritesToDispose;																		\
	/**
	 * @var VirtualNode	node
	 * @brief 			sorting nodes
	 * @memberof		SpriteManager
	 */																									\
	VirtualNode zSortingFirstNode;																		\
	/**
	 * @var VirtualNode	nextNode
	 * @brief
	 * @memberof		SpriteManager
	 */																									\
	VirtualNode zSortingSecondNode;																		\
	/**
	 * @var Sprite		spritePendingTextureWriting
	 * @brief 			sprite's texture writing
	 * @memberof		SpriteManager
	 */																									\
	Sprite spritePendingTextureWriting;																	\
	/**
	 * @var u8			freeLayer
	 * @brief 			next world layer
	 * @memberof		SpriteManager
	 */																									\
	u8 freeLayer;																						\
	/**
	 * @var s8			cyclesToWaitForSpriteTextureWriting
	 * @brief 			number of cycles that the texture writing is idle
	 * @memberof		SpriteManager
	 */																									\
	s8 cyclesToWaitForSpriteTextureWriting;																	\
	/**
	 * @var s8			texturesMaximumRowsToWrite
	 * @brief 			number of rows to write in texture's writing
	 * @memberof		SpriteManager
	 */																									\
	s8 texturesMaximumRowsToWrite;																		\
	/**
	 * @var s8			maximumParamTableRowsToComputePerCall
	 * @brief 			number of rows to write in affine transformations
	 * @memberof		SpriteManager
	 */																									\
	s8 maximumParamTableRowsToComputePerCall;															\
	/**
	 * @var s8			deferParamTableEffects
	 * @brief 			flag to control texture's writing deferring
	 * @memberof		SpriteManager
	 */																									\
	s8 deferParamTableEffects;																			\
	/**
	 * @var s8			waitToWriteSpriteTextures
	 * @brief 			delay before writing again
	 * @memberof		SpriteManager
	 */																									\
	s8 waitToWriteSpriteTextures;																		\
	/**
	 * @var bool		lockSpritesLists
	 * @brief 			semaphore to prevent manipulation of VirtualList during interrupt
	 * @memberof		SpriteManager
	 */																									\
	bool lockSpritesLists;																				\


/**
 * @class 	SpriteManager
 * @extends Object
 * @ingroup graphics-2d-sprites
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
static void SpriteManager_selectSpritePendingTextureWriting(SpriteManager this);
static bool SpriteManager_disposeSpritesProgressively(SpriteManager this);

#ifdef __PROFILE_GAME
int _totalPixelsToDraw = 0;
#endif

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SpriteManager_getInstance()
 * @memberof	SpriteManager
 * @public
 *
 * @return		SpriteManager instance
 */
__SINGLETON(SpriteManager);

/**
 * Class constructor
 *
 * @memberof	SpriteManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) SpriteManager_constructor(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	this->zSortingFirstNode = NULL;
	this->zSortingSecondNode = NULL;

	this->sprites = NULL;
	this->spritesToDispose = NULL;

	this->spritePendingTextureWriting = NULL;
	this->cyclesToWaitForSpriteTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->waitToWriteSpriteTextures = 0;
	this->lockSpritesLists = false;

	SpriteManager_reset(this);
}

/**
 * Class destructor
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 */
void SpriteManager_destructor(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::destructor: null this");

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	if(this->spritesToDispose)
	{
		while(SpriteManager_disposeSpritesProgressively(this));

		__DELETE(this->spritesToDispose);
		this->spritesToDispose = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Reset manager's state
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 */
void SpriteManager_reset(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::reset: null this");

	// must reset the ObjectSpriteContainerManager before the SpriteManager!
	ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager_getInstance());

	if(this->spritesToDispose)
	{
		while(SpriteManager_disposeSpritesProgressively(this));

		__DELETE(this->spritesToDispose);
		this->spritesToDispose = NULL;
	}

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	this->sprites = __NEW(VirtualList);
	this->spritesToDispose = __NEW(VirtualList);

	this->freeLayer = __TOTAL_LAYERS - 1;

	this->zSortingFirstNode = NULL;
	this->zSortingSecondNode = NULL;
	this->spritePendingTextureWriting = NULL;
	this->cyclesToWaitForSpriteTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->waitToWriteSpriteTextures = 0;

	SpriteManager_renderLastLayer(this);
}

/**
 * Delete disposable sprites
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param sprite	Sprite to dispose
 */
void SpriteManager_disposeSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::disposeSprite: null this");
	ASSERT(__IS_OBJECT_ALIVE(sprite), "SpriteManager::disposeSprite: trying to dispose dead sprite");

	if(sprite && !VirtualList_find(this->spritesToDispose, sprite))
	{
		this->lockSpritesLists = true;
		VirtualList_pushBack(this->spritesToDispose, sprite);

		Sprite_hide(sprite);
		this->lockSpritesLists = false;
	}
}

/**
 * Delete disposable sprites progressively
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		True if there were a sprite to delete
 */
static bool SpriteManager_disposeSpritesProgressively(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::disposeSprites: null this");

	if(!this->lockSpritesLists && this->spritesToDispose->head)
	{
		Sprite sprite = __SAFE_CAST(Sprite, VirtualList_front(this->spritesToDispose));

		VirtualList_popFront(this->spritesToDispose);

		__DELETE(sprite);

		this->spritePendingTextureWriting = __IS_OBJECT_ALIVE(this->spritePendingTextureWriting)? this->spritePendingTextureWriting : NULL;

		return true;
	}

	return false;
}

/**
 * Delete disposable sprites
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		True if there were a sprite to delete
 */
void SpriteManager_disposeSprites(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::disposeSprites: null this");

	if(this->spritesToDispose)
	{
		this->lockSpritesLists = false;
		while(SpriteManager_disposeSpritesProgressively(this));
	}
}

/**
 * Sort sprites according to their z coordinate
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 */
void SpriteManager_sortLayers(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::sortLayers: null this");

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
/**
 * Deferred sorting sprites according to their z coordinate
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 */
void SpriteManager_sortLayersProgressively(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::sortLayersProgressively: null this");

	this->zSortingFirstNode = this->zSortingFirstNode ? this->zSortingSecondNode ? this->zSortingFirstNode : this->zSortingFirstNode->next: this->sprites->head;

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	for(; this->zSortingFirstNode; this->zSortingFirstNode = this->zSortingFirstNode->next)
	{
		this->zSortingSecondNode = this->zSortingFirstNode->next;

		if(this->zSortingSecondNode)
		{
			Sprite sprite = __SAFE_CAST(Sprite, this->zSortingFirstNode->data);
			Sprite nextSprite = __SAFE_CAST(Sprite, this->zSortingSecondNode->data);
			VBVec2D position = __VIRTUAL_CALL(Sprite, getPosition, sprite);
			VBVec2D nextPosition = __VIRTUAL_CALL(Sprite, getPosition, nextSprite);

			// check if z positions are swapped
			if(nextPosition.z + nextSprite->displacement.z < position.z + sprite->displacement.z)
			{
				Sprite sprite = __SAFE_CAST(Sprite, this->zSortingFirstNode->data);
				Sprite nextSprite = __SAFE_CAST(Sprite, this->zSortingSecondNode->data);

				// get each entity's layer
				u8 worldLayer1 = sprite->worldLayer;
				u8 worldLayer2 = nextSprite->worldLayer;

				// don't render inmediately, it causes glitches
				Sprite_setWorldLayer(nextSprite, worldLayer1);
				Sprite_setWorldLayer(sprite, worldLayer2);

				// swap nodes' data
				VirtualNode_swapData(this->zSortingFirstNode, this->zSortingSecondNode);

				this->zSortingFirstNode = this->zSortingSecondNode;
			}
		}
	}
}

/**
 * Register a Sprite and assign a WORLD layer to it
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param sprite	Sprite to assign the WORLD layer
 */
void SpriteManager_registerSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::getWorldLayer: adding no sprite");

	s8 layer = 0;

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);

	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::getWorldLayer: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
		this->lockSpritesLists = true;
#endif
		// retrieve the next free layer, taking into account
		// if there are layers being freed up by the recovery algorithm
		layer = __TOTAL_LAYERS - 1;

		VirtualNode head = this->sprites->head;

		if(head)
		{
			layer = (__SAFE_CAST(Sprite, head->data))->worldLayer - 1;
		}

		NM_ASSERT(0 < layer, "SpriteManager::getWorldLayer: no more layers");

		// add to the front: last element corresponds to the 31 WORLD
		VirtualList_pushFront(this->sprites, sprite);

		this->zSortingFirstNode = NULL;
		this->zSortingSecondNode = NULL;

#ifdef __DEBUG
	}
#endif

	Sprite_setWorldLayer(sprite, layer);

	this->lockSpritesLists = false;
}

/**
 * Remove a registered Sprite and get back the WORLD layer previously assigned to it
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param sprite	Sprite to assign the WORLD layer
 */
void SpriteManager_unregisterSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::relinquishWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::relinquishWorldLayer: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::relinquishWorldLayer: sprite not found");

	this->lockSpritesLists = true;

	// check if exists
	if(VirtualList_removeElement(this->sprites, sprite))
	{
		VirtualList_removeElement(this->sprites, this->spritesToDispose);

		u8 spriteLayer = sprite->worldLayer;

		VirtualNode node = this->sprites->head;

		for(; node;)
		{
			// search for the next sprite with the closest layer to the freed layer
			if(spriteLayer < (__SAFE_CAST(Sprite, node->data))->worldLayer)
			{
				node = node->previous;
				break;
			}

			node = node->next;

			if(!node)
			{
				node = this->sprites->tail;
				break;
			}
		}

		for(; node; node = node->previous)
		{
			ASSERT(spriteLayer-- == (__SAFE_CAST(Sprite, node->data))->worldLayer + 1, "SpriteManager::relinquishWorldLayer: wrong layers");

			// move the sprite to the freed layer
			(__SAFE_CAST(Sprite, node->data))->worldLayer += 1;
		}

		// sorting needs to restart
		this->zSortingFirstNode = NULL;
		this->zSortingSecondNode = NULL;
	}
	else
	{
		ASSERT(false, "SpriteManager::relinquishWorldLayer: sprite not registered");
	}

	this->lockSpritesLists = false;
}

/**
 * Render the WORLD destined to printing output
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
void SpriteManager_renderLastLayer(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::setLastLayer: null this");

	ASSERT(0 <= (s8)this->freeLayer, "SpriteManager::setLastLayer: no more layers");
	ASSERT(__TOTAL_LAYERS > VirtualList_getSize(this->sprites), "SpriteManager::setLastLayer: no more free layers");

	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing_render(Printing_getInstance(), this->freeLayer);

	if(0 < this->freeLayer)
	{
		_worldAttributesBaseAddress[this->freeLayer - 1].head = __WORLD_END;
	}
}

/**
 * Select the sprite to write
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
static void SpriteManager_selectSpritePendingTextureWriting(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::selectSpritePendingTextureWriting: null this");

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		if(__IS_OBJECT_ALIVE(sprite) && !__VIRTUAL_CALL(Sprite, areTexturesWritten, sprite))
		{
			bool areTexturesWritten = __VIRTUAL_CALL(Sprite, writeTextures, sprite);

			this->waitToWriteSpriteTextures = this->cyclesToWaitForSpriteTextureWriting;
			this->spritePendingTextureWriting = !areTexturesWritten ? sprite : NULL;
			break;
		}
	}
}

/**
 * Write textures to DRAM
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
void SpriteManager_writeTextures(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::writeTextures: null this");

	CharSetManager_writeCharSets(CharSetManager_getInstance());

	s8 texturesMaximumRowsToWrite = this->texturesMaximumRowsToWrite;

	// allow complete texture writing
	this->texturesMaximumRowsToWrite = -1;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Sprite, writeTextures, node->data);
	}

	this->texturesMaximumRowsToWrite = texturesMaximumRowsToWrite;
}

/**
 * Write selected texture to DRAM
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
static bool SpriteManager_writeSelectedSprite(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::writeSelectedSprite: null this");

	bool textureWritten = false;

	if(!this->waitToWriteSpriteTextures)
	{
		if(this->spritePendingTextureWriting)
		{
			if(__IS_OBJECT_ALIVE(this->spritePendingTextureWriting) && !__VIRTUAL_CALL(Sprite, areTexturesWritten, this->spritePendingTextureWriting))
			{
				this->spritePendingTextureWriting = __VIRTUAL_CALL(Sprite, writeTextures, this->spritePendingTextureWriting) ? this->spritePendingTextureWriting : NULL;
				this->waitToWriteSpriteTextures = this->cyclesToWaitForSpriteTextureWriting;
				textureWritten = true;
			}
			else
			{
				this->spritePendingTextureWriting = NULL;
			}
		}
		else
		{
			SpriteManager_selectSpritePendingTextureWriting(this);
		}
	}
	else
	{
		this->waitToWriteSpriteTextures--;
	}

	return textureWritten;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */

void SpriteManager_render(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::render: null this");

	// must dispose sprites before doing anything else in
	// order to try to make room in DRAM to new sprites
	// as soon as possible

	bool skipNonCriticalProcesses = SpriteManager_disposeSpritesProgressively(this);
	skipNonCriticalProcesses |= CharSetManager_writeCharSetsProgressively(CharSetManager_getInstance());

	// write textures
	if(!skipNonCriticalProcesses && !SpriteManager_writeSelectedSprite(this))
	{
		// defragment param table
		if(!ParamTableManager_defragmentProgressively(ParamTableManager_getInstance()))
		{
			// z sorting
        	SpriteManager_sortLayersProgressively(this);
		}
	}

	VirtualNode node = this->sprites->head;

	if(!node)
	{
		this->freeLayer = __TOTAL_LAYERS - 1;
	}
	else
	{
		this->freeLayer = (__SAFE_CAST(Sprite, node->data))->worldLayer - 1;
	}

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		// first update
		if((u32)sprite->animationController | sprite->transparent)
		{
			Sprite_update(__SAFE_CAST(Sprite, sprite));
		}

		if(sprite->hidden | !sprite->visible)
		{
			_worldAttributesBaseAddress[sprite->worldLayer].head = __WORLD_OFF;
		}
		else
		{
			__VIRTUAL_CALL(Sprite, render, sprite);
		}
	}

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game_isInSpecialMode(Game_getInstance()))
	{
		_totalPixelsToDraw = _worldAttributesBaseAddress[this->freeLayer].w * _worldAttributesBaseAddress[this->freeLayer].h;

		VirtualNode node = this->sprites->head;

		for(; node; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			if(__WORLD_OFF != _worldAttributesBaseAddress[sprite->worldLayer].head)
			{
				_totalPixelsToDraw += _worldAttributesBaseAddress[sprite->worldLayer].w * _worldAttributesBaseAddress[sprite->worldLayer].h;
			}
		}
	}
#endif

	// configure printing layer and shutdown unused layers
	SpriteManager_renderLastLayer(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game_isInSpecialMode(Game_getInstance()))
	{
		SpriteManager_print(this, 1, 15, true);
	}
#endif
}

/**
 * Retrieve the next free WORLD layer
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Free WORLD layer
 */
u8 SpriteManager_getFreeLayer(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getFreeLayer: null this");

	return this->freeLayer;
}

/**
 * Show the Sprite in the given WORLD layer and hide the rest
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param layer		WORLD layer to show
 */
void SpriteManager_showLayer(SpriteManager this, u8 layer)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::showLayer: null this");

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

/**
 * Show all WORLD layers
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
void SpriteManager_recoverLayers(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::recoverLayers: null this");

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

	SpriteManager_renderLastLayer(this);
}

/**
 * Retrieve the Sprite assigned to the given WORLD
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param layer		WORLD layer to show
 *
 * @return			Sprite with the given WORLD layer
 */
Sprite SpriteManager_getSpriteAtLayer(SpriteManager this, u8 layer)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getSpriteAtLayer: null this");
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

/**
 * Retrieve the maximum number of texture rows allowed to be written on each render cycle
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 *
 * @return 			Maximum number of texture rows to write
 */
s8 SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getTextureMaximumRowsToWrite: null this");

	return this->texturesMaximumRowsToWrite;
}

/**
 * Set the number of idle cycles before allowing texture wrinting
 *
 * @memberof								SpriteManager
 * @public
 *
 * @param this								Function scope
 * @param cyclesToWaitForSpriteTextureWriting		Number of idle cycles
 */
void SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager this, u8 cyclesToWaitForSpriteTextureWriting)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getTextureMaximumRowsToWrite: null this");

	this->cyclesToWaitForSpriteTextureWriting = cyclesToWaitForSpriteTextureWriting;
}

/**
 * Set the maximum number of texture rows allowed to be written on each render cycle
 *
 * @memberof								SpriteManager
 * @public
 *
 * @param this								Function scope
 * @param texturesMaximumRowsToWrite		Number of texture rows allowed to be written
 */
void SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager this, u8 texturesMaximumRowsToWrite)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::setMaximumTextureRowsToWrite: null this");

	this->texturesMaximumRowsToWrite = 2 > (s8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

/**
 * Set the flag to defer affine transformation calculations
 *
 * @memberof							SpriteManager
 * @public
 *
 * @param this							Function scope
 * @param deferParamTableEffects	Flag
 */
void SpriteManager_deferParamTableEffects(SpriteManager this, bool deferParamTableEffects)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::deferParamTableEffects: null this");

	this->deferParamTableEffects = deferParamTableEffects;
}

/**
 * Retrieve the maximum number of rows to compute per render cycle
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of affine transformation rows to compute
 */
int SpriteManager_getMaximumParamTableRowsToComputePerCall(SpriteManager this)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::getMaximumAffineRowsPerCall: null this");

	return this->deferParamTableEffects ? this->maximumParamTableRowsToComputePerCall : -1;
}

/**
 * Set the maximum number of affine transformation rows to compute per render cycle
 *
 * @memberof										SpriteManager
 * @public
 *
 * @param this										Function scope
 * @param maximumParamTableRowsToComputePerCall		Number of affine transformation rows to compute per render cycle
 */
void SpriteManager_setMaximumParamTableRowsToComputePerCall(SpriteManager this, int maximumParamTableRowsToComputePerCall)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::setMaximumAffineRowsToComputePerCall: null this");

	this->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}

/**
 * Print manager's status
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param x			Screen x coordinate
 * @param y			Screen y coordinate
 * @param resumed	If true prints info about all the Sprites in the list
 */
void SpriteManager_print(SpriteManager this, int x, int y, bool resumed)
{
	ASSERT(__SAFE_CAST(SpriteManager, this), "SpriteManager::print: null this");

	Printing_text(Printing_getInstance(), "SPRITES' USAGE", x, y++, NULL);
#ifdef __PROFILE_GAME
	Printing_text(Printing_getInstance(), "Total pixels:                ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), _totalPixelsToDraw, x + 17, y, NULL);
#endif
	Printing_text(Printing_getInstance(), "Last free layer:     ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->freeLayer, x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Free layers:         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), __TOTAL_LAYERS - 1 - VirtualList_getSize(this->sprites), x + 17, y, NULL);
	Printing_text(Printing_getInstance(), "Sprites' count:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->sprites), x + 17, y, NULL);

	if(resumed)
	{
		return;
	}

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

		if((__SCREEN_HEIGHT_IN_CHARS) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}
}
