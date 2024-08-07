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

#include <DebugConfig.h>
#include <Mem.h>
#include <ObjectSprite.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Printing.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "ObjectSpriteContainer.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class ObjectSprite;
friend class Sprite;
friend class Texture;
friend class VirtualList;
friend class VirtualNode;


static int32 _spt;
static int16 _objectIndex;
static int16 _previousObjectIndex;
static uint16 _vipRegistersCache[__TOTAL_OBJECT_SEGMENTS];

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 */
void ObjectSpriteContainer::constructor()
{
	Base::constructor(NULL, NULL);

	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->head &= ~__WORLD_END;
	this->spt = 0;
	this->firstObjectIndex = 0;
	this->lastObjectIndex = 0;
	this->objectSprites = new VirtualList();
	this->transparent = __TRANSPARENCY_NONE;
	this->hideSprites = false;
	this->sortingSpriteNode = NULL;
}

/**
 * Class destructor
 */
void ObjectSpriteContainer::destructor()
{
	ASSERT(this->objectSprites, "ObjectSpriteContainer::destructor: null objectSprites");

	if(!isDeleted(this->objectSprites))
	{
		VirtualList objectSprites = this->objectSprites;
		this->objectSprites = NULL;

		VirtualList::deleteData(objectSprites);
		delete objectSprites;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Register
 *
 */
void ObjectSpriteContainer::registerWithManager()
{
	SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);
}

/**
 * Unregister
 *
 */
void ObjectSpriteContainer::unregisterWithManager()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);
}

/**
 * Add an ObjectSprite to this container
 *
 * @param objectSprite		Sprite to add
 */
bool ObjectSpriteContainer::registerSprite(ObjectSprite objectSprite)
{
	for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
	{
		NM_ASSERT(!isDeleted(node->data), "SpriteManager::doRegisterSprite: NULL node's data");

		ObjectSprite otherSprite = ObjectSprite::safeCast(node->data);

		NM_ASSERT(otherSprite != objectSprite, "SpriteManager::doRegisterSprite: sprite already registered");

		if(otherSprite == objectSprite)
		{
			return false;
		}

		if(objectSprite->position.z + objectSprite->displacement.z > otherSprite->position.z + otherSprite->displacement.z)
		{
			this->sortingSpriteNode = VirtualList::insertAfter(this->objectSprites, node, objectSprite);
			return true;
		}
	}

	this->sortingSpriteNode = VirtualList::pushFront(this->objectSprites, objectSprite);

	return true;
}

/**
 * Remove a previously registered ObjectSprite
 *
 * @param objectSprite		Sprite to remove
 */
void ObjectSpriteContainer::unregisterSprite(ObjectSprite objectSprite)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::unregisterSprite: null objectSprite");
#ifndef __ENABLE_PROFILER
	NM_ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::unregisterSprite: null found");
#endif

	this->sortingSpriteNode = NULL;

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);
}

/**
 * Sort the object sprites within this container according to their z coordinates
 *
 * @private
 */
bool ObjectSpriteContainer::sortProgressively(bool complete)
{
	if(NULL == this->sortingSpriteNode)
	{
		this->sortingSpriteNode = this->objectSprites->head;

		if(NULL == this->sortingSpriteNode)
		{
			return false;
		}
	}

	bool swapped = false;

	do
	{
		swapped = false;

		for(VirtualNode node = complete ? this->objectSprites->head : this->sortingSpriteNode; NULL != node && NULL != node->next; node = node->next)
		{
			VirtualNode nextNode = node->next;

			NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::sortProgressively: NULL node's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: node's data isn't a sprite");

			Sprite sprite = Sprite::safeCast(node->data);

			NM_ASSERT(!isDeleted(nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data cast");

			Sprite nextSprite = Sprite::safeCast(nextNode->data);

			// check if z positions are swapped
			if(nextSprite->position.z + nextSprite->displacement.z > sprite->position.z + sprite->displacement.z)
			{
				// swap nodes' data
				node->data = nextSprite;
				nextNode->data = sprite;

				node = nextNode;

				swapped = true;
			}

			if(!complete)
			{
				this->sortingSpriteNode = this->sortingSpriteNode->next;
				break;
			}
		}
	}
	while(complete && swapped);

	return swapped;
}

void ObjectSpriteContainer::hideSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::hideForDebug(this);

	for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::forceShow(objectSprite);
			ObjectSpriteContainer::forceShow(this);
			continue;
		}

		ObjectSprite::hideForDebug(objectSprite);
	}
}

/**
 * Show all WORLD layers
 */
void ObjectSpriteContainer::showSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::forceShow(this);

	for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::hideForDebug(objectSprite);
			continue;
		}

		ObjectSprite::forceShow(objectSprite);
	}
}

void ObjectSpriteContainer::forceShow()
{
	Base::forceShow(this);
	this->show = __HIDE;
	this->hideSprites = false;
}

void ObjectSpriteContainer::hideForDebug()
{
	this->show = __SHOW;
	this->hideSprites = true;
}

/**
 * Render
 *
 * @param index
 */
int16 ObjectSpriteContainer::doRender(int16 index)
{
	this->index = index;

	return index;
}

