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

#include <BgmapSprite.h>
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
friend class ObjectSprite;
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

	this->totalPixelsDrawn = 0;
	this->maximumParamTableRowsToComputePerCall = -1;
	this->deferParamTableEffects = false;
	this->animationsClock = NULL;
	this->bgmapIndex = __TOTAL_LAYERS - 1;
	this->deferTextureUpdating = false;
	this->texturesMaximumRowsToWrite = -1;
	this->completeSort = true;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->spt = __TOTAL_OBJECT_SEGMENTS - 1;
	this->objectIndex = __TOTAL_OBJECTS - 1;
	this->previousObjectIndex = __TOTAL_OBJECTS - 1;

	for(int16 i = 0; i < kSpriteListEnd; i++)
	{
		this->spriteRegistry[i].sprites = new VirtualList();
		this->spriteRegistry[i].sortingNode = NULL;
	}
	
	for(int16 i = 0; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->vipSPTRegistersCache[i] = this->objectIndex;
		this->objectSpriteContainers[i] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destructor()
{
	SpriteManager::stopListeningForVIP(this);

	for(int16 i = 0; i < kSpriteListEnd; i++)
	{
		if(!isDeleted(this->spriteRegistry[i].sprites))
		{
			delete this->spriteRegistry[i].sprites;
			this->spriteRegistry[i].sprites = NULL;
		}

		this->spriteRegistry[i].sortingNode = NULL;
	}

	for(int16 i = 0; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		if(!isDeleted(this->objectSpriteContainers[i]))
		{
			delete this->objectSpriteContainers[i];
			this->objectSpriteContainers[i] = NULL;
		}
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

	for(int32 i = __TOTAL_OBJECTS - 1; 0 <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	this->spt = __TOTAL_OBJECT_SEGMENTS - 1;
	this->objectIndex = __TOTAL_OBJECTS - 1;

	for(int16 i = 0; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->vipSPTRegistersCache[i] = this->objectIndex;
	}

	for(int32 i = 0; i < __TOTAL_OBJECTS; i++)
	{
		_vipRegisters[__SPT3 - i] = 0;
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = 0;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	this->bgmapIndex = __TOTAL_LAYERS - 1;
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

	for(int16 i = 0; i < kSpriteListEnd; i++)
	{
		if(!isDeleted(this->spriteRegistry[i].sprites))
		{
			VirtualList::clear(this->spriteRegistry[i].sprites);
		}

		this->spriteRegistry[i].sortingNode = NULL;
	}

	for(int16 i = 0; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->vipSPTRegistersCache[i] = this->objectIndex;

		if(!isDeleted(this->objectSpriteContainers[i]))
		{
			delete this->objectSpriteContainers[i];
			this->objectSpriteContainers[i] = NULL;
		}
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
	const int16 size[__TOTAL_OBJECT_SEGMENTS], const int16 z[__TOTAL_OBJECT_SEGMENTS], Clock animationsClock
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

	ClassPointer classPointer = Sprite::getBasicType(sprite);

	if(typeofclass(ObjectSprite) == classPointer)
	{
		int16 z = 0;

		if(NULL != sprite->transformation)
		{
			z = __METERS_TO_PIXELS(sprite->transformation->position.z);
		}
		
		int16 spriteListObjectIndex = SpriteManager::getObjectSpriteContainer(this, z + sprite->displacement.z);

		NM_ASSERT(0 <= spriteListObjectIndex, "SpriteManager::createSprite: invalid object list index");

		if(0 <= spriteListObjectIndex)
		{
			SpriteManager::registerSprite(this, sprite, &this->spriteRegistry[spriteListObjectIndex + kSpriteListObject1]);
		}

	}
	else if(typeofclass(BgmapSprite) == classPointer)
	{
		SpriteManager::registerSprite(this, sprite, &this->spriteRegistry[kSpriteListBgmap1]);
	}
	else if(typeofclass(Sprite) == classPointer)
	{
		SpriteManager::registerSprite(this, sprite, &this->spriteRegistry[kSpriteListBgmap1]);
	}

	if(Sprite::hasSpecialEffects(sprite))
	{
		if(!isDeleted(this->spriteRegistry[kSpriteListSpecial].sprites))
		{
			VirtualList::pushBack(this->spriteRegistry[kSpriteListSpecial].sprites, sprite);
		}
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

	ClassPointer classPointer = Sprite::getBasicType(sprite);

	if(typeofclass(ObjectSprite) == classPointer)
	{
		for(int16 i = kSpriteListObject1; i < kSpriteListObject1 + __TOTAL_OBJECT_SEGMENTS; i++)
		{
			SpriteManager::unregisterSprite(this, sprite, &this->spriteRegistry[i]);
		}
	}
	else if(typeofclass(BgmapSprite) == classPointer)
	{
		SpriteManager::unregisterSprite(this, sprite, &this->spriteRegistry[kSpriteListBgmap1]);
	}
	else if(typeofclass(Sprite) == classPointer)
	{
		SpriteManager::unregisterSprite(this, sprite, &this->spriteRegistry[kSpriteListBgmap1]);
	}

#ifdef __RELEASE
	if(Sprite::hasSpecialEffects(sprite))
#endif
	{
		if(!isDeleted(this->spriteRegistry[kSpriteListSpecial].sprites))
		{
			VirtualList::removeData(this->spriteRegistry[kSpriteListSpecial].sprites, sprite);
		}
	}

	delete sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::registerSprite(Sprite sprite, SpriteRegistry* spriteRegistry)
{
	NM_ASSERT(sprite, "SpriteManager::registerSprite: null sprite");
	NM_ASSERT(NULL != spriteRegistry, "SpriteManager::registerSprite: null spriteRegistry");
	NM_ASSERT(NULL != spriteRegistry->sprites, "SpriteManager::registerSprite: null sprites list");

	if(!isDeleted(sprite) && NULL != spriteRegistry && NULL != spriteRegistry->sprites)
	{
		for(VirtualNode node = spriteRegistry->sprites->head; NULL != node; node = node->next)
		{
			NM_ASSERT(!isDeleted(node->data), "SpriteManager::registerSprite: NULL node's data");

			Sprite otherSprite = Sprite::safeCast(node->data);

			NM_ASSERT(otherSprite != sprite, "SpriteManager::registerSprite: sprite already registered");

			if(otherSprite == sprite)
			{
				return;
			}

			if(sprite->position.z + sprite->displacement.z <= otherSprite->position.z + otherSprite->displacement.z)
			{
				spriteRegistry->sortingNode = NULL;
				VirtualList::insertBefore(spriteRegistry->sprites, node, sprite);
				return;
			}
		}

		spriteRegistry->sortingNode = NULL;
		VirtualList::pushBack(spriteRegistry->sprites, sprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::unregisterSprite(Sprite sprite, SpriteRegistry* spriteRegistry)
{
	if(!isDeleted(sprite) && NULL != spriteRegistry && NULL != spriteRegistry->sprites)
	{
		NM_ASSERT(Sprite::safeCast(sprite), "SpriteManager::unregisterSprite: removing no sprite");

#ifndef __ENABLE_PROFILER
//		NM_ASSERT(!isDeleted(VirtualList::find(sprites, sprite)), "SpriteManager::unregisterSprite: sprite not found");
#endif
		spriteRegistry->sortingNode = NULL;
		VirtualList::removeData(spriteRegistry->sprites, sprite);		
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::configureObjectSpriteContainers(const int16 size[__TOTAL_OBJECT_SEGMENTS], const int16 z[__TOTAL_OBJECT_SEGMENTS])
{
#ifndef __RELEASE
	int16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "SpriteManager::configureObjectSpriteContainers: wrong z");

		if(0 < size[i])
		{
			NM_ASSERT(isDeleted(this->objectSpriteContainers[i]), "SpriteManager::configureObjectSpriteContainers: error creating container");

			if(!isDeleted(this->objectSpriteContainers[i]))
			{
				delete this->objectSpriteContainers[i];
				this->objectSpriteContainers[i] = NULL;
			}
			
			this->objectSpriteContainers[i] = new ObjectSpriteContainer();
			
			SpriteManager::registerSprite(this, Sprite::safeCast(this->objectSpriteContainers[i]), &this->spriteRegistry[kSpriteListBgmap1]);

			PixelVector position =
			{
				0, 0, z[i], 0
			};

			ObjectSpriteContainer::setPosition(this->objectSpriteContainers[i], &position);

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

	this->bgmapIndex = __TOTAL_LAYERS - 1;

	bool updateAnimations = true;
	
	if(!isDeleted(this->animationsClock))
	{
		updateAnimations = !Clock::isPaused(this->animationsClock);
	}

	SpriteManager::startRendering(this);

	for(VirtualNode node = this->spriteRegistry[kSpriteListBgmap1].sprites->tail, previousNode = NULL; NULL != node; node = previousNode)
	{
		previousNode = node->previous;

		Sprite sprite = Sprite::safeCast(node->data);
/*
		if(sprite->deleteMe)
		{
			VirtualList::removeNode(this->spriteRegistry[kSpriteListBgmap1].sprites, node);

			this->sortingSpriteNode = NULL;

			if(Sprite::hasSpecialEffects(sprite))
			{
				VirtualList::removeData(this->spriteRegistry[kSpriteListSpecial].sprites, sprite);
			}

			SpriteManager::destroySprite(this, sprite);
			continue;
		}
*/
		// Saves on method calls quite a bit when there are lots of
		// Sprites. Don't remove.
		if(__HIDE == sprite->show || (sprite->transparency & this->evenFrame))
		{
			sprite->index = __NO_RENDER_INDEX;
			continue;
		}

		if(Sprite::render(sprite, this->bgmapIndex, updateAnimations) == this->bgmapIndex)
		{
			this->bgmapIndex--;
		}
	}

	NM_ASSERT(0 <= this->bgmapIndex, "SpriteManager::render: more sprites than WORLDs");

	for(int16 i = kSpriteListObject1; i < kSpriteListObject1 + __TOTAL_OBJECT_SEGMENTS; i++)
	{
		ObjectSpriteContainer objectSpriteContainer = this->objectSpriteContainers[i - kSpriteListObject1];

		if(NULL == objectSpriteContainer)
		{
			continue;
		}
		
		int16 firstObjectIndex = this->objectIndex;

		if(__SHOW == objectSpriteContainer->show)
		{
			for(VirtualNode node = this->spriteRegistry[i].sprites->head, nextNode = NULL; NULL != node; node = nextNode)
			{
				nextNode = node->next;

				ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);
/*
				if(objectSprite->deleteMe)
				{
					VirtualList::removeNode(objectSpriteContainer->objectSprites, node);

					objectSprite->objectSpriteContainer = NULL;
					objectSpriteContainer->sortingSpriteNode = NULL;
					SpriteManager::destroySprite(this, Sprite::safeCast(objectSprite));
					continue;
				}
*/
				// Saves on method calls quite a bit when there are lots of
				// Sprites. Don't remove.
				if(__HIDE == objectSprite->show || (objectSprite->transparency & this->evenFrame) || (0 > this->objectIndex - objectSprite->totalObjects))
				{
					NM_ASSERT(0 < this->objectIndex - objectSprite->totalObjects, "ObjectSpriteContainer::renderSprites: OBJECTS depleted");
					objectSprite->index = __NO_RENDER_INDEX;
					continue;
				}

				// Do not change the order of this condition, objectSprite->totalObjects may be modified during rendering
				// But calling ObjectSprite::getTotalObjects is too costly
				if
				(
					ObjectSprite::render(objectSprite, this->objectIndex - (objectSprite->totalObjects - 1), updateAnimations) 
					== 
					this->objectIndex - (objectSprite->totalObjects - 1)
				)
				{
					this->objectIndex -= objectSprite->totalObjects;
				}
			}
		}

		if(firstObjectIndex == this->objectIndex)
		{
			_objectAttributesCache[this->objectIndex].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			this->objectIndex--;

			_worldAttributesCache[objectSpriteContainer->index].head = __WORLD_OFF;
		}
		else
		{
			_worldAttributesCache[objectSpriteContainer->index].head = (__WORLD_ON | __WORLD_OBJECT | __WORLD_OVR) & (~__WORLD_END);

			// Make sure that the rest of spt segments only run up to the last
			// Used object index
			for(int32 i = this->spt--; i--;)
			{
				this->vipSPTRegistersCache[i] = this->objectIndex;
			}
		}
	}

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
	SpriteManager::writeAttributesToDRAM(this);

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

int8 SpriteManager::getbgmapIndex()
{
	return this->bgmapIndex;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 SpriteManager::getNumberOfSprites()
{
	return VirtualList::getCount(this->spriteRegistry[kSpriteListBgmap1].sprites);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::getSpriteAtIndex(int16 position)
{
	if(0 > position || position >= VirtualList::getCount(this->spriteRegistry[kSpriteListBgmap1].sprites))
	{
		return NULL;
	}

	int32 counter = 0;

	for(VirtualNode node = this->spriteRegistry[kSpriteListBgmap1].sprites->head; NULL != node; node = node->next, counter++)
	{
		if(counter == position)
		{
			return Sprite::safeCast(node->data);
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 SpriteManager::getObjectSpriteContainer(fixed_t z)
{
	int16 index = -1;

	for(int16 i = 0; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		if(NULL == this->objectSpriteContainers[i])
		{
			continue;
		}

		if(0 > index)
		{
			index = i;
		}
		else
		{
			if
			(
				__ABS
				(
					Sprite::getPosition(this->objectSpriteContainers[i])->z - z) 
					< 
					__ABS(Sprite::getPosition(this->objectSpriteContainers[index])->z - z
				)
			)
			{
				index = i;
			}
		}
	}

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ObjectSpriteContainer SpriteManager::getObjectSpriteContainerBySPT(int32 spt)
{
	ASSERT((unsigned)spt < __TOTAL_OBJECT_SEGMENTS, "SpriteManager::getObjectSpriteContainerBySPT: invalid segment");

	if((unsigned)spt > __TOTAL_OBJECT_SEGMENTS)
	{
		return NULL;
	}

	return this->objectSpriteContainers[spt];
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
	Printer::int32(__TOTAL_LAYERS - this->bgmapIndex, x + 22, y, NULL);
	Printer::text("Sprites count:              ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->spriteRegistry[kSpriteListBgmap1].sprites), x + 22, y, NULL);
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
	
	for(VirtualNode node = this->spriteRegistry[kSpriteListBgmap1].sprites->tail; NULL != node; node = node->previous, counter--)
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

	// TODO
	//	totalUsedObjects += ObjectSpriteContainer::getTotalUsedObjects(ObjectSpriteContainer::safeCast(node->data));

	Printer::text("Total used objects: ", x, ++y, NULL);
	Printer::int32(totalUsedObjects, x + 20, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::sortProgressively(bool complete)
{
	bool swapped = false;

	for(int16 i = kSpriteListBgmap1; i < kSpriteListEnd; i++)
	{
		if(NULL == this->spriteRegistry[i].sprites)
		{
			continue;
		}

		if(NULL == this->spriteRegistry[i].sortingNode)
		{
			this->spriteRegistry[i].sortingNode = this->spriteRegistry[i].sprites->head;

			if(NULL == this->spriteRegistry[i].sortingNode)
			{
				continue;
			}
		}

		do
		{
			swapped = false;

			for
			(
				VirtualNode node = complete ? this->spriteRegistry[i].sprites->head : this->spriteRegistry[i].sortingNode; 
				NULL != node && NULL != node->next; 
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
					this->spriteRegistry[i].sortingNode = nextNode;//this->spriteRegistry[i].sortingNode->next;
					break;
				}
			}
		}
		while(swapped);

		if(swapped)
		{
			break;
		}
	}

	return swapped;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 SpriteManager::getTotalPixelsDrawn()
{
	int32 totalPixelsToDraw = 0;
	
	for(VirtualNode node = this->spriteRegistry[kSpriteListBgmap1].sprites->head; NULL != node; node = node->next)
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

void SpriteManager::startRendering()
{
	this->spt = __TOTAL_OBJECT_SEGMENTS - 1;
	this->objectIndex = __TOTAL_OBJECTS - 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::stopRendering()
{
	NM_ASSERT(0 <= (int8)this->bgmapIndex, "SpriteManager::stopRendering: no more layers");

	if(0 <= this->bgmapIndex)
	{
		_worldAttributesCache[this->bgmapIndex].head = __WORLD_END;
	}

	// Clear OBJ memory
	for(int32 i = this->objectIndex; this->previousObjectIndex <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	this->previousObjectIndex = this->objectIndex;}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::applySpecialEffects()
{
	int32 maximumParamTableRowsToComputePerCall = SpriteManager::getMaximumParamTableRowsToComputePerCall(this);

	for(VirtualNode node = this->spriteRegistry[kSpriteListSpecial].sprites->head; NULL != node; node = node->next)
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

void SpriteManager::writeAttributesToDRAM()
{
#ifdef __SHOW_SPRITES_PROFILING
	_writtenObjectTiles = __TOTAL_OBJECTS - this->objectIndex;
#endif

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegisters[__SPT0 + i] = this->vipSPTRegistersCache[i] - this->objectIndex;
	}

	CACHE_RESET;

	Mem::copyWORD
	(
		(WORD*)(_objectAttributesBaseAddress), (WORD*)(_objectAttributesCache + this->objectIndex), 
		sizeof(ObjectAttributes) * (__TOTAL_OBJECTS - this->objectIndex) >> 2
	);

	Mem::copyWORD
	(
		(WORD*)(_worldAttributesBaseAddress + this->bgmapIndex), (WORD*)(_worldAttributesCache + this->bgmapIndex), 
		sizeof(WorldAttributes) * (__TOTAL_LAYERS - (this->bgmapIndex)) >> 2
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
