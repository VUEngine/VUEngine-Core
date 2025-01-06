/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with spriteManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Sprite;
friend class ObjectSpriteContainer;
friend class Texture;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAX_SPRITE_CLASS_NAME_SIZE			14

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __SHOW_SPRITES_PROFILING
int32 _renderedSprites = 0; 
int32 _writtenTiles = 0;
int32 _writtenTextureTiles = 0;
int32 _writtenObjectTiles = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::createComponent(Entity owner, const SpriteSpec* spriteSpec)
{
	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Base::createComponent(this, owner, (ComponentSpec*)spriteSpec);

	return SpriteManager::createSprite(owner, spriteSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destroyComponent(Entity owner, Sprite sprite) 
{
	if(isDeleted(sprite))
	{
		return;
	}

	Base::destroyComponent(this, owner, Component::safeCast(sprite));
	
	SpriteManager::destroySprite(sprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::isAnyVisible(Entity owner)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::reset()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	HardwareManager::suspendInterrupts();

	Texture::reset();
	Printing::reset();
	CharSetManager::reset();
	BgmapTextureManager::reset();
	ParamTableManager::reset();
	
	SpriteManager::cleanUp();
	ObjectSpriteContainer::reset();

	int32 i = 0;
	// Clean OBJ memory
	for(; i < __TOTAL_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	spriteManager->animationsClock = NULL;
	spriteManager->bgmapSprites = new VirtualList();
	spriteManager->objectSpriteContainers = new VirtualList();
	spriteManager->specialSprites = new VirtualList();

	spriteManager->freeLayer = __TOTAL_LAYERS - 1;
	spriteManager->deferTextureUpdating = false;
	spriteManager->texturesMaximumRowsToWrite = -1;
	spriteManager->sortingSpriteNode = NULL;
	spriteManager->completeSort = true;

	SpriteManager::stopRendering();

	spriteManager->evenFrame = __TRANSPARENCY_EVEN;

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::setAnimationsClock(Clock clock)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->animationsClock = clock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sprite SpriteManager::createSprite(Entity owner, const SpriteSpec* spriteSpec)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	NM_ASSERT(NULL != spriteSpec, "SpriteManager::createSprite: null spriteSpec");

	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Sprite sprite = ((Sprite (*)(Entity, const SpriteSpec*)) ((ComponentSpec*)spriteSpec)->allocator)(owner, spriteSpec);
	ASSERT(!isDeleted(sprite), "SpriteManager::createSprite: failed creating sprite");

	VirtualList::pushBack(spriteManager->components, sprite);

	Sprite::transform(sprite);
	Sprite::registerWithManager(sprite);

	return sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::destroySprite(Sprite sprite)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	NM_ASSERT(!isDeleted(sprite), "SpriteManager::destroySprite: trying to dispose dead sprite");
	NM_ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::destroySprite: trying to dispose a non sprite");

	if(isDeleted(sprite))
	{
		return;
	}

	VirtualList::removeData(spriteManager->components, sprite);

	Sprite::hide(sprite);
	Sprite::unregisterWithManager(sprite);

	delete sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool SpriteManager::registerSprite(Sprite sprite)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

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

	if(VirtualList::find(spriteManager->bgmapSprites, sprite))
	{
		Printing::setDebugMode();
		Printing::clear();
		Printing::text(__GET_CLASS_NAME(sprite), 1, 20, NULL);
		NM_ASSERT(false, "SpriteManager::registerSprite: sprite already registered");
	}
#endif

	if(!isDeleted(sprite))
	{
		if(SpriteManager::doRegisterSprite(sprite) && Sprite::hasSpecialEffects(sprite))
		{
			VirtualList::pushBack(spriteManager->specialSprites, sprite);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::unregisterSprite(Sprite sprite)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	NM_ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

#ifndef __ENABLE_PROFILER
	NM_ASSERT(!isDeleted(VirtualList::find(spriteManager->bgmapSprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");
#endif

	spriteManager->sortingSpriteNode = NULL;

	VirtualList::removeData(spriteManager->bgmapSprites, sprite);

#ifdef __RELEASE
	if(Sprite::hasSpecialEffects(sprite))
#endif
	{
		VirtualList::removeData(spriteManager->specialSprites, sprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::setupObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS])
{
	SpriteManager spriteManager = SpriteManager::getInstance();

#ifndef __RELEASE
	int16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	if(isDeleted(spriteManager->objectSpriteContainers) ||  0 < VirtualList::getCount(spriteManager->objectSpriteContainers))
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
			VirtualList::pushBack(spriteManager->objectSpriteContainers, objectSpriteContainer);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::setMaximumParamTableRowsToComputePerCall(int32 maximumParamTableRowsToComputePerCall)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 SpriteManager::getMaximumParamTableRowsToComputePerCall()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	return spriteManager->deferParamTableEffects ? spriteManager->maximumParamTableRowsToComputePerCall : -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->texturesMaximumRowsToWrite = 2 > (int8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int8 SpriteManager::getTexturesMaximumRowsToWrite()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	return spriteManager->texturesMaximumRowsToWrite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::deferTextureUpdating(bool deferTextureUpdating)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->deferTextureUpdating = deferTextureUpdating;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::deferParamTableEffects(bool deferParamTableEffects)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->deferParamTableEffects = deferParamTableEffects;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::sortSprites()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	while(SpriteManager::sortProgressively(true));

	spriteManager->completeSort = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::prepareAll()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	bool deferTextureUpdating = spriteManager->deferTextureUpdating;

	// Prevent VIP's interrupt from calling render during spriteManager process
	VIPManager::suspendDrawing(VIPManager::getInstance());

	// Must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(false);

	// Make sure all textures are written right now
	SpriteManager::writeTextures();

	// Sort all sprites' layers
	SpriteManager::sortSprites();

	// Render and draw sprites as soon as possible
	SpriteManager::renderAndDraw();

	// Sort all sprites' layers again
	// Don't remove me, some custom sprites depend on others
	// To have been setup up before
	SpriteManager::sortSprites();

	// Render and draw sprites as soon as possible again
	SpriteManager::renderAndDraw();

	// Defer rendering again
	SpriteManager::deferParamTableEffects(true);

	// Restore drawing
	VIPManager::resumeDrawing(VIPManager::getInstance());

	spriteManager->deferTextureUpdating = deferTextureUpdating;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::render()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

#ifdef __SHOW_SPRITES_PROFILING
	_renderedSprites = 0;
#endif

	spriteManager->completeSort = SpriteManager::sortProgressively(spriteManager->completeSort);

	ParamTableManager::defragment(true);

	// Switch between even and odd frame
	spriteManager->evenFrame = __TRANSPARENCY_EVEN == spriteManager->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	spriteManager->freeLayer = __TOTAL_LAYERS - 1;

	bool updateAnimations = true;
	
	if(!isDeleted(spriteManager->animationsClock))
	{
		updateAnimations = !Clock::isPaused(spriteManager->animationsClock);
	}

	for(VirtualNode node = spriteManager->bgmapSprites->tail; NULL != node && 0 < spriteManager->freeLayer; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::render: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		// Saves on method calls quite a bit when there are lots of
		// Sprites. Don't remove.
		if(__HIDE == sprite->show || (sprite->transparency & spriteManager->evenFrame))
		{
			sprite->index = __NO_RENDER_INDEX;
			continue;
		}

		if(Sprite::render(sprite, spriteManager->freeLayer, updateAnimations) == spriteManager->freeLayer)
		{
			spriteManager->freeLayer--;
		}
	}

	NM_ASSERT(0 <= spriteManager->freeLayer, "SpriteManager::render: more sprites than WORLDs");

	ObjectSpriteContainer::prepareForRendering();

	for(VirtualNode node = spriteManager->objectSpriteContainers->head; NULL != node; node = node->next)
	{
		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

		ObjectSpriteContainer::renderSprites(objectSpriteContainer, spriteManager->evenFrame, updateAnimations);
	}

	ObjectSpriteContainer::finishRendering();

	SpriteManager::stopRendering();

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInToolState(VUEngine::getInstance()))
	{
		SpriteManager::computeTotalPixelsDrawn();
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::forceRendering()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	for(VirtualNode node = spriteManager->components->tail; NULL != node; node = node->previous)
	{
		Sprite::invalidateRendering(Sprite::safeCast(node->data));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::renderAndDraw()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	SpriteManager::render();

	// Write render data
	SpriteManager::writeDRAM();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::writeDRAM()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

#ifdef __SHOW_SPRITES_PROFILING
	_writtenTiles = 0;
	_writtenTextureTiles = 0;
	_writtenObjectTiles = 0;
#endif

	// Update all graphical data

	// Update CHAR memory
	CharSetManager::defragment(true);

	// Update DRAM memory
	Texture::updateTextures(spriteManager->texturesMaximumRowsToWrite, spriteManager->deferTextureUpdating);

	// Update param tables
	SpriteManager::applySpecialEffects();

	// Finally, write OBJ and WORLD attributes to DRAM
	ObjectSpriteContainer::writeDRAM();

	// Finally, write OBJ and WORLD attributes to DRAM
	SpriteManager::writeWORLDAttributesToDRAM();

#ifdef __SHOW_SPRITES_PROFILING
	if(!VUEngine::isInToolState(VUEngine::getInstance()))
	{
		static int32 counter = __TARGET_FPS / 5;

		if(0 >= --counter)
		{
			counter = __TARGET_FPS / 10;
			SpriteManager::print(1, 15, true);
		}
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::writeTextures()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	CharSetManager::writeCharSets();

	Texture::updateTextures(-1, false);

	CharSetManager::writeCharSets();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::showAllSprites(Sprite spareSprite, bool showPrinting)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	for(VirtualNode node = spriteManager->components->tail; NULL != node; node = node->previous)
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
		Printing::show();
	}
	else
	{
		Printing::hide();
	}

	SpriteManager::stopRendering();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::hideAllSprites(Sprite spareSprite, bool hidePrinting)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	for(VirtualNode node = spriteManager->components->head; NULL != node; node = node->next)
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
		Printing::hide();
	}
	else
	{
		Printing::show();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::computeTotalPixelsDrawn()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	spriteManager->totalPixelsDrawn = SpriteManager::getTotalPixelsDrawn();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int8 SpriteManager::getFreeLayer()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	return spriteManager->freeLayer;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 SpriteManager::getNumberOfSprites()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	return VirtualList::getCount(spriteManager->bgmapSprites);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sprite SpriteManager::getSpriteAtIndex(int16 position)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	if(0 > position || position >= VirtualList::getCount(spriteManager->bgmapSprites))
	{
		return NULL;
	}

	int32 counter = 0;

	for(VirtualNode node = spriteManager->bgmapSprites->head; NULL != node; node = node->next, counter++)
	{
		if(counter == position)
		{
			return Sprite::safeCast(node->data);
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ObjectSpriteContainer SpriteManager::getObjectSpriteContainer(fixed_t z)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;

	NM_ASSERT
	(
		!isDeleted(spriteManager->objectSpriteContainers), 
		"SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers created"
	);

	NM_ASSERT
	(
		0 < VirtualList::getCount(spriteManager->objectSpriteContainers), 
		"SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers available"
	);

	if(isDeleted(spriteManager->objectSpriteContainers))
	{
		return NULL;
	}

	for(VirtualNode node = spriteManager->objectSpriteContainers->head; NULL != node; node = node->next)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ObjectSpriteContainer SpriteManager::getObjectSpriteContainerBySPT(int32 spt)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	ASSERT((unsigned)spt < __TOTAL_OBJECT_SEGMENTS, "SpriteManager::getObjectSpriteContainerBySPT: invalid segment");

	if((unsigned)spt > __TOTAL_OBJECT_SEGMENTS)
	{
		return NULL;
	}

	ObjectSpriteContainer objectSpriteContainer = NULL;
	VirtualNode node = spriteManager->objectSpriteContainers->head;

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::print(int32 x, int32 y, bool resumed)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	Printing::setWorldCoordinates(0, 0, Printing::getSpriteIndex().z, 0);
#ifndef __SHOW_SPRITES_PROFILING
	SpriteManager::computeTotalPixelsDrawn();
#endif

	Printing::text("SPRITES USAGE", x, y++, NULL);
	Printing::text("Total pixels:                ", x, ++y, NULL);
	Printing::int32(spriteManager->totalPixelsDrawn, x + 22, y, NULL);
	Printing::text("Used layers:                ", x, ++y, NULL);
	Printing::int32(__TOTAL_LAYERS - spriteManager->freeLayer, x + 22, y, NULL);
	Printing::text("Sprites count:              ", x, ++y, NULL);
	Printing::int32(VirtualList::getCount(spriteManager->bgmapSprites), x + 22, y, NULL);
#ifdef __SHOW_SPRITES_PROFILING
	Printing::text("Rendered sprites:              ", x, ++y, NULL);
	Printing::int32(_renderedSprites, x + 22, y, NULL);
	Printing::text("Written chars:              ", x, ++y, NULL);
	Printing::int32(_writtenTiles, x + 22, y, NULL);
	Printing::text("Written texture tiles:              ", x, ++y, NULL);
	Printing::int32(_writtenTextureTiles, x + 22, y, NULL);
	Printing::text("Written object tiles:              ", x, ++y, NULL);
	Printing::int32(_writtenObjectTiles, x + 22, y, NULL);
#endif

	if(resumed)
	{
		return;
	}

	int32 auxY = y + 2;
	int32 auxX = x;

	int32 counter = __TOTAL_LAYERS - 1;
	
	for(VirtualNode node = spriteManager->bgmapSprites->tail; NULL != node; node = node->previous, counter--)
	{
		char spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE];
		Sprite sprite = Sprite::safeCast(node->data);

		strncpy(spriteClassName, __GET_CLASS_NAME(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printing::int32(counter, auxX, auxY, NULL);
		Printing::text(": ", auxX + 2, auxY, NULL);
		Printing::text(spriteClassName, auxX + 4, auxY, NULL);
//		Printing::int32(sprite->position.z + sprite->displacement.z, auxX + 2, auxY, NULL);
//		Printing::hex(_worldAttributesBaseAddress[sprite->index].head, auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, 4, NULL);
//		Printing::int32(Sprite::getTotalPixels(sprite), auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, NULL);

		++auxY;
		if(__TOTAL_LAYERS / 2 == counter)
//		if((__SCREEN_HEIGHT_IN_CHARS) - 2 <= ++auxY)
		{
			auxY = y + 2;
			auxX += __MAX_SPRITE_CLASS_NAME_SIZE + 10;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::printObjectSpriteContainersStatus(int32 x, int32 y)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	Printing::text("OBJECTS USAGE", x, y++, NULL);
	int32 totalUsedObjects = 0;
	VirtualNode node = spriteManager->objectSpriteContainers->head;

	for(; NULL != node; node = node->next)
	{
		totalUsedObjects += ObjectSpriteContainer::getTotalUsedObjects(ObjectSpriteContainer::safeCast(node->data));
	}

	Printing::text("Total used objects: ", x, ++y, NULL);
	Printing::int32(totalUsedObjects, x + 20, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::cleanUp()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	if(!isDeleted(spriteManager->components))
	{
		VirtualList::deleteData(spriteManager->components);
	}

	if(!isDeleted(spriteManager->bgmapSprites))
	{
		delete spriteManager->bgmapSprites;
	}

	if(!isDeleted(spriteManager->objectSpriteContainers))
	{
		VirtualList::deleteData(spriteManager->objectSpriteContainers);
		delete spriteManager->objectSpriteContainers;
	}

	if(!isDeleted(spriteManager->specialSprites))
	{
		delete spriteManager->specialSprites;
	}

	spriteManager->bgmapSprites = NULL;
	spriteManager->objectSpriteContainers = NULL;
	spriteManager->specialSprites = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool SpriteManager::doRegisterSprite(Sprite sprite)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	for(VirtualNode node = spriteManager->bgmapSprites->head; NULL != node; node = node->next)
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
			spriteManager->sortingSpriteNode = VirtualList::insertBefore(spriteManager->bgmapSprites, node, sprite);
			return true;
		}
	}

	spriteManager->sortingSpriteNode = VirtualList::pushBack(spriteManager->bgmapSprites, sprite);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool SpriteManager::sortProgressively(bool complete)
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	if(NULL == spriteManager->sortingSpriteNode)
	{
		spriteManager->sortingSpriteNode = spriteManager->bgmapSprites->head;

		if(NULL == spriteManager->sortingSpriteNode)
		{
			return false;
		}
	}

	bool swapped = false;

	do
	{
		swapped = false;

		for
		(
			VirtualNode node = complete ? spriteManager->bgmapSprites->head : spriteManager->sortingSpriteNode; NULL != node && NULL != node->next; 
			node = node->next
		)
		{
			VirtualNode nextNode = node->next;

			NM_ASSERT(!isDeleted(node->data), "SpriteManager::sortProgressively: NULL node's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "SpriteManager::sortProgressively: node's data isn't a sprite");

			Sprite sprite = Sprite::safeCast(node->data);

			NM_ASSERT(!isDeleted(nextNode->data), "SpriteManager::sortProgressively: NULL nextNode's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "SpriteManager::sortProgressively: NULL nextNode's data cast");

			Sprite nextSprite = Sprite::safeCast(nextNode->data);

			// Check if z positions are swapped
			if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
			{
				// Swap nodes' data
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
		spriteManager->sortingSpriteNode = spriteManager->sortingSpriteNode->next;
	}

	if(!isDeleted(spriteManager->objectSpriteContainers))
	{
		for(VirtualNode node = spriteManager->objectSpriteContainers->head; NULL != node; node = node->next)
		{
			ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(node->data);

			swapped = swapped || ObjectSpriteContainer::sortProgressively(objectSpriteContainer, complete);
		}
	}

	return swapped;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 SpriteManager::getTotalPixelsDrawn()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	int32 totalPixelsToDraw = 0;
	
	for(VirtualNode node = spriteManager->bgmapSprites->head; NULL != node; node = node->next)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::stopRendering()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	NM_ASSERT(0 <= (int8)spriteManager->freeLayer, "SpriteManager::stopRendering: no more layers");

	if(0 <= spriteManager->freeLayer)
	{
		_worldAttributesCache[spriteManager->freeLayer].head = __WORLD_END;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::applySpecialEffects()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	for(VirtualNode node = spriteManager->specialSprites->head; NULL != node; node = node->next)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SpriteManager::writeWORLDAttributesToDRAM()
{
	SpriteManager spriteManager = SpriteManager::getInstance();

	CACHE_RESET;
	Mem::copyWORD
	(
		(WORD*)(_worldAttributesBaseAddress + spriteManager->freeLayer), (WORD*)(_worldAttributesCache + spriteManager->freeLayer), 
		sizeof(WorldAttributes) * (__TOTAL_LAYERS - (spriteManager->freeLayer)) >> 2
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::constructor()
{
	// Always explicitly call the base's constructor 
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
	this->objectTextureManager = ObjectTextureManager::getInstance();

	this->sortingSpriteNode = NULL;
	this->completeSort = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destructor()
{
	SpriteManager::cleanUp();

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
