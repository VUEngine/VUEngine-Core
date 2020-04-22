/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <VIPManager.h>
#include <ParamTableManager.h>
#include <CharSetManager.h>
#include <Camera.h>
#include <TimerManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_SPRITE_CLASS_NAME_SIZE			14


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Sprite;
friend class ObjectSpriteContainer;
friend class Texture;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SpriteManager::getInstance()
 * @memberof	SpriteManager
 * @public
 * @return		SpriteManager instance
 */

/**
 * Class constructor
 *
 * @private
 */
void SpriteManager::constructor()
{
	// construct base object
	Base::constructor();

	this->totalPixelsDrawn = 0;

	this->sprites = NULL;
	this->spritesToDispose = NULL;
	this->objectSpriteContainers = NULL;

	this->spritePendingTextureWriting = NULL;
	this->cyclesToWaitForSpriteTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->waitToWriteSpriteTextures = 0;
	this->bypassSpriteUpdateWhenWritingTextures = false;
	this->lockSpritesLists = false;
	this->evenFrame = true;

	SpriteManager::reset(this);
}

/**
 * Class destructor
 */
void SpriteManager::destructor()
{
	SpriteManager::cleanUp(this);

	// allow a new construct
	Base::destructor();
}

/**
 * Clean up lists
 */
void SpriteManager::cleanUp()
{
	if(this->spritesToDispose)
	{
		delete this->spritesToDispose;
		this->spritesToDispose = NULL;
	}

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		for(; node; node = node->next)
		{
			VirtualList::removeElement(this->objectSpriteContainers, node->data);
			delete node->data;
		}

		delete this->sprites;
		this->sprites = NULL;
	}

	if(this->objectSpriteContainers)
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->objectSpriteContainers;
		this->objectSpriteContainers = NULL;
	}
}

/**
 * Reset manager's state
 */
void SpriteManager::reset()
{
	this->lockSpritesLists = true;

	SpriteManager::cleanUp(this);

	int i = 0;
	// clean OBJ memory
	for(; i < __AVAILABLE_CHAR_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesBaseAddress[(i << 2) + 0] = 0;
		_objectAttributesBaseAddress[(i << 2) + 1] = 0;
		_objectAttributesBaseAddress[(i << 2) + 2] = 0;
		_objectAttributesBaseAddress[(i << 2) + 3] = 0;
	}

	this->sprites = new VirtualList();
	this->spritesToDispose = new VirtualList();
	this->objectSpriteContainers = new VirtualList();

	this->freeLayer = __TOTAL_LAYERS - 1;

	this->spritePendingTextureWriting = NULL;
	this->cyclesToWaitForSpriteTextureWriting = 0;
	this->texturesMaximumRowsToWrite = -1;
	this->waitToWriteSpriteTextures = 0;
	this->bypassSpriteUpdateWhenWritingTextures = false;

	SpriteManager::renderLastLayer(this);

	this->lockSpritesLists = false;
	this->evenFrame = true;
}

/**
 * Setup object sprite containers
 *
 * @param size			Array with the number of OBJECTs per container
 * @param z				Z coordinate of each container
 */
void SpriteManager::setupObjectSpriteContainers(s16 size[__TOTAL_OBJECT_SEGMENTS], s16 z[__TOTAL_OBJECT_SEGMENTS])
{
	int availableObjects = __AVAILABLE_CHAR_OBJECTS;
#ifndef __RELEASE
	s16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	if(this->objectSpriteContainers && VirtualList::getSize(this->objectSpriteContainers))
	{
		return;
	}

	// must add them from __SPT3 to __SPT0
	// so each they start presorted in the WORLDS

	int spt = __TOTAL_OBJECT_SEGMENTS - 1;
	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "SpriteManager::setupObjectSpriteContainers: wrong z");

		if(0 <= size[i])
		{
			availableObjects -= size[i];
			NM_ASSERT(0 <= availableObjects, "SpriteManager::setupObjectSpriteContainers: OBJs depleted");
			ObjectSpriteContainer objectSpriteContainer = new ObjectSpriteContainer(spt--, size[i], availableObjects);
			VirtualList::pushBack(this->objectSpriteContainers, objectSpriteContainer);

			PixelVector position =
			{
					0, 0, z[i], 0
			};

			if(size[i])
			{
				SpriteManager::registerSprite(this, Sprite::safeCast(objectSpriteContainer));
				ObjectSpriteContainer::setPosition(objectSpriteContainer, &position);
			}

#ifndef __RELEASE
			previousZ = z[i];
#endif
		}
	}
}

