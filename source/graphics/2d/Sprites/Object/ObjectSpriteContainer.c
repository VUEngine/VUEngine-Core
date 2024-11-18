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


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class ObjectSprite;
friend class Sprite;
friend class Texture;
friend class VirtualList;
friend class VirtualNode;


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

static int32 _spt = __TOTAL_OBJECT_SEGMENTS - 1;
static int16 _objectIndex = __TOTAL_OBJECTS - 1;
static int16 _previousObjectIndex = __TOTAL_OBJECTS - 1;
static uint16 _vipRegistersCache[__TOTAL_OBJECT_SEGMENTS];


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
static void ObjectSpriteContainer::prepareForRendering()
{
	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __TOTAL_OBJECTS - 1;
}
//---------------------------------------------------------------------------------------------------------
static void ObjectSpriteContainer::finishRendering()
{
	// clear OBJ memory
	for(int32 i = _objectIndex; _previousObjectIndex <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_previousObjectIndex = _objectIndex;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::constructor()
{
	Base::constructor(NULL, NULL);

	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->head &= ~__WORLD_END;
	this->spt = 0;
	this->firstObjectIndex = 0;
	this->lastObjectIndex = 0;
	this->objectSprites = new VirtualList();
	this->transparency = __TRANSPARENCY_NONE;
	this->hideSprites = false;
	this->sortingSpriteNode = NULL;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::registerWithManager()
{
	SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::unregisterWithManager()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
int16 ObjectSpriteContainer::doRender(int16 index)
{
	this->index = index;

	return index;
}
//---------------------------------------------------------------------------------------------------------
int32 ObjectSpriteContainer::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return (this->firstObjectIndex - this->lastObjectIndex) * 8 * 8;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::invalidateRendering()
{
	for(VirtualNode node = this->objectSprites->tail; NULL != node; node = node->previous)
	{
		ObjectSprite::invalidateRendering(ObjectSprite::safeCast(node->data));
	}
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::forceShow()
{
	Base::forceShow(this);
	this->show = __HIDE;
	this->hideSprites = false;
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::forceHide()
{
	this->show = __SHOW;
	this->hideSprites = true;
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->index, x + 18, y, NULL);
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
	Printing::int32(Printing::getInstance(), this->firstObjectIndex, x  + 18 + Math::getDigitsCount(this->firstObjectIndex) + 1, y, NULL);
	Printing::text(Printing::getInstance(), "Z Position: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->position.z, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Pixels: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ObjectSpriteContainer::getTotalPixels(this), x + 18, y, NULL);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::unregisterSprite(ObjectSprite objectSprite)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::unregisterSprite: null objectSprite");
#ifndef __ENABLE_PROFILER
	NM_ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::unregisterSprite: null found");
#endif

	this->sortingSpriteNode = NULL;

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeData(this->objectSprites, objectSprite);
}
//---------------------------------------------------------------------------------------------------------
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
				break;
			}
		}
	}
	while(complete && swapped);

	if(!complete)
	{
		this->sortingSpriteNode = this->sortingSpriteNode->next;
	}

	return swapped;
}
//---------------------------------------------------------------------------------------------------------
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
			if(__HIDE == objectSprite->show || (objectSprite->transparency & evenFrame) || (0 > _objectIndex - objectSprite->totalObjects))
			{
				NM_ASSERT(0 < _objectIndex - objectSprite->totalObjects, "ObjectSpriteContainer::renderSprites: OBJECTS depleted");
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
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::showSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::forceShow(this);

	for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::forceHide(objectSprite);
			continue;
		}

		ObjectSprite::forceShow(objectSprite);
	}
}
//---------------------------------------------------------------------------------------------------------
void ObjectSpriteContainer::hideSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::forceHide(this);

	for(VirtualNode node = this->objectSprites->head; NULL != node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::forceShow(objectSprite);
			ObjectSpriteContainer::forceShow(this);
			continue;
		}

		ObjectSprite::forceHide(objectSprite);
	}
}
//---------------------------------------------------------------------------------------------------------
int32 ObjectSpriteContainer::getTotalUsedObjects()
{
	return this->firstObjectIndex - this->lastObjectIndex;
}
//---------------------------------------------------------------------------------------------------------
