/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Camera.h>
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

/**
 * @class 	SpriteManager
 * @extends Object
 * @ingroup graphics-2d-sprites
 */
implements SpriteManager : Object;
friend class Sprite;
friend class ObjectSpriteContainer;
friend class Texture;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SpriteManager::constructor(SpriteManager this);
static void SpriteManager::selectSpritePendingTextureWriting(SpriteManager this);
static bool SpriteManager::disposeSpritesProgressively(SpriteManager this);
static void SpriteManager::registerSprite(SpriteManager this, Sprite sprite);
static void SpriteManager::unregisterSprite(SpriteManager this, Sprite sprite);

#ifdef __PROFILE_GAME
int _totalPixelsToDraw = 0;
#endif

void ObjectSprite::checkForContainer(ObjectSprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SpriteManager::getInstance()
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
static void __attribute__ ((noinline)) SpriteManager::constructor(SpriteManager this)
{
	ASSERT(this, "SpriteManager::constructor: null this");

	// construct base object
	Base::constructor();

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
	this->evenFrame = true;

	SpriteManager::reset(this);
}

/**
 * Class destructor
 *
 * @memberof	SpriteManager
 * @public
 *
 * @param this	Function scope
 */
void SpriteManager::destructor(SpriteManager this)
{
	ASSERT(this, "SpriteManager::destructor: null this");

	if(this->sprites)
	{
		__DELETE(this->sprites);
		this->sprites = NULL;
	}

	if(this->spritesToDispose)
	{
		while(SpriteManager::disposeSpritesProgressively(this));

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
void SpriteManager::reset(SpriteManager this)
{
	ASSERT(this, "SpriteManager::reset: null this");

	this->lockSpritesLists = true;

	// must reset the ObjectSpriteContainerManager before the SpriteManager!
	ObjectSpriteContainerManager::reset(ObjectSpriteContainerManager::getInstance());

	if(this->spritesToDispose)
	{
		__DELETE(this->spritesToDispose);
		this->spritesToDispose = NULL;
	}

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		for(; node; node = node->next)
		{
			__DELETE(node->data);
		}

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

	SpriteManager::renderLastLayer(this);

	this->lockSpritesLists = false;
	this->evenFrame = true;
}

/**
 * Setup object sprite containers
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param size			Array with the number of OBJECTs per container
 * @param z				Z coordinate of each container
 */
void SpriteManager::setupObjectSpriteContainers(SpriteManager this, s16 size[__TOTAL_OBJECT_SEGMENTS], s16 z[__TOTAL_OBJECT_SEGMENTS])
{
	const ObjectSpriteContainer* objectSpriteContainers = ObjectSpriteContainerManager::setupObjectSpriteContainers(ObjectSpriteContainerManager::getInstance(), size, z);

	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		// only request a WORLD layer if can hold any OBJECT
		if(objectSpriteContainers[i]->totalObjects)
		{
			// register to sprite manager
			SpriteManager::registerSprite(this, __SAFE_CAST(Sprite, objectSpriteContainers[i]));
		}
	}
}

/**
 * Dispose sprite
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param sprite	Sprite to dispose
 */
Sprite SpriteManager::createSprite(SpriteManager this, SpriteDefinition* spriteDefinition, Object owner)
{
	ASSERT(this, "SpriteManager::createSprite: null this");
	ASSERT(spriteDefinition, "SpriteManager::createSprite: null spriteDefinition");

	this->lockSpritesLists = true;

	Sprite sprite = ((Sprite (*)(SpriteDefinition*, Object)) spriteDefinition->allocator)((SpriteDefinition*)spriteDefinition, owner);
	ASSERT(__IS_OBJECT_ALIVE(sprite), "SpriteManager::createSprite: failed creating sprite");

	SpriteManager::registerSprite(this, sprite);

	this->lockSpritesLists = false;

	return sprite;
}

/**
 * Dispose sprite
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param sprite	Sprite to dispose
 */
void SpriteManager::disposeSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::disposeSprite: null this");
	ASSERT(__IS_OBJECT_ALIVE(sprite), "SpriteManager::disposeSprite: trying to dispose dead sprite");

	this->lockSpritesLists = true;

	if(sprite && !VirtualList::find(this->spritesToDispose, sprite))
	{
		VirtualList::pushBack(this->spritesToDispose, sprite);

		Sprite::hide(sprite);
	}

	this->lockSpritesLists = false;
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
static bool SpriteManager::disposeSpritesProgressively(SpriteManager this)
{
	ASSERT(this, "SpriteManager::disposeSprites: null this");

	if(!this->lockSpritesLists && this->spritesToDispose->head)
	{
		this->lockSpritesLists = true;

		Sprite sprite = __SAFE_CAST(Sprite, VirtualList::popFront(this->spritesToDispose));

		SpriteManager::unregisterSprite(SpriteManager::getInstance(), sprite);

		__DELETE(sprite);

		this->spritePendingTextureWriting = __IS_OBJECT_ALIVE(this->spritePendingTextureWriting)? this->spritePendingTextureWriting : NULL;

		this->lockSpritesLists = false;

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
void SpriteManager::disposeSprites(SpriteManager this)
{
	ASSERT(this, "SpriteManager::disposeSprites: null this");

	if(this->spritesToDispose)
	{
		this->lockSpritesLists = false;
		while(SpriteManager::disposeSpritesProgressively(this));
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
void SpriteManager::sortLayers(SpriteManager this)
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

				// check if z positions are swapped
				if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
				{
					// get each entity's layer
					u8 worldLayer1 = sprite->worldLayer;
					u8 worldLayer2 = nextSprite->worldLayer;

					// swap layers
					Sprite::setWorldLayer(sprite, worldLayer2);
					Sprite::setWorldLayer(nextSprite, worldLayer1);

					// swap array entries
					VirtualNode::swapData(node, nextNode);

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
void SpriteManager::sortLayersProgressively(SpriteManager this)
{
	ASSERT(this, "SpriteManager::sortLayersProgressively: null this");

	this->zSortingFirstNode = this->zSortingFirstNode ? this->zSortingSecondNode ? this->zSortingFirstNode : this->zSortingFirstNode->next: this->sprites->head;

	for(; this->zSortingFirstNode; this->zSortingFirstNode = this->zSortingFirstNode->next)
	{
		this->zSortingSecondNode = this->zSortingFirstNode->next;

		if(this->zSortingSecondNode)
		{
			Sprite sprite = __SAFE_CAST(Sprite, this->zSortingFirstNode->data);
			Sprite nextSprite = __SAFE_CAST(Sprite, this->zSortingSecondNode->data);

			// check if z positions are swapped
			if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
			{
				Sprite sprite = __SAFE_CAST(Sprite, this->zSortingFirstNode->data);
				Sprite nextSprite = __SAFE_CAST(Sprite, this->zSortingSecondNode->data);

				// get each entity's layer
				u8 worldLayer1 = sprite->worldLayer;
				u8 worldLayer2 = nextSprite->worldLayer;

				// don't render inmediately, it causes glitches
				Sprite::setWorldLayer(nextSprite, worldLayer1);
				Sprite::setWorldLayer(sprite, worldLayer2);

				// swap nodes' data
				VirtualNode::swapData(this->zSortingFirstNode, this->zSortingSecondNode);

				this->zSortingFirstNode = this->zSortingSecondNode;
			}
		}
	}
}

/**
 * Register a Sprite and assign a WORLD layer to it
 *
 * @memberof		SpriteManager
 * @private
 *
 * @param this		Function scope
 * @param sprite	Sprite to assign the WORLD layer
 */
static void SpriteManager::registerSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::registerSprite: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::registerSprite: adding no sprite");

	if(!__GET_CAST(ObjectSprite, sprite))
	{
		s8 layer = 0;

		VirtualNode alreadyLoadedSpriteNode = VirtualList::find(this->sprites, sprite);

		ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::registerSprite: sprite already registered");

		if(!alreadyLoadedSpriteNode)
		{
			this->lockSpritesLists = true;

			// retrieve the next free layer, taking into account
			// if there are layers being freed up by the recovery algorithm
			layer = __TOTAL_LAYERS - 1;

			VirtualNode head = this->sprites->head;

			if(head)
			{
				layer = (__SAFE_CAST(Sprite, head->data))->worldLayer - 1;
			}

			NM_ASSERT(0 < layer, "SpriteManager::registerSprite: no more layers");

			// add to the front: last element corresponds to the 31 WORLD
			VirtualList::pushFront(this->sprites, sprite);

			this->zSortingFirstNode = NULL;
			this->zSortingSecondNode = NULL;
		}

		Sprite::setWorldLayer(sprite, layer);

		this->lockSpritesLists = false;

	}
}

/**
 * Remove a registered Sprite and get back the WORLD layer previously assigned to it
 *
 * @memberof		SpriteManager
 * @private
 *
 * @param this		Function scope
 * @param sprite	Sprite to assign the WORLD layer
 */
static void SpriteManager::unregisterSprite(SpriteManager this, Sprite sprite)
{
	ASSERT(this, "SpriteManager::unregisterSprite: null this");
	ASSERT(__SAFE_CAST(Sprite, sprite), "SpriteManager::unregisterSprite: removing no sprite");

	if(!__GET_CAST(ObjectSprite, sprite))
	{
		ASSERT(__GET_CAST(BgmapSprite, sprite), "SpriteManager::unregisterSprite: non bgmap sprite");
		NM_ASSERT(VirtualList::find(this->sprites, sprite), "SpriteManager::unregisterSprite: sprite not found");

		// check if exists
		VirtualList::removeElement(this->sprites, sprite);

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
			Sprite sprite = __SAFE_CAST(Sprite, node->data);
			ASSERT(spriteLayer-- == sprite->worldLayer + 1, "SpriteManager::unregisterSprite: wrong layers");

			// move the sprite to the freed layer
			Sprite::setWorldLayer(sprite, sprite->worldLayer + 1);
		}

		// sorting needs to restart
		this->zSortingFirstNode = NULL;
		this->zSortingSecondNode = NULL;
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
void SpriteManager::renderLastLayer(SpriteManager this)
{
	ASSERT(this, "SpriteManager::renderLastLayer: null this");

	ASSERT(0 <= (s8)this->freeLayer, "SpriteManager::renderLastLayer: no more layers");
	ASSERT(__TOTAL_LAYERS > VirtualList::getSize(this->sprites), "SpriteManager::renderLastLayer: no more free layers");

	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing::render(Printing::getInstance(), this->freeLayer);

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
static void SpriteManager::selectSpritePendingTextureWriting(SpriteManager this)
{
	ASSERT(this, "SpriteManager::selectSpritePendingTextureWriting: null this");

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		if(__IS_OBJECT_ALIVE(sprite) && ! Sprite::areTexturesWritten(sprite))
		{
			bool areTexturesWritten =  Sprite::writeTextures(sprite);

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
void SpriteManager::writeTextures(SpriteManager this)
{
	ASSERT(this, "SpriteManager::writeTextures: null this");

	CharSetManager::writeCharSets(CharSetManager::getInstance());

	s8 texturesMaximumRowsToWrite = this->texturesMaximumRowsToWrite;

	// allow complete texture writing
	this->texturesMaximumRowsToWrite = -1;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		 Sprite::writeTextures(node->data);
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
static bool SpriteManager::writeSelectedSprite(SpriteManager this)
{
	ASSERT(this, "SpriteManager::writeSelectedSprite: null this");

	bool textureWritten = false;

	if(!this->waitToWriteSpriteTextures)
	{
		if(this->spritePendingTextureWriting)
		{
			if(__IS_OBJECT_ALIVE(this->spritePendingTextureWriting) && ! Sprite::areTexturesWritten(this->spritePendingTextureWriting))
			{
				this->spritePendingTextureWriting =  Sprite::writeTextures(this->spritePendingTextureWriting) ? this->spritePendingTextureWriting : NULL;
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
			SpriteManager::selectSpritePendingTextureWriting(this);
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

void SpriteManager::render(SpriteManager this)
{
	ASSERT(this, "SpriteManager::render: null this");

	// switch between even and odd frame
	this->evenFrame = !this->evenFrame;

	// must dispose sprites before doing anything else in order to try to make room in DRAM to new sprites
	// as soon as possible

	bool skipNonCriticalProcesses = SpriteManager::disposeSpritesProgressively(this);
	skipNonCriticalProcesses |= CharSetManager::writeCharSetsProgressively(CharSetManager::getInstance());

	// write textures
	if(!skipNonCriticalProcesses && !SpriteManager::writeSelectedSprite(this))
	{
		// defragment param table
		if(!ParamTableManager::defragmentProgressively(ParamTableManager::getInstance()))
		{
			// z sorting
        	SpriteManager::sortLayersProgressively(this);
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

	for(; node; node = node->next)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		// first update
		if((u32)sprite->animationController)
		{
			Sprite::update(__SAFE_CAST(Sprite, sprite));
		}

		if(sprite->hidden)
		{
			_worldAttributesBaseAddress[sprite->worldLayer].head = __WORLD_OFF;
		}
		else
		{
			 Sprite::render(sprite, this->evenFrame);

			if(!sprite->visible)
			{
				_worldAttributesBaseAddress[sprite->worldLayer].head = __WORLD_OFF;
			}
		}
	}

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
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
	SpriteManager::renderLastLayer(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		static int counter = __TARGET_FPS;

		if(0 >= --counter)
		{
			counter = __TARGET_FPS;
			SpriteManager::print(this, 1, 15, true);
		}
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
u8 SpriteManager::getFreeLayer(SpriteManager this)
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
void SpriteManager::showLayer(SpriteManager this, u8 layer)
{
	ASSERT(this, "SpriteManager::showLayer: null this");

	VirtualNode node = this->sprites->tail;

	for(; node; node = node->previous)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		if(sprite->worldLayer != layer)
		{
			 Sprite::hide(sprite);
		}
		else
		{
			 Sprite::show(sprite);
		}

		Sprite::setPosition(sprite, &sprite->position);

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
void SpriteManager::recoverLayers(SpriteManager this)
{
	ASSERT(this, "SpriteManager::recoverLayers: null this");

	VirtualNode node = this->sprites->tail;
	for(; node; node = node->previous)
	{
		Sprite sprite = __SAFE_CAST(Sprite, node->data);

		 Sprite::show(sprite);

		Sprite::setPosition(sprite, &sprite->position);

		_worldAttributesBaseAddress[sprite->worldLayer].head &= ~__WORLD_END;
	}

	SpriteManager::renderLastLayer(this);
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
Sprite SpriteManager::getSpriteAtLayer(SpriteManager this, u8 layer)
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
s8 SpriteManager::getTexturesMaximumRowsToWrite(SpriteManager this)
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
 * @param cyclesToWaitForSpriteTextureWriting		Number of idle cycles
 */
void SpriteManager::setCyclesToWaitForTextureWriting(SpriteManager this, u8 cyclesToWaitForSpriteTextureWriting)
{
	ASSERT(this, "SpriteManager::getTextureMaximumRowsToWrite: null this");

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
void SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager this, u8 texturesMaximumRowsToWrite)
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
 * @param deferParamTableEffects	Flag
 */
void SpriteManager::deferParamTableEffects(SpriteManager this, bool deferParamTableEffects)
{
	ASSERT(this, "SpriteManager::deferParamTableEffects: null this");

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
int SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager this)
{
	ASSERT(this, "SpriteManager::getMaximumAffineRowsPerCall: null this");

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
void SpriteManager::setMaximumParamTableRowsToComputePerCall(SpriteManager this, int maximumParamTableRowsToComputePerCall)
{
	ASSERT(this, "SpriteManager::setMaximumAffineRowsToComputePerCall: null this");

	this->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}

/**
 * Print manager's status
 *
 * @memberof		SpriteManager
 * @public
 *
 * @param this		Function scope
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 * @param resumed	If true prints info about all the Sprites in the list
 */
void SpriteManager::print(SpriteManager this, int x, int y, bool resumed)
{
	ASSERT(this, "SpriteManager::print: null this");

	Printing::text(Printing::getInstance(), "SPRITES' USAGE", x, y++, NULL);
#ifdef __PROFILE_GAME
	Printing::text(Printing::getInstance(), "Total pixels:                ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), _totalPixelsToDraw, x + 17, y, NULL);
#endif
	Printing::text(Printing::getInstance(), "Last free layer:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->freeLayer, x + 17, y, NULL);
	Printing::text(Printing::getInstance(), "Free layers:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __TOTAL_LAYERS - 1 - VirtualList::getSize(this->sprites), x + 17, y, NULL);
	Printing::text(Printing::getInstance(), "Sprites' count:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->sprites), x + 17, y, NULL);
	Printing::text(Printing::getInstance(), "Disposed sprites:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->spritesToDispose ? VirtualList::getSize(this->spritesToDispose) : 0, x + 17, y, NULL);

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

		Printing::int(Printing::getInstance(), Sprite::getWorldLayer(sprite), auxX, auxY, NULL);
		Printing::text(Printing::getInstance(), ": ", auxX + 2, auxY, NULL);
		Printing::text(Printing::getInstance(), spriteClassName, auxX + 4, auxY, NULL);
		Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[sprite->worldLayer].head, auxX + 14, auxY, 4, NULL);

		if((__SCREEN_HEIGHT_IN_CHARS) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}
}
