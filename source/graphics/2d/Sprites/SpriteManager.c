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
#include <CharSetManager.h>
#include <VIPManager.h>
#include <Screen.h>
#include <Printing.h>
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
	VirtualList spritesToDispose;																				\
	/**
	 * @var VirtualNode	node
	 * @brief 			sorting nodes
	 * @memberof		SpriteManager
	 */																									\
	VirtualNode node;																					\
	/**
	 * @var VirtualNode	nextNode
	 * @brief
	 * @memberof		SpriteManager
	 */																									\
	VirtualNode nextNode;																				\
	/**
	 * @var Texture		textureToWrite
	 * @brief 			texture writing
	 * @memberof		SpriteManager
	 */																									\
	Texture textureToWrite;																				\
	/**
	 * @var u8			freeLayer
	 * @brief 			next world layer
	 * @memberof		SpriteManager
	 */																									\
	u8 freeLayer;																						\
	/**
	 * @var s8			cyclesToWaitForTextureWriting
	 * @brief 			number of cycles that the texture writing is idle
	 * @memberof		SpriteManager
	 */																									\
	s8 cyclesToWaitForTextureWriting;																	\
	/**
	 * @var s8			texturesMaximumRowsToWrite
	 * @brief 			number of rows to write in texture's writing
	 * @memberof		SpriteManager
	 */																									\
	s8 texturesMaximumRowsToWrite;																		\
	/**
	 * @var s8			maximumAffineRowsToComputePerCall
	 * @brief 			number of rows to write in affine transformations
	 * @memberof		SpriteManager
	 */																									\
	s8 maximumAffineRowsToComputePerCall;																\
	/**
	 * @var s8			deferAffineTransformations
	 * @brief 			flag to control texture's writing deferring
	 * @memberof		SpriteManager
	 */																									\
	s8 deferAffineTransformations;																		\
	/**
	 * @var s8			waitToWrite
	 * @brief 			delay before writing again
	 * @memberof		SpriteManager
	 */																									\
	s8 waitToWrite;																						\

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
static void SpriteManager_selectTextureToWrite(SpriteManager this);
static bool SpriteManager_disposeSprites(SpriteManager this);

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
	// construct base object
	__CONSTRUCT_BASE(Object);

	this->node = NULL;
	this->nextNode = NULL;

	this->sprites = NULL;
	this->spritesToDispose = NULL;

	this->textureToWrite = NULL;
	this->cyclesToWaitForTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->maximumAffineRowsToComputePerCall = -1;
	this->deferAffineTransformations = false;
	this->waitToWrite = 0;

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
	ASSERT(this, "SpriteManager::destructor: null this");

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	if(this->spritesToDispose)
	{
		while(SpriteManager_disposeSprites(this));

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
	ASSERT(this, "SpriteManager::reset: null this");

	// must reset the ObjectSpriteContainerManager before the SpriteManager!
	ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager_getInstance());

	if(this->spritesToDispose)
	{
		while(SpriteManager_disposeSprites(this));

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

	this->node = NULL;
	this->nextNode = NULL;
	this->textureToWrite = NULL;
	this->cyclesToWaitForTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->waitToWrite = 0;

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
	ASSERT(this, "SpriteManager::disposeSprite: null this");
	ASSERT(__IS_OBJECT_ALIVE(sprite), "SpriteManager::disposeSprite: trying to dispose dead sprite");

	if(sprite && !VirtualList_find(this->spritesToDispose, sprite))
	{
		VirtualList_pushBack(this->spritesToDispose, sprite);

		Sprite_hide(sprite);
	}
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
static bool SpriteManager_disposeSprites(SpriteManager this)
{
	ASSERT(this, "SpriteManager::disposeSprites: null this");

	if(this->spritesToDispose->head)
	{
		Sprite sprite = __SAFE_CAST(Sprite, VirtualList_front(this->spritesToDispose));

		__DELETE(sprite);

		VirtualList_popFront(this->spritesToDispose);

		this->textureToWrite = NULL;

		return true;
	}

	return false;
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
				Sprite sprite = __SAFE_CAST(Sprite, this->node->data);
				Sprite nextSprite = __SAFE_CAST(Sprite, this->nextNode->data);

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
	ASSERT(this, "SpriteManager::getWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::getWorldLayer: adding no sprite");

	s8 layer = 0;

#ifdef __DEBUG
	VirtualNode alreadyLoadedSpriteNode = VirtualList_find(this->sprites, sprite);

	ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::getWorldLayer: sprite already registered");

	if(!alreadyLoadedSpriteNode)
	{
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

		this->node = NULL;
		this->nextNode = NULL;

#ifdef __DEBUG
	}
#endif

	Sprite_setWorldLayer(sprite, layer);
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
	ASSERT(this, "SpriteManager::relinquishWorldLayer: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::relinquishWorldLayer: removing no sprite");

	ASSERT(VirtualList_find(this->sprites, sprite), "SpriteManager::relinquishWorldLayer: sprite not found");

	// check if exists
	if(VirtualList_removeElement(this->sprites, sprite))
	{
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
		this->node = NULL;
		this->nextNode = NULL;
	}
	else
	{
		ASSERT(false, "SpriteManager::relinquishWorldLayer: sprite not registered");
	}
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
	ASSERT(this, "SpriteManager::setLastLayer: null this");

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
 * Select the texture to write
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 */
static void SpriteManager_selectTextureToWrite(SpriteManager this)
{
	ASSERT(this, "SpriteManager::selectTextureToWrite: null this");

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Texture texture = (__SAFE_CAST(Sprite, node->data))->texture;

		if(__IS_OBJECT_ALIVE(texture) && !texture->written && texture->textureDefinition)
		{
			__VIRTUAL_CALL(Texture, write, texture);

			this->waitToWrite = this->cyclesToWaitForTextureWriting;
			this->textureToWrite = !texture->written? texture : NULL;
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
	ASSERT(this, "SpriteManager::writeTextures: null this");

	CharSetManager_writeCharSets(CharSetManager_getInstance());

	s8 texturesMaximumRowsToWrite = this->texturesMaximumRowsToWrite;

	// allow complete texture writing
	this->texturesMaximumRowsToWrite = -1;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Texture texture = (__SAFE_CAST(Sprite, node->data))->texture;

		if(__IS_OBJECT_ALIVE(texture) && !texture->written && texture->textureDefinition)
		{
			__VIRTUAL_CALL(Texture, write, texture);
		}
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
static void SpriteManager_writeSelectedTexture(SpriteManager this)
{
	ASSERT(this, "SpriteManager::writeSelectedTexture: null this");

	if(!this->waitToWrite)
	{
		if(this->textureToWrite)
		{
			if(__IS_OBJECT_ALIVE(this->textureToWrite))
			{
				__VIRTUAL_CALL(Texture, write, this->textureToWrite);

				this->textureToWrite = !this->textureToWrite->written && this->textureToWrite->textureDefinition? this->textureToWrite : NULL;
				this->waitToWrite = this->cyclesToWaitForTextureWriting;
			}
			else
			{
				this->textureToWrite = NULL;
			}
		}
		else
		{
			SpriteManager_selectTextureToWrite(this);
		}
	}
	else
	{
		this->waitToWrite--;
	}
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
	ASSERT(this, "SpriteManager::render: null this");

	// write textures
	SpriteManager_writeSelectedTexture(this);

	if(!SpriteManager_disposeSprites(this))
	{
		// z sorting
		SpriteManager_sortLayersProgressively(this);
	}

	VirtualNode node = this->sprites->head;


#ifdef __PROFILE_GAME
	if(!Game_isInSpecialMode(Game_getInstance()))
	{
		_totalPixelsToDraw = __SCREEN_WIDTH * __SCREEN_HEIGHT;
	}
#endif

	if(!node)
	{
		this->freeLayer = __TOTAL_LAYERS - 1;
	}
	else
	{
		this->freeLayer = (__SAFE_CAST(Sprite, node->data))->worldLayer - 1;
	}

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		// first update
		if(!sprite->hidden && ((sprite->texture && sprite->texture->written && sprite->animationController) || sprite->transparent))
		{
			Sprite_update(__SAFE_CAST(Sprite, sprite));
		}

		if(sprite->hidden || !sprite->visible)
		{
			_worldAttributesBaseAddress[sprite->worldLayer].head = __WORLD_OFF;
#ifdef __PROFILE_GAME
			if(!Game_isInSpecialMode(Game_getInstance()) && (!sprite->hidden && sprite->transparent))
			{
				_totalPixelsToDraw += Sprite_getWorldWidth(sprite) * Sprite_getWorldHeight(sprite);
			}
#endif
		}
		else
		{
			__VIRTUAL_CALL(Sprite, render, sprite);
#ifdef __PROFILE_GAME
			if(!Game_isInSpecialMode(Game_getInstance()))
			{
				_totalPixelsToDraw += Sprite_getWorldWidth(sprite) * Sprite_getWorldHeight(sprite);
			}
#endif
		}
	}

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
	ASSERT(this, "SpriteManager::getFreeLayer: null this");

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
	ASSERT(this, "SpriteManager::getTextureMaximumRowsToWrite: null this");

	return this->texturesMaximumRowsToWrite;
}

/**
 * Set the number of idle cycles before allowing texture wrinting
 *
 * @memberof								SpriteManager
 * @public
 *
 * @param this								Function scope
 * @param cyclesToWaitForTextureWriting		Number of idle cycles
 */
void SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager this, u8 cyclesToWaitForTextureWriting)
{
	ASSERT(this, "SpriteManager::getTextureMaximumRowsToWrite: null this");

	this->cyclesToWaitForTextureWriting = cyclesToWaitForTextureWriting;
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
	ASSERT(this, "SpriteManager::setMaximumTextureRowsToWrite: null this");

	this->texturesMaximumRowsToWrite = 2 > (s8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

/**
 * Set the flag to defer affine transformation calculations
 *
 * @memberof							SpriteManager
 * @public
 *
 * @param this							Function scope
 * @param deferAffineTransformations	Flag
 */
void SpriteManager_deferAffineTransformations(SpriteManager this, bool deferAffineTransformations)
{
	ASSERT(this, "SpriteManager::deferAffineTransformations: null this");

	this->deferAffineTransformations = deferAffineTransformations;
}

/**
 * Retrieve the maximum number of affine transformation rows to compute per render cycle
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Number of affine transformation rows to compute
 */
int SpriteManager_getMaximumAffineRowsToComputePerCall(SpriteManager this)
{
	ASSERT(this, "SpriteManager::getMaximumAffineRowsPerCall: null this");

	return this->deferAffineTransformations ? this->maximumAffineRowsToComputePerCall : -1;
}

/**
 * Set the maximum number of affine transformation rows to compute per render cycle
 *
 * @memberof									SpriteManager
 * @public
 *
 * @param this									Function scope
 * @param maximumAffineRowsToComputePerCall		Number of affine transformation rows to compute per render cycle
 */
void SpriteManager_setMaximumAffineRowsToComputePerCall(SpriteManager this, int maximumAffineRowsToComputePerCall)
{
	ASSERT(this, "SpriteManager::setMaximumAffineRowsToComputePerCall: null this");

	this->maximumAffineRowsToComputePerCall = maximumAffineRowsToComputePerCall;
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
	ASSERT(this, "SpriteManager::print: null this");

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

		if((__SCREEN_HEIGHT >> 3) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}
}