/**
 * Render child sprites
 *
 * @param evenFrame
 * @param updateAnimations
 */
void ObjectSpriteContainer::renderSprites(bool evenFrame, bool updateAnimations)
{
	// Setup spt
	this->spt = _spt;

	this->firstObjectIndex = _objectIndex;

	if(!this->hideSprites)
	{
		for(VirtualNode node = this->objectSprites->head; NULL != node && 0 < _objectIndex; node = node->next)
		{
			NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::renderSprites: NULL node's data");

			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			// Saves on method calls quite a bit when there are lots of
			// sprites. Don't remove.
			if(__HIDE == objectSprite->show || (objectSprite->transparent & evenFrame) || (0 > _objectIndex - objectSprite->totalObjects))
			{
				objectSprite->index = __NO_RENDER_INDEX;
				continue;
			}

			// Do not change the order of this condition, objectSprite->totalObjects may be modified during rendering
			// but calling ObjectSprite::getTotalObjects is too costly
			if(ObjectSprite::render(objectSprite, _objectIndex - (objectSprite->totalObjects - 1), updateAnimations) == _objectIndex - (objectSprite->totalObjects - 1))
			{
				_objectIndex -= objectSprite->totalObjects;
			}
		}
	}

	if(this->firstObjectIndex == _objectIndex)
	{
		_objectAttributesCache[_objectIndex].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
		_objectIndex--;

		_worldAttributesCache[this->index].head = __WORLD_OFF;
	}
	else
	{
		_worldAttributesCache[this->index].head = this->head;

		// Make sure that the rest of spt segments only run up to the last
		// used object index
		for(int32 i = _spt--; i--;)
		{
			_vipRegistersCache[i] = _objectIndex;
		}
	}

	this->lastObjectIndex = _objectIndex;
}

/**
 * Retrieve the number of used OBJECTs within the segment assigned to this container
 *
 * @return 		Number of used OBJECTs
 */
int32 ObjectSpriteContainer::getTotalUsedObjects()
{
	return this->firstObjectIndex - this->lastObjectIndex;
}

/**
 * Retrieve the index of the first OBJECT within the segment assigned to this container
 *
 * @return 		Index of the first OBJECT
 */
int32 ObjectSpriteContainer::getFirstObjectIndex()
{
	return this->firstObjectIndex;
}

/**
 * Retrieve the index of the last OBJECT within the segment assigned to this container
 *
 * @return 		Index of the last OBJECT
 */
int32 ObjectSpriteContainer::getLastObjectIndex()
{
	return this->firstObjectIndex + this->lastObjectIndex;
}

/**
 * Print the container's status
 *
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
void ObjectSpriteContainer::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), SpriteManager::getSpritePosition(SpriteManager::getInstance(), Sprite::safeCast(this)), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getEffectiveHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "OBJECT   ", x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Segment:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->spt, x + 18, y++, NULL);
	Printing::text(Printing::getInstance(), "SPT value:                ", x, y, NULL);
	Printing::int32(Printing::getInstance(), _vipRegisters[__SPT0 + this->spt], x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "HEAD:                   ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[this->index].head, x + 18, y, 4, NULL);
	Printing::text(Printing::getInstance(), "Total OBJs:            ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex - this->lastObjectIndex, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "OBJ index range:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastObjectIndex, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "-", x  + 18 + Math::getDigitsCount(this->firstObjectIndex), y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex, x  + 18 + Math::getDigitsCount(ObjectSpriteContainer::getFirstObjectIndex(this)) + 1, y, NULL);
	Printing::text(Printing::getInstance(), "Z Position: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->position.z, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Pixels: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ObjectSpriteContainer::getTotalPixels(this), x + 18, y, NULL);
}

int32 ObjectSpriteContainer::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return (this->firstObjectIndex - this->lastObjectIndex) * 8 * 8;
	}

	return 0;
}

static void ObjectSpriteContainer::reset()
{
	for(int32 i = __TOTAL_OBJECTS - 1; 0 <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __TOTAL_OBJECTS - 1;

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}
}

static void ObjectSpriteContainer::prepareForRendering()
{
	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __TOTAL_OBJECTS - 1;

}

static void ObjectSpriteContainer::finishRendering()
{
	// clear OBJ memory
	for(int32 i = _objectIndex; _previousObjectIndex <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_previousObjectIndex = _objectIndex;
}

static void ObjectSpriteContainer::writeDRAM()
{
	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegisters[__SPT0 + i] = _vipRegistersCache[i] - _objectIndex;
	}

	CACHE_RESET;
	Mem::copyWORD((WORD*)(_objectAttributesBaseAddress), (WORD*)(_objectAttributesCache + _objectIndex), sizeof(ObjectAttributes) * (__TOTAL_OBJECTS - _objectIndex) >> 2);

#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenObjectTiles;
	_writtenObjectTiles = __TOTAL_OBJECTS - _objectIndex;
#endif
}

/**
 * Invalidate render flag on all sprites
 *
 */
void ObjectSpriteContainer::invalidateRendering()
{
	for(VirtualNode node = this->objectSprites->tail; NULL != node; node = node->previous)
	{
		ObjectSprite::invalidateRendering(ObjectSprite::safeCast(node->data));
	}
}