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

#include <CharSetManager.h>
#include <Clock.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <ParamTableManager.h>
#include <Printer.h>
#include <Sprite.h>
#include <TextureManager.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <DisplayUnit.h>

#include "SpriteManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Sprite;
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

#ifdef __DEBUGGING_SPRITES
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
	this->specialEffectsRowsPerFrame = -1;
	this->animationsClock = NULL;
	this->deferTextureUpdating = false;
	this->texturesMaximumRowsToWrite = -1;
	this->completeSort = true;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->specialSprites = new VirtualList();

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		this->spriteRegistry[i].sprites = NULL;
		this->spriteRegistry[i].sortingNode = NULL;
		this->spriteRegistry[i].availableSlots = 0;
		this->spriteRegistry[i].nextSlotIndex = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::destructor()
{
	SpriteManager::stopListeningForVBlank(this);

	if(!isDeleted(this->specialSprites))
	{
		delete this->specialSprites;
		this->specialSprites = NULL;
	}

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		if(!isDeleted(this->spriteRegistry[i].sprites))
		{
			delete this->spriteRegistry[i].sprites;
			this->spriteRegistry[i].sprites = NULL;
		}

		this->spriteRegistry[i].sortingNode = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventDisplayUnitVBlank:
		{
			SpriteManager::commitGraphics(this);
		
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

	CharSetManager::clearGraphicMemory(CharSetManager::getInstance());
	TextureManager::clearGraphicMemory();
	DisplayUnit::clearGraphicMemory();
	Printer::reset(Printer::getInstance());
	CharSetManager::reset(CharSetManager::getInstance());
	ParamTableManager::reset(ParamTableManager::getInstance());

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		NM_ASSERT(NULL == this->spriteRegistry[i].sprites, "SpriteManager::enable: invalid sprites list");
		this->spriteRegistry[i].sprites = new VirtualList();
		this->spriteRegistry[i].sortingNode = NULL;
		this->spriteRegistry[i].availableSlots = 0;
		this->spriteRegistry[i].nextSlotIndex = 0;
	}

	this->completeSort = true;
	this->evenFrame = __TRANSPARENCY_EVEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::disable()
{
	SpriteManager::stopListeningForVBlank(this);

	Base::disable(this);

	if(!isDeleted(this->specialSprites))
	{
		VirtualList::clear(this->specialSprites);
	}

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		if(!isDeleted(this->spriteRegistry[i].sprites))
		{
			VirtualList::clear(this->spriteRegistry[i].sprites);
			delete this->spriteRegistry[i].sprites;
		}

		this->spriteRegistry[i].sprites = NULL;
		this->spriteRegistry[i].sortingNode = NULL;
	}

	SpriteManager::destroyAllComponents(this);

	DisplayUnit::disableRendering();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::create(Entity owner, const SpriteSpec* spriteSpec)
{
	if(NULL == spriteSpec)
	{
		return NULL;
	}

	if(NULL == this->components->head)
	{
		SpriteManager::startListeningForVBlank(this);			
	}

	Sprite sprite = ((Sprite (*)(Entity, const SpriteSpec*)) ((ComponentSpec*)spriteSpec)->allocator)(owner, spriteSpec);

	Sprite::transform(sprite);

	int16 spriteListIndex = DisplayUnit::getSpriteListIndex(sprite);

	if(0 <= spriteListIndex && spriteListIndex < __TOTAL_SPRITE_LISTS)
	{
		SpriteManager::registerSprite(this, sprite, &this->spriteRegistry[spriteListIndex]);
	}

	if(Sprite::hasSpecialEffects(sprite))
	{
		if(!isDeleted(this->specialSprites))
		{
			VirtualList::pushBack(this->specialSprites, sprite);
		}
	}

	return sprite;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::purgeComponents()
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->deleteMe)
		{
			for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
			{
				if(NULL != this->spriteRegistry[i].sprites)
				{
					this->spriteRegistry[i].sortingNode = NULL;
					VirtualList::removeData(this->spriteRegistry[i].sprites, sprite);
					VirtualList::removeData(this->specialSprites, sprite);
				}
			}
		}
	}

	// Make sure that graphics pending updating are so now
	SpriteManager::writeTextures(this);

	Base::purgeComponents(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::configure(RenderingConfig renderingConfig, Clock animationsClock)
{
	SpriteManager::setTexturesMaximumRowsToWrite(this, renderingConfig.texturesMaximumRowsToWrite);
	SpriteManager::setSpecialEffectsRowsPerFrame(this, renderingConfig.specialEffectsRowsPerFrame);
	SpriteManager::setAnimationsClock(this, animationsClock);

	int16 availableSlots[__TOTAL_SPRITE_LISTS] = {0};
	const int16* nextSlotIndexes[__TOTAL_SPRITE_LISTS] = {NULL};
	
	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		nextSlotIndexes[i] = (const int16*)&this->spriteRegistry[i].nextSlotIndex;
	}

	DisplayUnit::fillAvailableSlots(availableSlots, nextSlotIndexes, __TOTAL_SPRITE_LISTS);

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
	{
		NM_ASSERT(0 < availableSlots[i], "SpriteManager::enable: invalid available sprite slots");
		NM_ASSERT(!isDeleted(this->spriteRegistry[i].sprites), "SpriteManager::enable: null sprites list");
		this->spriteRegistry[i].sortingNode = NULL;
		this->spriteRegistry[i].availableSlots = availableSlots[i];
		this->spriteRegistry[i].nextSlotIndex = availableSlots[i] - 1;
	}

	DisplayUnit::configure(renderingConfig.displayUnitConfig);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::setAnimationsClock(Clock clock)
{
	this->animationsClock = clock;
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
		SpriteManager::stopListeningForVBlank(this);			
	}

	if(Sprite::hasSpecialEffects(sprite))
	{
		if(!isDeleted(this->specialSprites))
		{
			VirtualList::removeData(this->specialSprites, sprite);
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
				spriteRegistry->sortingNode = VirtualList::insertBefore(spriteRegistry->sprites, node, sprite);
				return;
			}
		}

		spriteRegistry->sortingNode = VirtualList::pushBack(spriteRegistry->sprites, sprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::setSpecialEffectsRowsPerFrame(int32 specialEffectsRowsPerFrame)
{
	this->specialEffectsRowsPerFrame = specialEffectsRowsPerFrame;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 SpriteManager::getSpecialEffectsRowsPerFrame()
{
	return this->deferSpecialEffectsProcessing ? this->specialEffectsRowsPerFrame : -1;
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

void SpriteManager::deferSpecialEffectsProcessing(bool deferSpecialEffectsProcessing)
{
	this->deferSpecialEffectsProcessing = deferSpecialEffectsProcessing;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::sortSprites()
{
	while(SpriteManager::sortProgressively(this, true));

	this->completeSort = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::render()
{
#ifdef __DEBUGGING_SPRITES
	_renderedSprites = 0;
#endif

	Hardware::suspendInterrupts();
	DisplayUnit::startRendering();

	// Deframent video RAM
	CharSetManager::defragment(CharSetManager::getInstance(), true);

	this->completeSort = SpriteManager::sortProgressively(this, this->completeSort);

	// Switch between even and odd frame
	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	bool updateAnimations = true;
	
	if(!isDeleted(this->animationsClock))
	{
		updateAnimations = !Clock::isPaused(this->animationsClock);
	}

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS ; i++)
	{
		this->spriteRegistry[i].nextSlotIndex = this->spriteRegistry[i].availableSlots - 1;

		for(VirtualNode node = this->spriteRegistry[i].sprites->tail, previousNode = NULL; NULL != node; node = previousNode)
		{
			previousNode = node->previous;

			Sprite sprite = Sprite::safeCast(node->data);

			if(sprite->deleteMe)
			{
				this->spriteRegistry[i].sortingNode = NULL;
				VirtualList::removeNode(this->spriteRegistry[i].sprites, node);

				SpriteManager::destroySprite(this, sprite);
				continue;
			}

			// Saves on method calls quite a bit when there are lots of
			// Sprites. Don't remove.
			if(__HIDE == sprite->show || (sprite->transparency & this->evenFrame))
			{
				sprite->index = __NO_RENDER_INDEX;
				continue;
			}

			int16 usedSlots = Sprite::render(sprite, this->spriteRegistry[i].nextSlotIndex, updateAnimations);

			if(0 < usedSlots)
			{
				this->spriteRegistry[i].nextSlotIndex -= usedSlots;

#ifdef __ALERT_WORLD_MEMORY_DEPLETION
				if(0 > this->spriteRegistry[i].nextSlotIndex)
				{
					Printer::setDebugMode();
					Printer::clear();

					NM_ASSERT(false, "SpriteManager::render: slots depleted");		
				}
#endif
			}
		}
	}

	DisplayUnit::stopRendering();
	Hardware::resumeInterrupts();

#ifdef __DEBUGGING_SPRITES
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
	SpriteManager::commitGraphics(this);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::commitGraphics()
{
#ifdef __DEBUGGING_SPRITES
	_writtenTiles = 0;
	_writtenTextureTiles = 0;
	_writtenObjectTiles = 0;
#endif

	// Update graphics memory
	TextureManager::updateTextures(this->texturesMaximumRowsToWrite, this->deferTextureUpdating);
	SpriteManager::applySpecialEffects(this);

	DisplayUnit::commitGraphics();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::writeTextures()
{
	CharSetManager::writeCharSets(CharSetManager::getInstance());

	TextureManager::updateTextures(-1, false);

	CharSetManager::writeCharSets(CharSetManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::invalidateRendering()
{
	for(VirtualNode node = this->components->tail; NULL != node; node = node->previous)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::showSprites: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->deleteMe)
		{
			continue;
		}

		Sprite::invalidateRendering(sprite);
	}
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
	}

	if(showPrinting)
	{
		Printer::show(Printer::getInstance());
	}
	else
	{
		Printer::hide(Printer::getInstance());
	}
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

int32 SpriteManager::getNumberOfSprites()
{
	return VirtualList::getCount(this->components);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sprite SpriteManager::getSpriteAtIndex(int16 position)
{
	/*
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
	*/

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::print(int32 x, int32 y, bool resumed __attribute__((unused)))
{
	Printer::setWorldCoordinates(0, 0, Printer::getActiveSpritePosition().z, 0);

#ifndef __DEBUGGING_SPRITES
	SpriteManager::computeTotalPixelsDrawn(this);
#endif

	Printer::text("SPRITES USAGE", x, y++, NULL);
	Printer::text("Total pixels:                ", x, ++y, NULL);
	Printer::int32(this->totalPixelsDrawn, x + 22, y, NULL);
	Printer::text("Sprites count:              ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->components), x + 22, y, NULL);
#ifdef __DEBUGGING_SPRITES
	Printer::text("Rendered sprites:              ", x, ++y, NULL);
	Printer::int32(_renderedSprites, x + 22, y, NULL);
	Printer::text("Written chars:              ", x, ++y, NULL);
	Printer::int32(_writtenTiles, x + 22, y, NULL);
	Printer::text("Written texture tiles:              ", x, ++y, NULL);
	Printer::int32(_writtenTextureTiles, x + 22, y, NULL);
	Printer::text("Written object tiles:              ", x, ++y, NULL);
	Printer::int32(_writtenObjectTiles, x + 22, y, NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SpriteManager::sortProgressively(bool complete)
{
	bool swapped = false;

	for(int16 i = 0; i < __TOTAL_SPRITE_LISTS; i++)
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

					if(!complete)
					{
						this->spriteRegistry[i].sortingNode = this->spriteRegistry[i].sortingNode->next;
						break;
					}
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
	
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
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

void SpriteManager::applySpecialEffects()
{
	int32 specialEffectsRowsPerFrame = SpriteManager::getSpecialEffectsRowsPerFrame(this);

	for(VirtualNode node = this->specialSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::applySpecialEffects: NULL node's data");

		Sprite sprite = Sprite::safeCast(node->data);

		if(sprite->deleteMe || __HIDE == sprite->show)
		{
			continue;
		}

		Sprite::processEffects(sprite, specialEffectsRowsPerFrame);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::startListeningForVBlank()
{
	DisplayUnit::addEventListener(DisplayUnit::getInstance(), ListenerObject::safeCast(this), kEventDisplayUnitVBlank);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SpriteManager::stopListeningForVBlank()
{
	DisplayUnit::removeEventListener(DisplayUnit::getInstance(), ListenerObject::safeCast(this), kEventDisplayUnitVBlank);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
