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
#include <DebugUtilities.h>
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
	this->lockSpritesLists = false;
	this->hideSprites = false;
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
	ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");

#ifndef __ENABLE_PROFILER
	NM_ASSERT(!VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::registerSprite: already registered");
#endif

	if(!isDeleted(objectSprite))
	{
		this->lockSpritesLists = true;

		VirtualList::pushFront(this->objectSprites, objectSprite);

		this->lockSpritesLists = false;

		return true;
	}

	NM_ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");
	return false;
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

	this->lockSpritesLists = true;

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);

	this->lockSpritesLists = false;
}

/**
 * Set 2D position
 *
 * @param position		New 2D position
 */
void ObjectSpriteContainer::setPosition(const PixelVector* position)
{
	if(this->objectSprites)
	{
		for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
		{
			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			ObjectSprite::setPosition(objectSprite, position);
		}
	}

	this->center.z = position->z;
}

/**
 * Sort the object sprites within this container according to their z coordinates
 *
 * @private
 */
bool ObjectSpriteContainer::sortProgressively(bool deferred)
{
	if(this->lockSpritesLists)
	{
		return false;
	}

	bool swapped = false;

	for(VirtualNode node = this->objectSprites->head; NULL != node && NULL != node->next; node = node->next)
	{
		VirtualNode nextNode = node->next;

		NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::sortProgressively: NULL node's data");
		ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: node's data isn't a sprite");

		Sprite sprite = Sprite::safeCast(node->data);

		NM_ASSERT(!isDeleted(nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data");
		ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data cast");

		Sprite nextSprite = Sprite::safeCast(nextNode->data);

		// check if z positions are swapped
		if(nextSprite->center.z + nextSprite->displacement.z > sprite->center.z + sprite->displacement.z)
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
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
int16 ObjectSpriteContainer::doRender(int16 index, bool evenFrame __attribute__((unused)))
{
	this->index = index;
	this->renderFlag = true;

	return index;
}

void ObjectSpriteContainer::renderSprites(bool evenFrame)
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
			if(ObjectSprite::render(objectSprite, _objectIndex - (objectSprite->totalObjects - 1), evenFrame) == _objectIndex - (objectSprite->totalObjects - 1))
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
	Printing::text(Printing::getInstance(), "-", x  + 18 + Utilities::getDigitsCount(this->firstObjectIndex), y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex, x  + 18 + Utilities::getDigitsCount(ObjectSpriteContainer::getFirstObjectIndex(this)) + 1, y, NULL);
	Printing::text(Printing::getInstance(), "Z Position: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->center.z, x + 18, y, NULL);
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

/**
 * Write textures
 *
 * @return			true it all textures are written
 */
bool ObjectSpriteContainer::writeTextures(int16 maximumTextureRowsToWrite)
{
	if(!isDeleted(this->objectSprites))
	{
		for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
		{
			ObjectSprite::writeTextures(ObjectSprite::safeCast(node->data), maximumTextureRowsToWrite);
		}
	}

	return true;
}

static void ObjectSpriteContainer::reset()
{
	for(int32 i = __AVAILABLE_CHAR_OBJECTS - 1; 0 <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __AVAILABLE_CHAR_OBJECTS - 1;

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}
}

static void ObjectSpriteContainer::prepareForRendering()
{
	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __AVAILABLE_CHAR_OBJECTS - 1;

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}
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
	Mem::copyWORD((WORD*)(_objectAttributesBaseAddress), (WORD*)(_objectAttributesCache + _objectIndex), sizeof(ObjectAttributes) * (__AVAILABLE_CHAR_OBJECTS - _objectIndex) >> 2);

#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenObjectTiles;
	_writtenObjectTiles = __AVAILABLE_CHAR_OBJECTS - _objectIndex;
#endif
}

/**
 * Invalidate render flag on all sprites
 *
 */
void ObjectSpriteContainer::invalidateRenderFlag()
{
	for(VirtualNode node = this->objectSprites->tail; NULL != node; node = node->previous)
	{
		ObjectSprite::invalidateRenderFlag(ObjectSprite::safeCast(node->data));
	}
}