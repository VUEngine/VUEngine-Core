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
#include <Mem.h>
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
	this->objectSpriteContainers = NULL;
	this->texturesToUpdate = NULL;
	this->specialSprites = NULL;

	this->texturesMaximumRowsToWrite = -1;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->waitToWriteSpriteTextures = 0;
	this->lockSpritesLists = false;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->lockTextureList = false;

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
	if(!isDeleted(this->objectSpriteContainers))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->objectSpriteContainers;
		this->objectSpriteContainers = NULL;
	}

	if(!isDeleted(this->sprites))
	{
		NM_ASSERT(0 == VirtualList::getSize(this->sprites), "SpriteManager::cleanUp: sprites list not empty");

		VirtualList sprites = new VirtualList();
		VirtualList::copy(sprites, this->sprites);

		VirtualNode node = sprites->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete sprites;

		delete this->sprites;
		this->sprites = NULL;
	}

	if(!isDeleted(this->texturesToUpdate))
	{
		delete this->texturesToUpdate;
		this->texturesToUpdate = NULL;
	}

	if(!isDeleted(this->specialSprites))
	{
		delete this->specialSprites;
		this->specialSprites = NULL;
	}
}

/**
 * Reset manager's state
 */
void SpriteManager::reset()
{
	Printing::reset(Printing::getInstance());

	this->lockSpritesLists = true;

	SpriteManager::cleanUp(this);

	int i = 0;
	// clean OBJ memory
	for(; i < __AVAILABLE_CHAR_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	this->sprites = new VirtualList();
	this->objectSpriteContainers = new VirtualList();
	this->texturesToUpdate = new VirtualList();
	this->specialSprites = new VirtualList();

	this->freeLayer = __TOTAL_LAYERS - 1;

	this->texturesMaximumRowsToWrite = -1;
	this->waitToWriteSpriteTextures = 0;

	SpriteManager::stopRendering(this);

	Printing::setupSprite(Printing::getInstance());

	this->lockSpritesLists = false;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->lockTextureList = false;
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

		if(0 < size[i])
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

	Sprite sprite = ((Sprite (*)(SpriteSpec*, Object)) spriteSpec->allocator)((SpriteSpec*)spriteSpec, owner);
	ASSERT(!isDeleted(sprite), "SpriteManager::createSprite: failed creating sprite");

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
	NM_ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::disposeSprite: trying to dispose a non sprite");

	if(isDeleted(sprite))
	{
		return;
	}

	Sprite::hide(sprite);
	delete sprite;
}

/**
 * Sort sprites according to their z coordinate
 */
void SpriteManager::sort()
{
	while(SpriteManager::sortProgressively(this));
}

/**
 * Sort sprite according to its z coordinate
 */
void SpriteManager::doRegisterSprite(Sprite sprite)
{
	for(VirtualNode node = this->sprites->head; node; node = node->next)
	{
		Sprite otherSprite = Sprite::safeCast(node->data);

		if(otherSprite == sprite)
		{
			return;
		}

		if(!otherSprite->positioned)
		{
			continue;
		}

		// check if z positions are swapped
		if(otherSprite->position.z + otherSprite->displacement.z > sprite->position.z + sprite->displacement.z)
		{
			VirtualList::insertBefore(this->sprites, node, sprite);

			return;
		}
	}

	VirtualList::pushFront(this->sprites, sprite);
}

// check if any entity must be assigned another world layer
/**
 * Deferred sorting sprites according to their z coordinate
 */
bool SpriteManager::sortProgressively()
{
	bool swapped = false;

	VirtualNode node = this->sprites->head;

	for(; node; node = node->next)
	{
		VirtualNode nextNode = node->next;

		if(nextNode)
		{
			Sprite sprite = Sprite::safeCast(node->data);

			if(!sprite->positioned)
			{
				continue;
			}

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

	if(!isDeleted(this->objectSpriteContainers))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			ObjectSpriteContainer::sortProgressively(objectSpriteContainer);
		}
	}

	return swapped;
}

/**
 * Register a Sprite and assign a WORLD layer to it
 *
 * @private
 * @param sprite	Sprite to assign the WORLD layer
 * @param hasEffects	Flag to signal that the sprite has special effects applied to it
 */
void SpriteManager::registerSprite(Sprite sprite, bool hasEffects)
{
	ASSERT(Sprite::safeCast(sprite), "SpriteManager::registerSprite: adding no sprite");

	ASSERT(__TOTAL_LAYERS > VirtualList::getSize(this->sprites), "SpriteManager::registerSprite: exceding available WORLDS");
	NM_ASSERT(!VirtualList::find(this->sprites, sprite), "SpriteManager::registerSprite: sprite already registered");
	NM_ASSERT(!__GET_CAST(ObjectSprite, sprite), "SpriteManager::registerSprite: trying to register an object sprite");

	this->lockSpritesLists = true;

	SpriteManager::doRegisterSprite(this, sprite);

	if(hasEffects)
	{
		VirtualList::pushBack(this->specialSprites, sprite);
	}

	this->lockSpritesLists = false;
}

/**
 * Remove a registered Sprite and get back the WORLD layer previously assigned to it
 *
 * @private
 * @param sprite		Sprite to assign the WORLD layer
 * @param hasEffects	Flag to signal that the sprite has special effects applied to it
 */
void SpriteManager::unregisterSprite(Sprite sprite, bool hasEffects __attribute__((unused)))
{
	ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

	this->lockSpritesLists = true;

	NM_ASSERT(!isDeleted(VirtualList::find(this->sprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");

	VirtualList::removeElement(this->sprites, sprite);

#ifdef __RELEASE
	if(hasEffects)
#endif
	{
		VirtualList::removeElement(this->specialSprites, sprite);
	}

	this->lockSpritesLists = false;
}

/**
 * End drawing
 */
void SpriteManager::stopRendering()
{
	NM_ASSERT(0 <= (s8)this->freeLayer, "SpriteManager::stopRendering: no more layers");

	if(0 <= this->freeLayer)
	{
		_worldAttributesBaseAddress[this->freeLayer].head = __WORLD_END;
	}
}

int SpriteManager::getNumberOfSprites()
{
	return VirtualList::getSize(this->sprites);
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

	CharSetManager::writeCharSets(CharSetManager::getInstance());
}

void SpriteManager::updateTexture(Texture texture)
{
	if(!isDeleted(texture) && !VirtualList::find(this->texturesToUpdate, texture))
	{
		this->lockTextureList = true;
		VirtualList::pushBack(this->texturesToUpdate, texture);
		this->lockTextureList = false;
	}
}

void SpriteManager::writeGraphicsToDRAM()
{
	CharSetManager::writeCharSetsProgressively(CharSetManager::getInstance());

	if(!this->lockTextureList)
	{
		for(VirtualNode node = this->texturesToUpdate->head; node;)
		{
			Texture texture = Texture::safeCast(node->data);

			VirtualNode auxNode = node;
			node = node->next;

			if(!isDeleted(texture))
			{
				if(kTextureWritten != texture->status && !Texture::update(texture))
				{
					continue;
				}
			}

			VirtualList::removeNode(this->texturesToUpdate, auxNode);
		}
	}

	for(VirtualNode node = this->specialSprites->head; node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->hidden | !sprite->positioned)
		{
			continue;
		}
		
		Sprite::processEffects(sprite);
	}
}

void SpriteManager::writeDRAM()
{
	SpriteManager::writeGraphicsToDRAM(this);

	Mem::copyWORD((WORD*)(_worldAttributesBaseAddress + this->freeLayer + 1), (WORD*)(_worldAttributesCache + this->freeLayer + 1), sizeof(WorldAttributes) * (__TOTAL_LAYERS - (this->freeLayer + 1)) >> 2);

	for(VirtualNode node = this->objectSpriteContainers->head; node; node = node->next)
	{
		ObjectSpriteContainer::writeDRAM(ObjectSpriteContainer::safeCast(node->data));
	}

	SpriteManager::stopRendering(this);
}

/**
 * Write WORLD data to DRAM
 */
void SpriteManager::render()
{
	ParamTableManager::defragmentProgressively(ParamTableManager::getInstance());

	SpriteManager::sortProgressively(this);

	// switch between even and odd frame
	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	this->freeLayer = __TOTAL_LAYERS - 1;

	for(VirtualNode node = this->sprites->tail; node && 0 < this->freeLayer; node = node->previous)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		// Saves on method calls quite a bit when there are lots of
		// sprites. Don't remove.
		if(sprite->hidden | !sprite->positioned)
		{
			continue;
		}

		if(sprite->transparent & this->evenFrame)
		{
			continue;
		}

		if(this->freeLayer == Sprite::render(sprite, this->freeLayer, this->evenFrame))
		{
			this->freeLayer--;
		}
	}

	NM_ASSERT(0 <= this->freeLayer, "SpriteManager::render: more sprites than WORLDs");

#ifdef __SHOW_SPRITES_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		SpriteManager::computeTotalPixelsDrawn(this);
	}
#endif

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
s8 SpriteManager::getFreeLayer()
{
	return this->freeLayer;
}

/**
 * Show the Sprite in the given WORLD layer and hide the rest
 *
 * @param layer		WORLD layer to show
 */
#ifdef __TOOLS
void SpriteManager::hideSprites(Sprite spareSprite, bool hidePrinting)
{
	for(VirtualNode node = this->sprites->head; node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::showForDebug(spareSprite);
			continue;
		}

		Sprite::hideForDebug(sprite);
	}

	if(!isDeleted(spareSprite) && !VirtualList::find(this->sprites, spareSprite))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			ObjectSpriteContainer::hideSprites(objectSpriteContainer, ObjectSprite::safeCast(spareSprite));
		}
	}

	if(hidePrinting)
	{
		Printing::hide(Printing::getInstance());
	}
	else
	{
		Printing::show(Printing::getInstance());
	}
}

