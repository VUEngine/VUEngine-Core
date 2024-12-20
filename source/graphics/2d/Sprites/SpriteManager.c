/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <string.h>

#include <BgmapTextureManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectTextureManager.h>
#include <ParamTableManager.h>
#include <Printing.h>
#include <Sprite.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "SpriteManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Sprite;
friend class ObjectSpriteContainer;
friend class Texture;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __MAX_SPRITE_CLASS_NAME_SIZE			14


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

#ifdef __SHOW_SPRITES_PROFILING
int32 _renderedSprites = 0; 
int32 _writtenTiles = 0;
int32 _writtenTextureTiles = 0;
int32 _writtenObjectTiles = 0;
#endif


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
Sprite SpriteManager::createComponent(SpatialObject owner, const SpriteSpec* spriteSpec)
{
	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Base::createComponent(this, owner, (ComponentSpec*)spriteSpec);

	return SpriteManager::createSprite(this, owner, spriteSpec);
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::destroyComponent(SpatialObject owner, Sprite sprite) 
{
	if(isDeleted(sprite))
	{
		return;
	}

	Base::destroyComponent(this, owner, Component::safeCast(sprite));
	
	SpriteManager::destroySprite(this, sprite);
}
//---------------------------------------------------------------------------------------------------------
bool SpriteManager::isAnyVisible(SpatialObject owner)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(owner == sprite->owner && Sprite::isVisible(sprite))
		{
			return true;
		}
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::reset()
{
	HardwareManager::suspendInterrupts();

	Texture::reset();
	Printing::reset(this->printing);
	CharSetManager::reset(this->charSetManager);
	BgmapTextureManager::reset(this->bgmapTextureManager);
	ParamTableManager::reset(this->paramTableManager);
	
	SpriteManager::cleanUp(this);
	ObjectSpriteContainer::reset();

	int32 i = 0;
	// clean OBJ memory
	for(; i < __TOTAL_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	this->animationsClock = NULL;
	this->bgmapSprites = new VirtualList();
	this->objectSpriteContainers = new VirtualList();
	this->specialSprites = new VirtualList();

	this->freeLayer = __TOTAL_LAYERS - 1;
	this->deferTextureUpdating = false;
	this->texturesMaximumRowsToWrite = -1;
	this->sortingSpriteNode = NULL;
	this->completeSort = true;

	SpriteManager::stopRendering(this);

	this->evenFrame = __TRANSPARENCY_EVEN;

	HardwareManager::resumeInterrupts();
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::setAnimationsClock(Clock clock)
{
	this->animationsClock = clock;
}
//---------------------------------------------------------------------------------------------------------
Sprite SpriteManager::createSprite(SpatialObject owner, const SpriteSpec* spriteSpec)
{
	NM_ASSERT(NULL != spriteSpec, "SpriteManager::createSprite: null spriteSpec");

	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Sprite sprite = ((Sprite (*)(SpatialObject, const SpriteSpec*)) ((ComponentSpec*)spriteSpec)->allocator)(owner, spriteSpec);
	ASSERT(!isDeleted(sprite), "SpriteManager::createSprite: failed creating sprite");

	VirtualList::pushBack(this->components, sprite);

	Sprite::render(sprite, __NO_RENDER_INDEX, false);
	Sprite::registerWithManager(sprite);

	return sprite;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::destroySprite(Sprite sprite)
{
	NM_ASSERT(!isDeleted(sprite), "SpriteManager::destroySprite: trying to dispose dead sprite");
	NM_ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::destroySprite: trying to dispose a non sprite");

	if(isDeleted(sprite))
	{
		return;
	}

	VirtualList::removeData(this->components, sprite);

	Sprite::hide(sprite);
	Sprite::unregisterWithManager(sprite);

	delete sprite;
}
//---------------------------------------------------------------------------------------------------------
bool SpriteManager::registerSprite(Sprite sprite)
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

	if(VirtualList::find(this->bgmapSprites, sprite))
	{
		Printing::setDebugMode(Printing::getInstance());
		Printing::clear(Printing::getInstance());
		Printing::text(Printing::getInstance(), __GET_CLASS_NAME(sprite), 1, 20, NULL);
		NM_ASSERT(false, "SpriteManager::registerSprite: sprite already registered");
	}
#endif

	if(!isDeleted(sprite))
	{
		if(SpriteManager::doRegisterSprite(this, sprite) && Sprite::hasSpecialEffects(sprite))
		{
			VirtualList::pushBack(this->specialSprites, sprite);
		}

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
//---------------------------------------------------------------------------------------------------------
void SpriteManager::unregisterSprite(Sprite sprite)
{
	NM_ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

#ifndef __ENABLE_PROFILER
	NM_ASSERT(!isDeleted(VirtualList::find(this->bgmapSprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");
#endif

	this->sortingSpriteNode = NULL;

	VirtualList::removeData(this->bgmapSprites, sprite);

#ifdef __RELEASE
	if(Sprite::hasSpecialEffects(sprite))
#endif
	{
		VirtualList::removeData(this->specialSprites, sprite);
	}
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::setupObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS])
{
#ifndef __RELEASE
	int16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	if(isDeleted(this->objectSpriteContainers) ||  0 < VirtualList::getCount(this->objectSpriteContainers))
	{
		return;
	}
	
	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "SpriteManager::setupObjectSpriteContainers: wrong z");

		if(0 < size[i])
		{
			ObjectSpriteContainer objectSpriteContainer = new ObjectSpriteContainer();
			ObjectSpriteContainer::registerWithManager(objectSpriteContainer);
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
//---------------------------------------------------------------------------------------------------------
void SpriteManager::setMaximumParamTableRowsToComputePerCall(int32 maximumParamTableRowsToComputePerCall)
{
	this->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}
//---------------------------------------------------------------------------------------------------------
int32 SpriteManager::getMaximumParamTableRowsToComputePerCall()
{
	return this->deferParamTableEffects ? this->maximumParamTableRowsToComputePerCall : -1;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite)
{
	this->texturesMaximumRowsToWrite = 2 > (int8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}
//---------------------------------------------------------------------------------------------------------
int8 SpriteManager::getTexturesMaximumRowsToWrite()
{
	return this->texturesMaximumRowsToWrite;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::deferTextureUpdating(bool deferTextureUpdating)
{
	this->deferTextureUpdating = deferTextureUpdating;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::deferParamTableEffects(bool deferParamTableEffects)
{
	this->deferParamTableEffects = deferParamTableEffects;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::sortSprites()
{
	while(SpriteManager::sortProgressively(this, true));

	this->completeSort = true;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::prepareAll()
{
	bool deferTextureUpdating = this->deferTextureUpdating;

	// Prevent VIP's interrupt from calling render during this process
	VIPManager::suspendDrawing(VIPManager::getInstance());

	// Must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(this, false);

	// Make sure all textures are written right now
	SpriteManager::writeTextures(this);

	// Sort all sprites' layers
	SpriteManager::sortSprites(this);

	// Render and draw sprites as soon as possible
	SpriteManager::renderAndDraw(this);

	// Sort all sprites' layers again
	// don't remove me, some custom sprites depend on others
	// to have been setup up before
	SpriteManager::sortSprites(this);

	// Render and draw sprites as soon as possible again
	SpriteManager::renderAndDraw(this);

	// Defer rendering again
	SpriteManager::deferParamTableEffects(this, true);

	// Restore drawing
	VIPManager::resumeDrawing(VIPManager::getInstance());

	this->deferTextureUpdating = deferTextureUpdating;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::render()
{
#ifdef __SHOW_SPRITES_PROFILING
	_renderedSprites = 0;
#endif

	this->completeSort = SpriteManager::sortProgressively(this, this->completeSort);

	ParamTableManager::defragment(this->paramTableManager, true);

	// switch between even and odd frame
	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	this->freeLayer = __TOTAL_LAYERS - 1;

	bool updateAnimations = true;
	
	if(!isDeleted(this->animationsClock))
	{
		updateAnimations = !Clock::isPaused(this->animationsClock);
	}

	for(VirtualNode node = this->bgmapSprites->tail; NULL != node && 0 < this->freeLayer; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::render: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		// Saves on method calls quite a bit when there are lots of
		// sprites. Don't remove.
		if(__HIDE == sprite->show || (sprite->transparency & this->evenFrame))
		{
			sprite->index = __NO_RENDER_INDEX;
			continue;
		}

		if(Sprite::render(sprite, this->freeLayer, updateAnimations) == this->freeLayer)
		{
			this->freeLayer--;
		}
	}

	NM_ASSERT(0 <= this->freeLayer, "SpriteManager::render: more sprites than WORLDs");

	ObjectSpriteContainer::prepareForRendering();

	for(VirtualNode node = this->objectSpriteContainers->head; NULL != node; node = node->next)
	{
		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		ObjectSpriteContainer::renderSprites(objectSpriteContainer, this->evenFrame, updateAnimations);
	}

	ObjectSpriteContainer::finishRendering();

	SpriteManager::stopRendering(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInToolState(VUEngine::getInstance()))
	{
		SpriteManager::computeTotalPixelsDrawn(this);
	}
#endif
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::forceRendering()
{
	for(VirtualNode node = this->components->tail; NULL != node; node = node->previous)
	{
		Sprite::invalidateRendering(Sprite::safeCast(node->data));
	}
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::renderAndDraw()
{
	SpriteManager::render(this);

	// Write render data
	SpriteManager::writeDRAM(this);
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::writeDRAM()
{
#ifdef __SHOW_SPRITES_PROFILING
	_writtenTiles = 0;
	_writtenTextureTiles = 0;
	_writtenObjectTiles = 0;
#endif

	// Update all graphical data

	// Update CHAR memory
	CharSetManager::defragment(this->charSetManager, true);

	// Update DRAM memory
	Texture::updateTextures(this->texturesMaximumRowsToWrite, this->deferTextureUpdating);

	// Update param tables
	SpriteManager::applySpecialEffects(this);

	// Finally, write OBJ and WORLD attributes to DRAM
	ObjectSpriteContainer::writeDRAM();

	// Finally, write OBJ and WORLD attributes to DRAM
	SpriteManager::writeWORLDAttributesToDRAM(this);

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInToolState(VUEngine::getInstance()))
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
//---------------------------------------------------------------------------------------------------------
void SpriteManager::writeTextures()
{
	NM_ASSERT(!isDeleted(this->charSetManager), "SpriteManager::writeTextures: invalid charset manager");

	if(isDeleted(this->charSetManager))
	{
		return;
	}

	CharSetManager::writeCharSets(this->charSetManager);

	Texture::updateTextures(-1, false);

	CharSetManager::writeCharSets(this->charSetManager);
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::showAllSprites(Sprite spareSprite, bool showPrinting)
{
	for(VirtualNode node = this->components->tail; NULL != node; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::showSprites: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::forceHide(spareSprite);
			continue;
		}

		Sprite::forceShow(sprite);

		Sprite::setPosition(sprite, &sprite->position);

		_worldAttributesBaseAddress[sprite->index].head &= ~__WORLD_END;
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
//---------------------------------------------------------------------------------------------------------
void SpriteManager::hideAllSprites(Sprite spareSprite, bool hidePrinting)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::hideAllSprites: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite == spareSprite)
		{
			Sprite::forceShow(spareSprite);
			continue;
		}

		Sprite::forceHide(sprite);
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
//---------------------------------------------------------------------------------------------------------
void SpriteManager::computeTotalPixelsDrawn()
{
	this->totalPixelsDrawn = SpriteManager::getTotalPixelsDrawn(this);
}
//---------------------------------------------------------------------------------------------------------
int8 SpriteManager::getFreeLayer()
{
	return this->freeLayer;
}
//---------------------------------------------------------------------------------------------------------
int32 SpriteManager::getNumberOfSprites()
{
	return VirtualList::getCount(this->bgmapSprites);
}
//---------------------------------------------------------------------------------------------------------
Sprite SpriteManager::getSpriteAtIndex(int16 position)
{
	if(0 > position || position >= VirtualList::getCount(this->bgmapSprites))
	{
		return NULL;
	}

	int32 counter = 0;

	for(VirtualNode node = this->bgmapSprites->head; NULL != node; node = node->next, counter++)
	{
		if(counter == position)
		{
			return Sprite::safeCast(node->data);
		}
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
ObjectSpriteContainer SpriteManager::getObjectSpriteContainer(fixed_t z)
{
	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;

	NM_ASSERT(!isDeleted(this->objectSpriteContainers), "SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers created");
	NM_ASSERT(0 < VirtualList::getCount(this->objectSpriteContainers), "SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers available");

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
//---------------------------------------------------------------------------------------------------------
ObjectSpriteContainer SpriteManager::getObjectSpriteContainerBySPT(int32 spt)
{
	ASSERT((unsigned)spt < __TOTAL_OBJECT_SEGMENTS, "SpriteManager::getObjectSpriteContainerBySPT: invalid segment");

	if((unsigned)spt > __TOTAL_OBJECT_SEGMENTS)
	{
		return NULL;
	}

	ObjectSpriteContainer objectSpriteContainer = NULL;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; NULL != node; node = node->next)
	{
		objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		if(objectSpriteContainer->spt == spt)
		{
			return objectSpriteContainer;
		}
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::print(int32 x, int32 y, bool resumed)
{
	Printing::setWorldCoordinates(this->printing, 0, 0, Printing::getSpriteIndex(this->printing).z, 0);
#ifndef __SHOW_SPRITES_PROFILING
	SpriteManager::computeTotalPixelsDrawn(this);
#endif

	Printing::text(this->printing, "SPRITES USAGE", x, y++, NULL);
	Printing::text(this->printing, "Total pixels:                ", x, ++y, NULL);
	Printing::int32(this->printing, this->totalPixelsDrawn, x + 22, y, NULL);
	Printing::text(this->printing, "Used layers:                ", x, ++y, NULL);
	Printing::int32(this->printing, __TOTAL_LAYERS - this->freeLayer, x + 22, y, NULL);
	Printing::text(this->printing, "Sprites count:              ", x, ++y, NULL);
	Printing::int32(this->printing, VirtualList::getCount(this->bgmapSprites), x + 22, y, NULL);
#ifdef __SHOW_SPRITES_PROFILING
	Printing::text(this->printing, "Rendered sprites:              ", x, ++y, NULL);
	Printing::int32(this->printing, _renderedSprites, x + 22, y, NULL);
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
	
	for(VirtualNode node = this->bgmapSprites->tail; NULL != node; node = node->previous, counter--)
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SpriteManager::constructor()
{
	// construct base object
	Base::constructor();

	this->totalPixelsDrawn = 0;
	this->deferTextureUpdating = false;

	this->bgmapSprites = NULL;
	this->objectSpriteContainers = NULL;
	this->specialSprites = NULL;

	this->texturesMaximumRowsToWrite = -1;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->evenFrame = __TRANSPARENCY_EVEN;

	this->animationsClock = NULL;
	this->printing = Printing::getInstance();
	this->paramTableManager = ParamTableManager::getInstance();
	this->charSetManager = CharSetManager::getInstance();
	this->bgmapTextureManager = BgmapTextureManager::getInstance();
	this->paramTableManager = ParamTableManager::getInstance();
	this->objectTextureManager = ObjectTextureManager::getInstance();


	this->sortingSpriteNode = NULL;
	this->completeSort = true;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::destructor()
{
	SpriteManager::cleanUp(this);

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::cleanUp()
{
	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
	}

	if(!isDeleted(this->bgmapSprites))
	{
		delete this->bgmapSprites;
	}

	if(!isDeleted(this->objectSpriteContainers))
	{
		delete this->objectSpriteContainers;
	}

	if(!isDeleted(this->specialSprites))
	{
		delete this->specialSprites;
	}

	this->bgmapSprites = NULL;
	this->objectSpriteContainers = NULL;
	this->specialSprites = NULL;
}
//---------------------------------------------------------------------------------------------------------
bool SpriteManager::doRegisterSprite(Sprite sprite)
{
	for(VirtualNode node = this->bgmapSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::doRegisterSprite: NULL node's data");

		Sprite otherSprite = Sprite::safeCast(node->data);

		NM_ASSERT(otherSprite != sprite, "SpriteManager::doRegisterSprite: sprite already registered");

		if(otherSprite == sprite)
		{
			return false;
		}

		if(sprite->position.z + sprite->displacement.z <= otherSprite->position.z + otherSprite->displacement.z)
		{
			this->sortingSpriteNode = VirtualList::insertBefore(this->bgmapSprites, node, sprite);
			return true;
		}
	}

	this->sortingSpriteNode = VirtualList::pushBack(this->bgmapSprites, sprite);

	return true;
}
//---------------------------------------------------------------------------------------------------------
bool SpriteManager::sortProgressively(bool complete)
{
	if(NULL == this->sortingSpriteNode)
	{
		this->sortingSpriteNode = this->bgmapSprites->head;

		if(NULL == this->sortingSpriteNode)
		{
			return false;
		}
	}

	bool swapped = false;

	do
	{
		swapped = false;

		for(VirtualNode node = complete ? this->bgmapSprites->head : this->sortingSpriteNode; NULL != node && NULL != node->next; node = node->next)
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
			}

			if(!complete)
			{
				break;
			}
		}
	}
	while(complete && swapped);

	if(!complete)
	{
		this->sortingSpriteNode = this->sortingSpriteNode->next;
	}

	if(!isDeleted(this->objectSpriteContainers))
	{
		for(VirtualNode node = this->objectSpriteContainers->head; NULL != node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			swapped = swapped || ObjectSpriteContainer::sortProgressively(objectSpriteContainer, complete);
		}
	}

	return swapped;
}
//---------------------------------------------------------------------------------------------------------
int32 SpriteManager::getTotalPixelsDrawn()
{
	int32 totalPixelsToDraw = 0;
	
	for(VirtualNode node = this->bgmapSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::getTotalPixelsDrawn: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(__SHOW == sprite->show)
		{
			totalPixelsToDraw += Sprite::getTotalPixels(sprite);
		}
	}

	return totalPixelsToDraw;
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::stopRendering()
{
	NM_ASSERT(0 <= (int8)this->freeLayer, "SpriteManager::stopRendering: no more layers");

	if(0 <= this->freeLayer)
	{
		_worldAttributesCache[this->freeLayer].head = __WORLD_END;
	}
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::applySpecialEffects()
{
	for(VirtualNode node = this->specialSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::writeGraphicsToDRAM: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(__HIDE == sprite->show)
		{
			continue;
		}

		Sprite::processEffects(sprite);
	}
}
//---------------------------------------------------------------------------------------------------------
void SpriteManager::writeWORLDAttributesToDRAM()
{
	CACHE_RESET;
	Mem::copyWORD((WORD*)(_worldAttributesBaseAddress + this->freeLayer), (WORD*)(_worldAttributesCache + this->freeLayer), sizeof(WorldAttributes) * (__TOTAL_LAYERS - (this->freeLayer)) >> 2);
}
//---------------------------------------------------------------------------------------------------------
