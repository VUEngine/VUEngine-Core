/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <SpriteManager.h>
#include <Mem.h>
#include <VUEngine.h>
#include <VIPManager.h>
#include <ParamTableManager.h>
#include <CharSetManager.h>
#include <BgmapTextureManager.h>
#include <ObjectTextureManager.h>
#include <Camera.h>
#include <TimerManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_SPRITE_CLASS_NAME_SIZE			14

#ifdef __SHOW_SPRITES_PROFILING
int32 _writtenTiles = 0;
int32 _writtenTextureTiles = 0;
int32 _writtenObjectTiles = 0;
#endif


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
	this->specialSprites = NULL;

	this->texturesMaximumRowsToWrite = -1;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->waitToWriteSpriteTextures = 0;
	this->lockSpritesLists = false;
	this->evenFrame = __TRANSPARENCY_EVEN;

	this->printing = NULL;
	this->paramTableManager = NULL;
	this->charSetManager = NULL;
	this->bgmapTextureManager = NULL;
	this->objectTextureManager = NULL;

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
		VirtualList::deleteData(this->objectSpriteContainers);
		delete this->objectSpriteContainers;
		this->objectSpriteContainers = NULL;
	}

	if(!isDeleted(this->sprites))
	{
		VirtualList::deleteData(this->sprites);
		delete this->sprites;
		this->sprites = NULL;
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
	HardwareManager::suspendInterrupts();

	this->printing = Printing::getInstance();
	this->paramTableManager = ParamTableManager::getInstance();
	this->charSetManager = CharSetManager::getInstance();
	this->bgmapTextureManager = BgmapTextureManager::getInstance();
	this->paramTableManager = ParamTableManager::getInstance();
	this->objectTextureManager = ObjectTextureManager::getInstance();

	Printing::reset(this->printing);
	CharSetManager::reset(this->charSetManager);
	BgmapTextureManager::reset(this->bgmapTextureManager);
	ParamTableManager::reset(this->paramTableManager);

	this->lockSpritesLists = true;

	SpriteManager::cleanUp(this);
	ObjectSpriteContainer::reset();

	int32 i = 0;
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
	this->specialSprites = new VirtualList();

	this->freeLayer = __TOTAL_LAYERS - 1;

	this->texturesMaximumRowsToWrite = -1;
	this->waitToWriteSpriteTextures = 0;

	SpriteManager::stopRendering(this);

	Printing::setupSprite(this->printing);

	this->lockSpritesLists = false;
	this->evenFrame = __TRANSPARENCY_EVEN;

	HardwareManager::resumeInterrupts();
}

/**
 * Setup object sprite containers
 *
 * @param size			Array with the number of OBJECTs per container
 * @param z				Z coordinate of each container
 */