/**
 * Show all WORLD layers
 */
void SpriteManager::showSprites(Sprite spareSprite, bool showPrinting)
{
	for(VirtualNode node = this->sprites->tail; node; node = node->previous)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::hideForDebug(spareSprite);
			continue;
		}

		Sprite::showForDebug(sprite);

		Sprite::setPosition(sprite, &sprite->position);

		_worldAttributesBaseAddress[sprite->index].head &= ~__WORLD_END;
	}

	if(!isDeleted(spareSprite) && !VirtualList::find(this->sprites, spareSprite))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			ObjectSpriteContainer::showSprites(objectSpriteContainer, ObjectSprite::safeCast(spareSprite));
		}
	}

	if(showPrinting)
	{
		Printing::show(Printing::getInstance());
	}
	else
	{
		Printing::hide(Printing::getInstance());
	}

	SpriteManager::stopRendering(this);
}
#endif

/**
 * Retrieve the Sprite assigned to the given WORLD
 *
 * @param layer		WORLD layer to show
 * @return			Sprite with the given WORLD layer
 */
Sprite SpriteManager::getSpriteAtPosition(s16 position)
{
	if(0 > position || position >= VirtualList::getSize(this->sprites))
	{
		return NULL;
	}

	VirtualNode node = this->sprites->head;

	int counter = 0;

	for(; node; node = node->next, counter++)
	{
		if(counter == position)
		{
			return Sprite::safeCast(node->data);
		}
	}

	return NULL;
}