/**
 * Retrieve an ObjectSpriteContainer capable of allocating the given number of OBJECTs and close to the given z coordinate
 *
 * @param numberOfObjects		Number of OBJECTs required
 * @param z						Z coordinate
 * @return 						ObjectSpriteContainer instance
 */
ObjectSpriteContainer SpriteManager::getObjectSpriteContainer(int numberOfObjects, fix10_6 z)
{
	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; node; node = node->next)
	{
		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		if(ObjectSpriteContainer::hasRoomFor(objectSpriteContainer, numberOfObjects))
		{
			if(!suitableObjectSpriteContainer)
			{
				suitableObjectSpriteContainer = objectSpriteContainer;
			}
			else
			{
				if(__ABS(Sprite::getPosition(objectSpriteContainer)->z - z) < __ABS(Sprite::getPosition(suitableObjectSpriteContainer)->z - z))
				{
					suitableObjectSpriteContainer = objectSpriteContainer;
				}
			}
		}
	}

	NM_ASSERT(suitableObjectSpriteContainer, "SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers available");

	return suitableObjectSpriteContainer;
}

/**
 * Set flag to bypass sprite update
 *
 * @param bypassSpriteUpdateWhenWritingTextures		bool
 */
void SpriteManager::bypassSpriteUpdateWhenWritingTextures(bool bypassSpriteUpdateWhenWritingTextures)
{
	this->bypassSpriteUpdateWhenWritingTextures = bypassSpriteUpdateWhenWritingTextures;
}

/**
 * Retrieve the SpriteManager for the given segment
 *
 * @param segment		Spt segment
 * @return 				ObjectSpriteContainer instance
 */
ObjectSpriteContainer SpriteManager::getObjectSpriteContainerBySegment(int segment)
{
	ASSERT((unsigned)segment < __TOTAL_OBJECT_SEGMENTS, "SpriteManager::getObjectSpriteContainerBySegment: invalid segment");

	if((unsigned)segment > __TOTAL_OBJECT_SEGMENTS)
	{
		return NULL;
	}

	ObjectSpriteContainer objectSpriteContainer = NULL;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; node; node = node->next)
	{
		objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		if(objectSpriteContainer->spt == segment)
		{
			return objectSpriteContainer;
		}
	}

	return NULL;
}

/**
 * Dispose sprite
 *
 * @param sprite	Sprite to dispose
 */
Sprite SpriteManager::createSprite(SpriteSpec* spriteSpec, Object owner)
{
	ASSERT(spriteSpec, "SpriteManager::createSprite: null spriteSpec");
	ASSERT(spriteSpec->allocator, "SpriteManager::createSprite: no sprite allocator");

	this->lockSpritesLists = true;

	Sprite sprite = ((Sprite (*)(SpriteSpec*, Object)) spriteSpec->allocator)((SpriteSpec*)spriteSpec, owner);
	ASSERT(!isDeleted(sprite), "SpriteManager::createSprite: failed creating sprite");

	SpriteManager::registerSprite(this, sprite);

	this->lockSpritesLists = false;

	return sprite;
}

/**
 * Dispose sprite
 *
 * @param sprite	Sprite to dispose
 */
void SpriteManager::disposeSprite(Sprite sprite)
{
	ASSERT(!isDeleted(sprite), "SpriteManager::disposeSprite: trying to dispose dead sprite");

	if(isDeleted(sprite))
	{
		return;
	}

	this->lockSpritesLists = true;

	Sprite::releaseTexture(sprite);

	if(!__GET_CAST(ObjectSprite, sprite))
	{
		if(sprite && !VirtualList::find(this->spritesToDispose, sprite))
		{
			VirtualList::pushBack(this->spritesToDispose, sprite);

			Sprite::disposed(sprite);
		}
	}
	else
	{
		delete sprite;
	}

	this->lockSpritesLists = false;
}

/**
 * Delete disposable sprites progressively
 *
 * @return 		True if there were a sprite to delete
 */
bool SpriteManager::disposeSpritesProgressively()
{
	if(!this->lockSpritesLists && this->spritesToDispose->head)
	{
		this->lockSpritesLists = true;

		Sprite sprite = Sprite::safeCast(VirtualList::popFront(this->spritesToDispose));

		SpriteManager::unregisterSprite(this, sprite);

		delete sprite;

		this->spritePendingTextureWriting = !isDeleted(this->spritePendingTextureWriting)? this->spritePendingTextureWriting : NULL;

		this->lockSpritesLists = false;

		return true;
	}

	return false;
}