void SpriteManager::setupObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS])
{
#ifndef __RELEASE
	int16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	if(isDeleted(this->objectSpriteContainers) ||  0 < VirtualList::getSize(this->objectSpriteContainers))
	{
		return;
	}
	
	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "SpriteManager::setupObjectSpriteContainers: wrong z");

		if(0 < size[i])
		{
			ObjectSpriteContainer objectSpriteContainer = new ObjectSpriteContainer();
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
 * @param z						Z coordinate
 * @return 						ObjectSpriteContainer instance
 */
ObjectSpriteContainer SpriteManager::getObjectSpriteContainer(fixed_t z)
{
	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;

	NM_ASSERT(!isDeleted(this->objectSpriteContainers), "SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers created");
	NM_ASSERT(0 < VirtualList::getSize(this->objectSpriteContainers), "SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers available");

	if(isDeleted(this->objectSpriteContainers))
	{
		return NULL;
	}

	for(VirtualNode node = this->objectSpriteContainers->head; NULL != node; node = node->next)
	{
		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		if(NULL == suitableObjectSpriteContainer)
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

	NM_ASSERT(suitableObjectSpriteContainer, "SpriteManager::getObjectSpriteContainer: no suitable ObjectSpriteContainers found");

	return suitableObjectSpriteContainer;
}

/**
 * Retrieve the SpriteManager for the given segment
 *
 * @param segment		Spt segment
 * @return 				ObjectSpriteContainer instance
 */
ObjectSpriteContainer SpriteManager::getObjectSpriteContainerBySegment(int32 segment)
{
	ASSERT((unsigned)segment < __TOTAL_OBJECT_SEGMENTS, "SpriteManager::getObjectSpriteContainerBySegment: invalid segment");

	if((unsigned)segment > __TOTAL_OBJECT_SEGMENTS)
	{
		return NULL;
	}

	ObjectSpriteContainer objectSpriteContainer = NULL;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; NULL != node; node = node->next)
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
 * Create sprite
 *
 * @param sprite	Sprite to create
 */
Sprite SpriteManager::createSprite(SpriteSpec* spriteSpec, ListenerObject owner)
{
	ASSERT(spriteSpec, "SpriteManager::createSprite: null spriteSpec");
	ASSERT(spriteSpec->allocator, "SpriteManager::createSprite: no sprite allocator");

	Sprite sprite = ((Sprite (*)(SpriteSpec*, ListenerObject)) spriteSpec->allocator)((SpriteSpec*)spriteSpec, owner);
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
	NM_ASSERT(!isDeleted(sprite), "SpriteManager::disposeSprite: trying to dispose dead sprite");
	ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::disposeSprite: trying to dispose a non sprite");

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
	while(SpriteManager::sortProgressively(this, false));
}

/**
 * Sort sprite according to its z coordinate
 */
void SpriteManager::doRegisterSprite(Sprite sprite)
{
	for(VirtualNode node = this->sprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::doRegisterSprite: NULL node's data");

		Sprite otherSprite = Sprite::safeCast(node->data);

		NM_ASSERT(otherSprite != sprite, "SpriteManager::doRegisterSprite: sprite already registered");

#ifndef __RELEASE
		if(otherSprite == sprite)
		{
			return;
		}
#endif

		// check if z positions are swapped
		if(otherSprite->position.z + otherSprite->displacement.z > sprite->position.z + sprite->displacement.z)
		{
			VirtualList::insertBefore(this->sprites, node, sprite);
			return;
		}
	}

	VirtualList::pushFront(this->sprites, sprite);
}

/**
 * Deferred sorting sprites according to their z coordinate
 */
bool SpriteManager::sortProgressively(bool deferred)
{
	bool swapped = false;

	for(VirtualNode node = this->sprites->head; NULL != node && NULL != node->next; node = node->next)
	{
		VirtualNode nextNode = node->next;

		NM_ASSERT(!isDeleted(node->data), "SpriteManager::sortProgressively: NULL node's data");
		ASSERT(__GET_CAST(Sprite, nextNode->data), "SpriteManager::sortProgressively: node's data isn't a sprite");

		Sprite sprite = Sprite::safeCast(node->data);

		NM_ASSERT(!isDeleted(nextNode->data), "SpriteManager::sortProgressively: NULL nextNode's data");
		ASSERT(__GET_CAST(Sprite, nextNode->data), "SpriteManager::sortProgressively: NULL nextNode's data cast");

		Sprite nextSprite = Sprite::safeCast(nextNode->data);

		// check if z positions are swapped
		if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
		{
			// swap nodes' data
			node->data = nextSprite;
			nextNode->data = sprite;

			node = nextNode;

			swapped = true;

			if(deferred)
			{
				break;
			}
		}
	}

	if(!swapped && !isDeleted(this->objectSpriteContainers))
	{
		for(VirtualNode node = this->objectSpriteContainers->head; NULL != node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			swapped = swapped || ObjectSpriteContainer::sortProgressively(objectSpriteContainer, deferred);
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
bool SpriteManager::registerSprite(Sprite sprite, bool hasEffects)
{
#ifndef __RELEASE
	static bool registeringSprite = false;

	NM_ASSERT(!registeringSprite, "SpriteManager::registerSprite: already registering a sprite!");

	if(registeringSprite)
	{
		return false;
	}

	registeringSprite = true;

	ASSERT(Sprite::safeCast(sprite), "SpriteManager::registerSprite: adding no sprite");

	ASSERT(!__GET_CAST(ObjectSprite, sprite), "SpriteManager::registerSprite: trying to register an object sprite");

	if(VirtualList::find(this->sprites, sprite))
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::clear(Printing::getInstance());
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME(sprite), 1, 20, NULL);
		NM_ASSERT(false, "SpriteManager::registerSprite: sprite already registered");
	}
#else
	if(VirtualList::find(this->sprites, sprite))
	{
		return true;
	}
#endif

	if(!isDeleted(sprite))
	{
		this->lockSpritesLists = true;

		SpriteManager::doRegisterSprite(this, sprite);

		if(hasEffects)
		{
			VirtualList::pushBack(this->specialSprites, sprite);
		}

		this->lockSpritesLists = false;

#ifndef __RELEASE
		registeringSprite = false;
#endif
		return true;
	}

#ifndef __RELEASE
	registeringSprite = false;
#endif

	NM_ASSERT(sprite, "SpriteManager::registerSprite: null sprite");
	return false;
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

#ifndef __ENABLE_PROFILER
	NM_ASSERT(!isDeleted(VirtualList::find(this->sprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");
#endif

	this->lockSpritesLists = true;

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
	NM_ASSERT(0 <= (int8)this->freeLayer, "SpriteManager::stopRendering: no more layers");

	if(0 <= this->freeLayer)
	{
		_worldAttributesCache[this->freeLayer].head = __WORLD_END;
	}
}

int32 SpriteManager::getNumberOfSprites()
{
	return VirtualList::getSize(this->sprites);
}

/**
 * Write textures to DRAM
 */
void SpriteManager::writeTextures()
{
	CharSetManager::writeCharSets(this->charSetManager);

	int8 texturesMaximumRowsToWrite = this->texturesMaximumRowsToWrite;

	// allow complete texture writing
	this->texturesMaximumRowsToWrite = -1;

	for(VirtualNode node = this->sprites->head; NULL != node; node = node->next)
	{
		Sprite::writeTextures(node->data);
	}

	this->texturesMaximumRowsToWrite = texturesMaximumRowsToWrite;

	CharSetManager::writeCharSets(this->charSetManager);
}

void SpriteManager::applySpecialEffects()
{
	for(VirtualNode node = this->specialSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::writeGraphicsToDRAM: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(__HIDE == sprite->show || !sprite->positioned)
		{
			continue;
		}
		
		Sprite::processEffects(sprite);
	}
}

void SpriteManager::writeWORLDAttributesToDRAM()
{
	Mem::copyWORD((WORD*)(_worldAttributesBaseAddress + this->freeLayer), (WORD*)(_worldAttributesCache + this->freeLayer), sizeof(WorldAttributes) * (__TOTAL_LAYERS - (this->freeLayer)) >> 2);
}

void SpriteManager::writeDRAM()
{
#ifdef __SHOW_SPRITES_PROFILING
	_writtenTiles = 0;
	_writtenTextureTiles = 0;
	_writtenObjectTiles = 0;
#endif

	// Update all graphical data

	// Update CHAR memory
	CharSetManager::writeCharSetsProgressively(this->charSetManager);

	// Update BGMAP memory
	BgmapTextureManager::updateTextures(this->bgmapTextureManager);

	// Update param tables
	SpriteManager::applySpecialEffects(this);

	// Finally, write OBJ and WORLD attributes to DRAM
	if(NULL != this->objectSpriteContainers->head)
	{
		ObjectTextureManager::updateTextures(this->objectTextureManager);
		ObjectSpriteContainer::writeDRAM();
	}

	// Finally, write OBJ and WORLD attributes to DRAM
	SpriteManager::writeWORLDAttributesToDRAM(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInSpecialMode(VUEngine::getInstance()))
	{
		static int32 counter = __TARGET_FPS / 5;

		if(0 >= --counter)
		{
			counter = __TARGET_FPS / 10;
			SpriteManager::print(this, 1, 15, true);
		}
	}
#endif
}

/**
 * Write WORLD data to DRAM
 */
void SpriteManager::render()
{
	if(!this->lockSpritesLists)
	{
		SpriteManager::sortProgressively(this, true);
	}

	ObjectSpriteContainer::prepareForRendering();

	ParamTableManager::defragmentProgressively(this->paramTableManager);

	// switch between even and odd frame
	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	this->freeLayer = __TOTAL_LAYERS - 1;

	for(VirtualNode node = this->sprites->tail; NULL != node && 0 < this->freeLayer; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::render: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		// Saves on method calls quite a bit when there are lots of
		// sprites. Don't remove.
		if(__HIDE == sprite->show || !sprite->positioned || (sprite->transparent & this->evenFrame))
		{
			sprite->index = __NO_RENDER_INDEX;
			continue;
		}

		if(Sprite::render(sprite, this->freeLayer, this->evenFrame) == this->freeLayer)
		{
			this->freeLayer--;
		}
	}

	NM_ASSERT(0 <= this->freeLayer, "SpriteManager::render: more sprites than WORLDs");

	ObjectSpriteContainer::finishRendering();

	SpriteManager::stopRendering(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInSpecialMode(VUEngine::getInstance()))
	{
		SpriteManager::computeTotalPixelsDrawn(this);
	}
#endif
}

/**
 * Invalidate render flag on all sprites
 *
 */
void SpriteManager::forceRendering()
{
	for(VirtualNode node = this->sprites->tail; NULL != node; node = node->previous)
	{
		Sprite::invalidateRenderFlag(Sprite::safeCast(node->data));
	}
}

/**
 * Retrieve the next free WORLD layer
 *
 * @return			Free WORLD layer
 */
int8 SpriteManager::getFreeLayer()
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
	for(VirtualNode node = this->sprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::hideSprites: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::forceShow(spareSprite);
			continue;
		}

		Sprite::hideForDebug(sprite);
	}

	if(!isDeleted(spareSprite) && !VirtualList::find(this->sprites, spareSprite))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; NULL != node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			ObjectSpriteContainer::hideSprites(objectSpriteContainer, ObjectSprite::safeCast(spareSprite));
		}
	}

	if(hidePrinting)
	{
		Printing::hide(this->printing);
	}
	else
	{
		Printing::show(this->printing);
	}
}

/**
 * Show all WORLD layers
 */
void SpriteManager::showSprites(Sprite spareSprite, bool showPrinting)
{
	for(VirtualNode node = this->sprites->tail; NULL != node; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::showSprites: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::hideForDebug(spareSprite);
			continue;
		}

		Sprite::forceShow(sprite);

		Sprite::setPosition(sprite, &sprite->position);

		_worldAttributesBaseAddress[sprite->index].head &= ~__WORLD_END;
	}

	if(!isDeleted(spareSprite) && !VirtualList::find(this->sprites, spareSprite))
	{
		VirtualNode node = this->objectSpriteContainers->head;

		for(; NULL != node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			ObjectSpriteContainer::showSprites(objectSpriteContainer, ObjectSprite::safeCast(spareSprite));
		}
	}

	if(showPrinting)
	{
		Printing::show(this->printing);
	}
	else
	{
		Printing::hide(this->printing);
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
Sprite SpriteManager::getSpriteAtPosition(int16 position)
{
	if(0 > position || position >= VirtualList::getSize(this->sprites))
	{
		return NULL;
	}

	int32 counter = 0;

	for(VirtualNode node = this->sprites->head; NULL != node; node = node->next, counter++)
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
int16 SpriteManager::getSpritePosition(Sprite sprite)
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
int8 SpriteManager::getTexturesMaximumRowsToWrite()
{
	return this->texturesMaximumRowsToWrite;
}

/**
 * Set the maximum number of texture rows allowed to be written on each render cycle
 *
 * @param texturesMaximumRowsToWrite		Number of texture rows allowed to be written
 */
void SpriteManager::setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite)
{
	this->texturesMaximumRowsToWrite = 2 > (int8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
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
int32 SpriteManager::getMaximumParamTableRowsToComputePerCall()
{
	return this->deferParamTableEffects ? this->maximumParamTableRowsToComputePerCall : -1;
}

/**
 * Set the maximum number of affine transformation rows to compute per render cycle
 *
 * @param maximumParamTableRowsToComputePerCall		Number of affine transformation rows to compute per render cycle
 */
void SpriteManager::setMaximumParamTableRowsToComputePerCall(int32 maximumParamTableRowsToComputePerCall)
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
int32 SpriteManager::getTotalPixelsDrawn()
{
	int32 totalPixelsToDraw = (_worldAttributesBaseAddress[this->freeLayer].w + 1) * (_worldAttributesBaseAddress[this->freeLayer].h + 1);

	for(VirtualNode node = this->sprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::getTotalPixelsDrawn: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->positioned && __SHOW == sprite->show)
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
void SpriteManager::print(int32 x, int32 y, bool resumed)
{
	Printing::setWorldCoordinates(this->printing, 0, 0, Printing::getSpritePosition(this->printing).z, 0);
#ifndef __SHOW_SPRITES_PROFILING
	SpriteManager::computeTotalPixelsDrawn(this);
#endif

	Printing::text(this->printing, "SPRITES USAGE", x, y++, NULL);
	Printing::text(this->printing, "Total pixels:                ", x, ++y, NULL);
	Printing::int32(this->printing, this->totalPixelsDrawn, x + 22, y, NULL);
	Printing::text(this->printing, "Used layers:                ", x, ++y, NULL);
	Printing::int32(this->printing, __TOTAL_LAYERS - this->freeLayer, x + 22, y, NULL);
	Printing::text(this->printing, "Sprites count:              ", x, ++y, NULL);
	Printing::int32(this->printing, VirtualList::getSize(this->sprites), x + 22, y, NULL);
#ifdef __SHOW_SPRITES_PROFILING
	Printing::text(this->printing, "Written chars:              ", x, ++y, NULL);
	Printing::int32(this->printing, _writtenTiles, x + 22, y, NULL);
	Printing::text(this->printing, "Written texture tiles:              ", x, ++y, NULL);
	Printing::int32(this->printing, _writtenTextureTiles, x + 22, y, NULL);
	Printing::text(this->printing, "Written object tiles:              ", x, ++y, NULL);
	Printing::int32(this->printing, _writtenObjectTiles, x + 22, y, NULL);
#endif

	if(resumed)
	{
		return;
	}

	int32 auxY = y + 2;
	int32 auxX = x;

	int32 counter = __TOTAL_LAYERS - 1;
	
	for(VirtualNode node = this->sprites->tail; NULL != node; node = node->previous, counter--)
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = Sprite::safeCast(node->data);

		strncpy(spriteClassName, __GET_CLASS_NAME_UNSAFE(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printing::int32(this->printing, counter, auxX, auxY, NULL);
		Printing::text(this->printing, ": ", auxX + 2, auxY, NULL);
		Printing::text(this->printing, spriteClassName, auxX + 4, auxY, NULL);
//		Printing::int32(this->printing, sprite->position.z + sprite->displacement.z, auxX + 2, auxY, NULL);
//		Printing::hex(this->printing, _worldAttributesBaseAddress[sprite->index].head, auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, 4, NULL);
//		Printing::int32(this->printing, Sprite::getTotalPixels(sprite), auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, NULL);

		++auxY;
		if(__TOTAL_LAYERS / 2 == counter)
//		if((__SCREEN_HEIGHT_IN_CHARS) - 2 <= ++auxY)
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
void SpriteManager::printObjectSpriteContainersStatus(int32 x, int32 y)
{
	Printing::text(this->printing, "OBJECTS USAGE", x, y++, NULL);
	int32 totalUsedObjects = 0;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; NULL != node; node = node->next)
	{
		totalUsedObjects += ObjectSpriteContainer::getTotalUsedObjects(ObjectSpriteContainer::safeCast(node->data));
	}

	Printing::text(this->printing, "Total used objects: ", x, ++y, NULL);
	Printing::int32(this->printing, totalUsedObjects, x + 20, y, NULL);
}