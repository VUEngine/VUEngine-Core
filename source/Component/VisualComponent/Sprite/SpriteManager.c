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
#include <Printer.h>
#include <Sprite.h>
#include <VirtualList.h>
#include <VirtualNode.h>

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

void SpriteManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->bgmapSprites = new VirtualList();
	this->objectSpriteContainers = new VirtualList();
	this->specialSprites = new VirtualList();
	this->totalPixelsDrawn = 0;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->animationsClock = NULL;
	this->freeLayer = __TOTAL_LAYERS - 1;
	this->deferTextureUpdating = false;
	this->texturesMaximumRowsToWrite = -1;
	this->sortingSpriteNode = NULL;
	this->completeSort = true;
	this->evenFrame = __TRANSPARENCY_EVEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destructor()
{
	SpriteManager::stopListeningForVIP(this);

	if(!isDeleted(this->bgmapSprites))
	{
		delete this->bgmapSprites;
		this->bgmapSprites = NULL;
	}

	if(!isDeleted(this->specialSprites))
	{
		delete this->specialSprites;
		this->specialSprites = NULL;
	}

	if(!isDeleted(this->objectSpriteContainers))
	{
		VirtualList::deleteData(this->objectSpriteContainers);
		delete this->objectSpriteContainers;
		this->objectSpriteContainers = NULL;
	}

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::onEvent(ListenerObject eventFirer __attribute__((unused)), uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventVIPManagerGAMESTART:
		{
			SpriteManager::render(this);

			return true;
		}

		case kEventVIPManagerXPEND:
		{
			SpriteManager::writeDRAM(this);

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SpriteManager::getType()
{
	return kSpriteComponent;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::enable()
{
	Base::enable(this);

	HardwareManager::suspendInterrupts();

	Texture::reset();
	Printer::reset(Printer::getInstance());
	CharSetManager::reset(CharSetManager::getInstance());
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());
	ParamTableManager::reset(ParamTableManager::getInstance());
	ObjectSpriteContainer::reset();

	for(int32 i = 0; i < __TOTAL_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	this->freeLayer = __TOTAL_LAYERS - 1;
	this->sortingSpriteNode = NULL;
	this->completeSort = true;
	this->evenFrame = __TRANSPARENCY_EVEN;

	SpriteManager::stopRendering(this);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::disable()
{
	SpriteManager::stopListeningForVIP(this);

	Base::disable(this);

	HardwareManager::suspendInterrupts();

	SpriteManager::destroyAllComponents(this);

	if(!isDeleted(this->bgmapSprites))
	{
		VirtualList::clear(this->bgmapSprites);
	}

	if(!isDeleted(this->objectSpriteContainers))
	{
		VirtualList::deleteData(this->objectSpriteContainers);
	}

	if(!isDeleted(this->specialSprites))
	{
		VirtualList::clear(this->specialSprites);
	}

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::instantiateComponent(Entity owner, const SpriteSpec* spriteSpec)
{
	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Base::instantiateComponent(this, owner, (ComponentSpec*)spriteSpec);

	return SpriteManager::createSprite(this, owner, spriteSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::deinstantiateComponent(Entity owner, Sprite sprite) 
{
	if(isDeleted(sprite))
	{
		return;
	}

	Base::deinstantiateComponent(this, owner, Component::safeCast(sprite));
	
	SpriteManager::destroySprite(this, sprite);
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

void SpriteManager::configure
(
	uint8 texturesMaximumRowsToWrite, int32 maximumParamTableRowsToComputePerCall,
	int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS], Clock animationsClock
)
{
	SpriteManager::setTexturesMaximumRowsToWrite(this, texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall(this, maximumParamTableRowsToComputePerCall);
	SpriteManager::configureObjectSpriteContainers(this, size, z);
	SpriteManager::setAnimationsClock(this, animationsClock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::setAnimationsClock(Clock clock)
{
	this->animationsClock = clock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::createSprite(Entity owner, const SpriteSpec* spriteSpec)
{
	NM_ASSERT(NULL != spriteSpec, "SpriteManager::createSprite: null spriteSpec");

	if(NULL == spriteSpec)
	{
		return NULL;
	}

	Sprite sprite = ((Sprite (*)(Entity, const SpriteSpec*)) ((ComponentSpec*)spriteSpec)->allocator)(owner, spriteSpec);
	ASSERT(!isDeleted(sprite), "SpriteManager::createSprite: failed creating sprite");

	if(NULL == this->components->head)
	{
		SpriteManager::startListeningForVIP(this);			
	}

	VirtualList::pushBack(this->components, sprite);

	Sprite::transform(sprite);

	ClassPointer managerClassPointer = Sprite::getManagerClass(sprite);

	if(typeofclass(SpriteManager) == managerClassPointer)
	{
		SpriteManager::registerSprite(this, sprite);
	}
	else if(typeofclass(ObjectSpriteContainer) == managerClassPointer)
	{
		int16 z = 0;

		if(NULL != sprite->transformation)
		{
			z = __METERS_TO_PIXELS(sprite->transformation->position.z);
		}
		
		ObjectSpriteContainer objectSpriteContainer = SpriteManager::getObjectSpriteContainer(this, z + sprite->displacement.z);

		NM_ASSERT(!isDeleted(objectSpriteContainer), "SpriteManager::createSprite: couldn't get a manager");
		ObjectSpriteContainer::registerSprite(objectSpriteContainer, ObjectSprite::safeCast(sprite));
	}

	return sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destroySprite(Sprite sprite)
{
	NM_ASSERT(!isDeleted(sprite), "SpriteManager::destroySprite: trying to destroy dead sprite");

	if(isDeleted(sprite) || NULL == this->components->head)
	{
		return;
	}

	NM_ASSERT(__GET_CAST(Sprite, sprite), "SpriteManager::destroySprite: trying to destroy a non sprite");

	VirtualList::removeData(this->components, sprite);

	if(NULL == this->components->head)
	{
		SpriteManager::stopListeningForVIP(this);			
	}

	Sprite::hide(sprite);

	ClassPointer managerClassPointer = Sprite::getManagerClass(sprite);

	if(typeofclass(SpriteManager) == managerClassPointer)
	{
		SpriteManager::unregisterSprite(this, sprite);
	}
	else if(typeofclass(ObjectSpriteContainer) == managerClassPointer && NULL != this->objectSpriteContainers)
	{
		ObjectSpriteContainer objectSpriteContainer = ObjectSpriteContainer::safeCast(Sprite::getManager(sprite));

		NM_ASSERT(!isDeleted(objectSpriteContainer), "SpriteManager::destroySprite: couldn't get a manager");
		ObjectSpriteContainer::unregisterSprite(objectSpriteContainer, ObjectSprite::safeCast(sprite));
	}

	delete sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::registerSprite(Sprite sprite)
{
#ifndef __RELEASE
	bool registeringSprite = false;

	NM_ASSERT(!registeringSprite, "SpriteManager::registerSprite: already registering a sprite!");

	if(registeringSprite)
	{
		return false;
	}

	registeringSprite = true;

	ASSERT(Sprite::safeCast(sprite), "SpriteManager::registerSprite: adding no sprite");

//	ASSERT(!__GET_CAST(ObjectSprite, sprite), "SpriteManager::registerSprite: trying to register an object sprite");

	if(VirtualList::find(this->bgmapSprites, sprite))
	{
		Printer::setDebugMode();
		Printer::clear();
		Printer::text(__GET_CLASS_NAME(sprite), 1, 20, NULL);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::unregisterSprite(Sprite sprite)
{
	if(NULL != this->bgmapSprites)
	{
		NM_ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

#ifndef __ENABLE_PROFILER
		NM_ASSERT(!isDeleted(VirtualList::find(this->bgmapSprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");
#endif

		this->sortingSpriteNode = NULL;

		VirtualList::removeData(this->bgmapSprites, sprite);		
	}

	if(NULL != this->specialSprites)
	{
#ifdef __RELEASE
		if(Sprite::hasSpecialEffects(sprite))
#endif
		{
			VirtualList::removeData(this->specialSprites, sprite);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::configureObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS])
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
		NM_ASSERT(z[i] <= previousZ, "SpriteManager::configureObjectSpriteContainers: wrong z");

		if(0 < size[i])
		{
			ObjectSpriteContainer objectSpriteContainer = new ObjectSpriteContainer();
			
			SpriteManager::registerSprite(this, Sprite::safeCast(objectSpriteContainer));
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::setMaximumParamTableRowsToComputePerCall(int32 maximumParamTableRowsToComputePerCall)
{
	this->maximumParamTableRowsToComputePerCall = maximumParamTableRowsToComputePerCall;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 SpriteManager::getMaximumParamTableRowsToComputePerCall()
{
	return this->deferParamTableEffects ? this->maximumParamTableRowsToComputePerCall : -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite)
{
	this->texturesMaximumRowsToWrite = 2 > (int8)texturesMaximumRowsToWrite ? 2 : texturesMaximumRowsToWrite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 SpriteManager::getTexturesMaximumRowsToWrite()
{
	return this->texturesMaximumRowsToWrite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::deferTextureUpdating(bool deferTextureUpdating)
{
	this->deferTextureUpdating = deferTextureUpdating;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::deferParamTableEffects(bool deferParamTableEffects)
{
	this->deferParamTableEffects = deferParamTableEffects;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::sortSprites()
{
	while(SpriteManager::sortProgressively(this, true));

	this->completeSort = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::prepareAll()
{
	bool deferTextureUpdating = this->deferTextureUpdating;

	// Prevent VIP's interrupt from calling render during spriteManager process
	VIPManager::suspendDrawing(VIPManager::getInstance());

	// Must make sure that all textures are completely written
	SpriteManager::deferParamTableEffects(this, false);

	// Make sure all textures are written right now
	SpriteManager::writeTextures(this);

	// Sort all sprites' layers
	SpriteManager::sortSprites(this);

	// Render and draw sprites as soon as possible
	SpriteManager::render(this);
	SpriteManager::writeDRAM(this);

	// Sort all sprites' layers again
	// Don't remove me, some custom sprites depend on others
	// To have been setup up before
	SpriteManager::sortSprites(this);

	// Render and draw sprites as soon as possible again
	SpriteManager::render(this);
	SpriteManager::writeDRAM(this);

	// Defer rendering again
	SpriteManager::deferParamTableEffects(this, true);

	// Restore drawing
	VIPManager::resumeDrawing(VIPManager::getInstance());

	this->deferTextureUpdating = deferTextureUpdating;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::render()
{
#ifdef __SHOW_SPRITES_PROFILING
	_renderedSprites = 0;
#endif

	this->completeSort = SpriteManager::sortProgressively(this, this->completeSort);

	ParamTableManager::defragment(ParamTableManager::getInstance(), true);

	// Switch between even and odd frame
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
		// Sprites. Don't remove.
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
	SpriteManager::computeTotalPixelsDrawn(this);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// This is unsafe since it calls external methods that could trigger modifications of the list of components
#ifdef __TOOLS
void SpriteManager::renderAndDraw()
{
	SpriteManager::render(this);

	// Write render data
	SpriteManager::writeDRAM(this);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::writeDRAM()
{
#ifdef __SHOW_SPRITES_PROFILING
	_writtenTiles = 0;
	_writtenTextureTiles = 0;
	_writtenObjectTiles = 0;
#endif

	// Update all graphical data

	// Update CHAR memory
	CharSetManager::defragment(CharSetManager::getInstance(), true);

	// Update DRAM memory
	Texture::updateTextures(this->texturesMaximumRowsToWrite, this->deferTextureUpdating);

	// Update param tables
	SpriteManager::applySpecialEffects(this);

	// Finally, write OBJ and WORLD attributes to DRAM
	ObjectSpriteContainer::writeDRAM();

	// Finally, write OBJ and WORLD attributes to DRAM
	SpriteManager::writeWORLDAttributesToDRAM(this);

#ifdef __SHOW_SPRITES_PROFILING
	int32 counter = __TARGET_FPS / 5;

	if(0 >= --counter)
	{
		counter = __TARGET_FPS / 10;
		SpriteManager::print(this, 1, 15, true);
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::writeTextures()
{
	CharSetManager::writeCharSets(CharSetManager::getInstance());

	Texture::updateTextures(-1, false);

	CharSetManager::writeCharSets(CharSetManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// This is unsafe since it calls external methods that could trigger modifications of the list of components
#ifdef __TOOLS
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
		Printer::show(Printer::getInstance());
	}
	else
	{
		Printer::hide(Printer::getInstance());
	}

	SpriteManager::stopRendering(this);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// This is unsafe since it calls external methods that could trigger modifications of the list of components
#ifdef __TOOLS
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
		Printer::hide(Printer::getInstance());
	}
	else
	{
		Printer::show(Printer::getInstance());
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::computeTotalPixelsDrawn()
{
	this->totalPixelsDrawn = SpriteManager::getTotalPixelsDrawn(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 SpriteManager::getFreeLayer()
{
	return this->freeLayer;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 SpriteManager::getNumberOfSprites()
{
	return VirtualList::getCount(this->bgmapSprites);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ObjectSpriteContainer SpriteManager::getObjectSpriteContainer(fixed_t z)
{
	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;

	NM_ASSERT
	(
		!isDeleted(this->objectSpriteContainers), 
		"SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers created"
	);

	NM_ASSERT
	(
		0 < VirtualList::getCount(this->objectSpriteContainers), 
		"SpriteManager::getObjectSpriteContainer: no ObjectSpriteContainers available"
	);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::print(int32 x, int32 y, bool resumed)
{
	Printer::setWorldCoordinates(0, 0, Printer::getActiveSpritePosition().z, 0);
#ifndef __SHOW_SPRITES_PROFILING
	SpriteManager::computeTotalPixelsDrawn(this);
#endif

	Printer::text("SPRITES USAGE", x, y++, NULL);
	Printer::text("Total pixels:                ", x, ++y, NULL);
	Printer::int32(this->totalPixelsDrawn, x + 22, y, NULL);
	Printer::text("Used layers:                ", x, ++y, NULL);
	Printer::int32(__TOTAL_LAYERS - this->freeLayer, x + 22, y, NULL);
	Printer::text("Sprites count:              ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->bgmapSprites), x + 22, y, NULL);
#ifdef __SHOW_SPRITES_PROFILING
	Printer::text("Rendered sprites:              ", x, ++y, NULL);
	Printer::int32(_renderedSprites, x + 22, y, NULL);
	Printer::text("Written chars:              ", x, ++y, NULL);
	Printer::int32(_writtenTiles, x + 22, y, NULL);
	Printer::text("Written texture tiles:              ", x, ++y, NULL);
	Printer::int32(_writtenTextureTiles, x + 22, y, NULL);
	Printer::text("Written object tiles:              ", x, ++y, NULL);
	Printer::int32(_writtenObjectTiles, x + 22, y, NULL);
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

		strncpy(spriteClassName, __GET_CLASS_NAME(sprite), __MAX_SPRITE_CLASS_NAME_SIZE);
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 1] = 0;
		spriteClassName[__MAX_SPRITE_CLASS_NAME_SIZE - 2] = '.';

		Printer::int32(counter, auxX, auxY, NULL);
		Printer::text(": ", auxX + 2, auxY, NULL);
		Printer::text(spriteClassName, auxX + 4, auxY, NULL);
//		Printer::int32(sprite->position.z + sprite->displacement.z, auxX + 2, auxY, NULL);
//		Printer::hex(_worldAttributesBaseAddress[sprite->index].head, auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, 4, NULL);
//		Printer::int32(Sprite::getTotalPixels(sprite), auxX + __MAX_SPRITE_CLASS_NAME_SIZE + 4, auxY, NULL);

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

void SpriteManager::printObjectSpriteContainersStatus(int32 x, int32 y)
{
	Printer::text("OBJECTS USAGE", x, y++, NULL);
	int32 totalUsedObjects = 0;
	VirtualNode node = this->objectSpriteContainers->head;

	for(; NULL != node; node = node->next)
	{
		totalUsedObjects += ObjectSpriteContainer::getTotalUsedObjects(ObjectSpriteContainer::safeCast(node->data));
	}

	Printer::text("Total used objects: ", x, ++y, NULL);
	Printer::int32(totalUsedObjects, x + 20, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		for
		(
			VirtualNode node = complete ? this->bgmapSprites->head : this->sortingSpriteNode; NULL != node && NULL != node->next; 
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::stopRendering()
{
	NM_ASSERT(0 <= (int8)this->freeLayer, "SpriteManager::stopRendering: no more layers");

	if(0 <= this->freeLayer)
	{
		_worldAttributesCache[this->freeLayer].head = __WORLD_END;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::applySpecialEffects()
{
	int32 maximumParamTableRowsToComputePerCall = SpriteManager::getMaximumParamTableRowsToComputePerCall(this);

	for(VirtualNode node = this->specialSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::writeGraphicsToDRAM: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(__HIDE == sprite->show)
		{
			continue;
		}

		Sprite::processEffects(sprite, maximumParamTableRowsToComputePerCall);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::writeWORLDAttributesToDRAM()
{
	CACHE_RESET;
	Mem::copyWORD
	(
		(WORD*)(_worldAttributesBaseAddress + this->freeLayer), (WORD*)(_worldAttributesCache + this->freeLayer), 
		sizeof(WorldAttributes) * (__TOTAL_LAYERS - (this->freeLayer)) >> 2
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::startListeningForVIP()
{
	HardwareManager::suspendInterrupts();

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::stopListeningForVIP()
{
	HardwareManager::suspendInterrupts();

	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);

	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