/**
 * Delete disposable sprites
 *
 * @return 		True if there were a sprite to delete
 */
void SpriteManager::disposeSprites()
{
	if(this->spritesToDispose)
	{
		this->lockSpritesLists = false;
		while(SpriteManager::disposeSpritesProgressively(this));
	}
}

/**
 * Sort sprites according to their z coordinate
 */
void SpriteManager::sort()
{
	while(SpriteManager::sortProgressively(this));
}

// check if any entity must be assigned another world layer
/**
 * Deferred sorting sprites according to their z coordinate
 */
bool SpriteManager::sortProgressively()
{
	if(this->lockSpritesLists)
	{
		return false;
	}

	bool swapped = false;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		VirtualNode nextNode = node->next;

		if(nextNode)
		{
			Sprite sprite = Sprite::safeCast(node->data);
			Sprite nextSprite = Sprite::safeCast(nextNode->data);

			// check if z positions are swapped
			if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
			{
				// swap nodes' data
				VirtualNode::swapData(node, nextNode);

				node = nextNode;

				swapped = true;
			}
		}
	}

	return swapped;
}

/**
 * Register a Sprite and assign a WORLD layer to it
 *
 * @private
 * @param sprite	Sprite to assign the WORLD layer
 */
void SpriteManager::registerSprite(Sprite sprite)
{
	ASSERT(Sprite::safeCast(sprite), "SpriteManager::registerSprite: adding no sprite");

	if(!__GET_CAST(ObjectSprite, sprite))
	{
		VirtualNode alreadyLoadedSpriteNode = VirtualList::find(this->sprites, sprite);

		NM_ASSERT(!alreadyLoadedSpriteNode, "SpriteManager::registerSprite: sprite already registered");

		if(!alreadyLoadedSpriteNode)
		{
			this->lockSpritesLists = true;

			// add to the front: last element corresponds to the 31 WORLD
			VirtualList::pushFront(this->sprites, sprite);

			this->lockSpritesLists = false;
		}
	}
}

/**
 * Remove a registered Sprite and get back the WORLD layer previously assigned to it
 *
 * @private
 * @param sprite	Sprite to assign the WORLD layer
 */