/**
 * Retrieve the Sprite assigned to the given WORLD
 *
 * @param layer		WORLD layer to show
 * @return			Sprite with the given WORLD layer
 */
s16 SpriteManager::getSpritePosition(Sprite sprite)
{
	if(isDeleted(sprite) || !VirtualList::find(this->sprites, sprite))
	{
		return -1;
	}

	return (__TOTAL_LAYERS - VirtualList::getSize(this->sprites)) + VirtualList::getDataPosition(this->sprites, sprite);
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

	// Must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(this, false);

	// Make sure all textures are written right now
	SpriteManager::writeTextures(this);

	// Sort all sprites' layers
	SpriteManager::sort(this);

	// Render and draw sprites as soon as possible
	SpriteManager::renderAndDraw(this);

	// Sort all sprites' layers again
	// don't remove me, some custom sprites depend on others
	// to have been setup up before
	SpriteManager::sort(this);

	// Render and draw sprites as soon as possible again
	SpriteManager::renderAndDraw(this);

	// Defer rendering again
	SpriteManager::deferParamTableEffects(this, true);

	if(isDrawingAllowed)
	{
		// Restore drawing
		HardwareManager::enableRendering(HardwareManager::getInstance());
		while(VIPManager::isRenderingPending(VIPManager::getInstance()));
	}
}

void SpriteManager::renderAndDraw()
{
	SpriteManager::render(this);

	// Write render data
	SpriteManager::writeDRAM(this);
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
	Printing::setWorldCoordinates(Printing::getInstance(), 0, 0, 0, 0);
	SpriteManager::computeTotalPixelsDrawn(this);

	Printing::text(Printing::getInstance(), "SPRITES USAGE", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Total pixels:                ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->totalPixelsDrawn, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Used layers:                ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), __TOTAL_LAYERS - this->freeLayer, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Sprites count:              ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->sprites), x + 18, y, NULL);

	if(resumed)
	{
		return;
	}

	int auxY = y + 2;
	int auxX = x;

	int counter = __TOTAL_LAYERS - 1;
	VirtualNode node = this->sprites->tail;

	for(; node; node = node->previous, counter--)
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = Sprite::safeCast(node->data);

		strncpy(spriteClassName, __GET_CLASS_NAME_UNSAFE(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printing::int(Printing::getInstance(), counter, auxX, auxY, NULL);
		Printing::text(Printing::getInstance(), ": ", auxX + 2, auxY, NULL);
		Printing::text(Printing::getInstance(), spriteClassName, auxX + 4, auxY, NULL);
//		Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[sprite->index].head, auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, 4, NULL);
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