void SpriteManager::unregisterSprite(Sprite sprite)
{
	ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

	if(!__GET_CAST(ObjectSprite, sprite))
	{
		ASSERT(__GET_CAST(BgmapSprite, sprite), "SpriteManager::unregisterSprite: non bgmap sprite");

		NM_ASSERT(!isDeleted(VirtualList::find(this->sprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");

		// check if exists
		VirtualList::removeElement(this->sprites, sprite);
	}
}

/**
 * Render the WORLD destined to printing output
 */
void SpriteManager::renderLastLayer()
{
	NM_ASSERT(0 <= (s8)this->freeLayer, "SpriteManager::renderLastLayer: no more layers");
	NM_ASSERT(__TOTAL_LAYERS > VirtualList::getSize(this->sprites), "SpriteManager::renderLastLayer: no more free layers");

	this->freeLayer = 0 < this->freeLayer ? this->freeLayer : 0;

	Printing::render(Printing::getInstance(), this->freeLayer);

	if(0 < this->freeLayer)
	{
		_worldAttributesBaseAddress[this->freeLayer - 1].head = __WORLD_END;
	}
}

/**
 * Select the sprite to write
 */
void SpriteManager::selectSpritePendingTextureWriting()
{
	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(!isDeleted(sprite) && ! Sprite::areTexturesWritten(sprite))
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
 */
void SpriteManager::writeTextures()
{
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
 */
bool SpriteManager::writeSelectedSprite()
{
	if(this->lockSpritesLists)
	{
		return false;
	}

	bool textureWritten = false;

	if(!this->waitToWriteSpriteTextures)
	{
		if(this->spritePendingTextureWriting)
		{
			if(!isDeleted(this->spritePendingTextureWriting) && ! Sprite::areTexturesWritten(this->spritePendingTextureWriting))
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
 */
void SpriteManager::render()
{
	// switch between even and odd frame
	this->evenFrame = !this->evenFrame;

	VIPManager vipManager = VIPManager::getInstance();

	// must dispose sprites before doing anything else in order to try to make room in DRAM to new sprites
	// as soon as possible
	SpriteManager::disposeSpritesProgressively(this);

	SpriteManager::sortProgressively(this);

	VirtualNode node = this->sprites->tail;

	this->freeLayer = __TOTAL_LAYERS - 1;

	for(; node && 0 < this->freeLayer; node = node->previous)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->hidden | sprite->disposed)
		{
			continue;
		}
		else
		{
			Sprite::updateTransparency(sprite, this->evenFrame);

			if(!sprite->visible)
			{
				continue;
			}
			else
			{
				if((u32)sprite->animationController && !VIPManager::hasFrameStarted(vipManager))
				{
					Sprite::update(sprite);
				}

				if(Sprite::render(sprite, this->freeLayer))
				{
					this->freeLayer--;
				}
			}
		}
	}

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		SpriteManager::computeTotalPixelsDrawn(this);
	}
#endif

	// configure printing layer and shutdown unused layers
	SpriteManager::renderLastLayer(this);

	if(!VIPManager::hasFrameStarted(vipManager) && !CharSetManager::writeCharSetsProgressively(CharSetManager::getInstance()))
	{
		if(!VIPManager::hasFrameStarted(vipManager) && !SpriteManager::writeSelectedSprite(this))
		{
			ParamTableManager::defragmentProgressively(ParamTableManager::getInstance());
		}
	}

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		static int counter = __TARGET_FPS / 10;

		if(0 >= --counter)
		{
			counter = __TARGET_FPS / 10;
			SpriteManager::print(this, 1, 15, true);
		}
	}
#endif
}

/**
 * Retrieve the next free WORLD layer
 *
 * @return			Free WORLD layer
 */
u8 SpriteManager::getFreeLayer()
{
	return this->freeLayer;
}

/**
 * Show the Sprite in the given WORLD layer and hide the rest
 *
 * @param layer		WORLD layer to show
 */
void SpriteManager::showLayer(u8 layer)
{
	VirtualNode node = this->sprites->tail;

	for(; node; node = node->previous)
	{
		Sprite sprite = Sprite::safeCast(node->data);

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
 */
void SpriteManager::recoverLayers()
{
	VirtualNode node = this->sprites->tail;
	for(; node; node = node->previous)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		Sprite::show(sprite);

		Sprite::setPosition(sprite, &sprite->position);

		_worldAttributesBaseAddress[sprite->worldLayer].head &= ~__WORLD_END;
	}

	SpriteManager::renderLastLayer(this);
}

/**
 * Retrieve the Sprite assigned to the given WORLD
 *
 * @param layer		WORLD layer to show
 * @return			Sprite with the given WORLD layer
 */
Sprite SpriteManager::getSpriteAtLayer(u8 layer)
{
	ASSERT((unsigned)layer < __TOTAL_LAYERS, "SpriteManager::getSpriteAtLayer: invalid layer");

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		if((Sprite::safeCast(node->data))->worldLayer == layer)
		{
			return Sprite::safeCast(node->data);
		}
	}

	return NULL;
}

/**
 * Retrieve the maximum number of texture rows allowed to be written on each render cycle
 *
 * @return 			Maximum number of texture rows to write
 */
s8 SpriteManager::getTexturesMaximumRowsToWrite()
{
	return this->texturesMaximumRowsToWrite;
}

/**
 * Set the number of idle cycles before allowing texture wrinting
 *
 * @param cyclesToWaitForSpriteTextureWriting		Number of idle cycles
 */
void SpriteManager::setCyclesToWaitForTextureWriting(u8 cyclesToWaitForSpriteTextureWriting)
{
	this->cyclesToWaitForSpriteTextureWriting = cyclesToWaitForSpriteTextureWriting;
}

/**
 * Set the maximum number of texture rows allowed to be written on each render cycle
 *
 * @param texturesMaximumRowsToWrite		Number of texture rows allowed to be written
 */
void SpriteManager::setTexturesMaximumRowsToWrite(u8 texturesMaximumRowsToWrite)
{
	this->texturesMaximumRowsToWrite = 2 > (s8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

/**
 * Set the flag to defer affine transformation calculations
 *
 * @param deferParamTableEffects	Flag
 */
void SpriteManager::deferParamTableEffects(bool deferParamTableEffects)
{
	this->deferParamTableEffects = deferParamTableEffects;
}

/**
 * Retrieve the maximum number of rows to compute per render cycle
 *
 * @return			Number of affine transformation rows to compute
 */
int SpriteManager::getMaximumParamTableRowsToComputePerCall()
{
	return this->deferParamTableEffects ? this->maximumParamTableRowsToComputePerCall : -1;
}

/**
 * Set the maximum number of affine transformation rows to compute per render cycle
 *
 * @param maximumParamTableRowsToComputePerCall		Number of affine transformation rows to compute per render cycle
 */
void SpriteManager::setMaximumParamTableRowsToComputePerCall(int maximumParamTableRowsToComputePerCall)
{
	this->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}

/**
 * Compute the total amount of pixels to be drawn for all the sprites
 *
 */
void SpriteManager::computeTotalPixelsDrawn()
{
	this->totalPixelsDrawn = SpriteManager::getTotalPixelsDrawn(this);
}

/**
 * Render everything
 *
 */
void SpriteManager::prepareAll()
{
	bool isDrawingAllowed = HardwareManager::isDrawingAllowed(HardwareManager::getInstance());

	// Prevent VIP's interrupt from calling render during this process
	HardwareManager::disableRendering(HardwareManager::getInstance());

	// Clean up
	SpriteManager::disposeSprites(this);

	// Must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(this, false);

	// Make sure all textures are written right now
	SpriteManager::writeTextures(this);

	// Sort all sprites' layers
	SpriteManager::sort(this);

	// Render sprites as soon as possible
	SpriteManager::render(this);

	// Sort all sprites' layers again
	// don't remove me, some custom sprites depend on others
	// to have been setup up before
	SpriteManager::sort(this);

	// Render sprites as soon as possible
	SpriteManager::render(this);

	// Defer rendering again
	SpriteManager::deferParamTableEffects(this, true);

	if(isDrawingAllowed)
	{
		// Restore drawing
		HardwareManager::enableRendering(HardwareManager::getInstance());
	}
}

/**
 * Is even frame?
 *
 * @return			Bool
 */
bool SpriteManager::isEvenFrame()
{
	return this->evenFrame;
}

/**
 * Retrieve the total amount of pixels to be drawn for all the sprites
 *
 * @return			Number of pixels
 */
int SpriteManager::getTotalPixelsDrawn()
{
	int totalPixelsToDraw = (_worldAttributesBaseAddress[this->freeLayer].w + 1) * (_worldAttributesBaseAddress[this->freeLayer].h + 1);

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->visible && sprite->positioned && !sprite->hidden)
		{
			totalPixelsToDraw += Sprite::getTotalPixels(sprite);
		}
	}

	return totalPixelsToDraw;
}

/**
 * Print manager's status
 *
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 * @param resumed	If true prints info about all the Sprites in the list
 */
void SpriteManager::print(int x, int y, bool resumed)
{
	SpriteManager::computeTotalPixelsDrawn(this);

	Printing::text(Printing::getInstance(), "SPRITES USAGE", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Total pixels:                ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->totalPixelsDrawn, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Last free layer:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->freeLayer, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Free layers:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __TOTAL_LAYERS - 1 - VirtualList::getSize(this->sprites), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Sprites count:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->sprites), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Disposed sprites:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->spritesToDispose ? VirtualList::getSize(this->spritesToDispose) : 0, x + 18, y, NULL);

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
		Sprite sprite = Sprite::safeCast(node->data);

		strncpy(spriteClassName, __GET_CLASS_NAME_UNSAFE(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printing::int(Printing::getInstance(), Sprite::getWorldLayer(sprite), auxX, auxY, NULL);
		Printing::text(Printing::getInstance(), ": ", auxX + 2, auxY, NULL);
		Printing::text(Printing::getInstance(), spriteClassName, auxX + 4, auxY, NULL);
//		Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[sprite->worldLayer].head, auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, 4, NULL);
//		Printing::int(Printing::getInstance(), Sprite::getTotalPixels(sprite), auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, NULL);

		if((__SCREEN_HEIGHT_IN_CHARS) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}
}

/**
 * Print the manager's status
 *
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
void SpriteManager::printObjectSpriteContainersStatus(int x, int y)
{
	Printing::text(Printing::getInstance(), "OBJECTS USAGE", x, y++, NULL);
	int totalUsedObjects = 0;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; node; node = node->next)
	{
		totalUsedObjects += ObjectSpriteContainer::getTotalUsedObjects(ObjectSpriteContainer::safeCast(node->data));
	}

	Printing::text(Printing::getInstance(), "Total used objects: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), totalUsedObjects, x + 20, y, NULL);
}
