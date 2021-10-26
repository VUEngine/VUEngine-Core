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

#include <ObjectSpriteContainer.h>
#include <Mem.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Camera.h>
#include <SpriteManager.h>
#include <VIPManager.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Sprite;
friend class Texture;
friend class ObjectSprite;
friend class VirtualNode;
friend class VirtualList;


static int32 _spt;
static int16 _objectIndex;
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
	this->hidden = false;
	this->visible = true;
	this->transparent = __TRANSPARENCY_NONE;
	this->positioned = true;
	this->lockSpritesLists = false;
	this->hideSprites = false;

	SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);
}

/**
 * Class destructor
 */
void ObjectSpriteContainer::destructor()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);

	ASSERT(this->objectSprites, "ObjectSpriteContainer::destructor: null objectSprites");

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		ObjectSprite::invalidateObjectSpriteContainer(node->data);
		delete node->data;
	}

	delete this->objectSprites;
	this->objectSprites = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add an ObjectSprite to this container
 *
 * @param objectSprite		Sprite to add
 */
bool ObjectSpriteContainer::registerSprite(ObjectSprite objectSprite)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");

	NM_ASSERT(!VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::registerSprite: already registered");

	if(objectSprite)
	{
		this->lockSpritesLists = true;

		VirtualList::pushBack(this->objectSprites, objectSprite);

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
	NM_ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::unregisterSprite: null found");

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
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			ObjectSprite::setPosition(objectSprite, &objectSprite->position);
		}
	}

	this->position.z = position->z;
}

/**
 * Sort the object sprites within this container according to their z coordinates
 *
 * @private
 */
void ObjectSpriteContainer::sortProgressively()
{
	VirtualNode node = this->objectSprites->head;

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
			if(nextSprite->position.z + nextSprite->displacement.z > sprite->position.z + sprite->displacement.z)
			{
				// swap nodes' data
				VirtualNode::swapData(node, nextNode);

				node = nextNode;
			}
		}
	}
}

#ifdef __TOOLS
void ObjectSpriteContainer::hideSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::hideForDebug(this);

	for(VirtualNode node = this->objectSprites->head; node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::showForDebug(objectSprite);
			ObjectSpriteContainer::showForDebug(this);
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
	ObjectSpriteContainer::showForDebug(this);

	for(VirtualNode node = this->objectSprites->head; node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::hideForDebug(objectSprite);
			continue;
		}

		ObjectSprite::showForDebug(objectSprite);
	}
}
#endif

void ObjectSpriteContainer::showForDebug()
{
	Base::showForDebug(this);
	this->hidden = false;
	this->positioned = true;
	this->hideSprites = false;
}

void ObjectSpriteContainer::hideForDebug()
{
	this->hidden = false;
	this->positioned = true;
	this->hideSprites = true;
}

/**
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
int16 ObjectSpriteContainer::doRender(int16 index __attribute__((unused)), bool evenFrame __attribute__((unused)))
{
	// Setup spt
	this->spt = _spt;
	_vipRegistersCache[_spt] = _objectIndex;

	this->firstObjectIndex = _objectIndex;

	_worldAttributesCache[index].head = this->head;

	if(!this->hideSprites)
	{
		for(VirtualNode node = this->objectSprites->head; node && 0 < _objectIndex; node = node->next)
		{
			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			// Saves on method calls quite a bit when there are lots of
			// sprites. Don't remove.
			if(objectSprite->hidden || !objectSprite->positioned)
			{
				continue;
			}

			if(objectSprite->transparent & evenFrame)
			{
				continue;
			}

			if(0 > _objectIndex - objectSprite->totalObjects)
			{
				break;
			}

			if(_objectIndex - objectSprite->totalObjects == ObjectSprite::render(objectSprite, _objectIndex - objectSprite->totalObjects, evenFrame))
			{
				_objectIndex -= objectSprite->totalObjects;
			}
		}
	}

	_objectIndex--;

	// Make sure that the rest of spt segments only run up to the last
	// used object index
	for(int32 i = _spt--; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}

	this->lastObjectIndex = _objectIndex;

	return index;
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
 * Add displacement to position
 *
 * @param displacement		2D position displacement
 */
void ObjectSpriteContainer::addDisplacement(const PixelVector* displacement)
{
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			Sprite::addDisplacement(node->data, displacement);
		}
	}
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
	Printing::hex(Printing::getInstance(), Sprite::getWorldHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "OBJECT   ", x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Segment:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->spt, x + 18, y++, NULL);
	Printing::text(Printing::getInstance(), "STP value:                ", x, y, NULL);
	Printing::int32(Printing::getInstance(), _vipRegisters[__SPT0 + this->spt], x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "HEAD:                   ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[this->index].head, x + 18, y, 4, NULL);
	Printing::text(Printing::getInstance(), "Total OBJs:            ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex - this->lastObjectIndex, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "OBJ index range:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ObjectSpriteContainer::getFirstObjectIndex(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "-", x  + 18 + Utilities::intLength(ObjectSpriteContainer::getFirstObjectIndex(this)), y, NULL);
	Printing::int32(Printing::getInstance(), ObjectSpriteContainer::getLastObjectIndex(this), x  + 18 + Utilities::intLength(ObjectSpriteContainer::getFirstObjectIndex(this)) + 1, y, NULL);
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


/**
 * Set Sprite's render mode
 *
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSpriteContainer::setMode(uint16 display __attribute__ ((unused)), uint16 mode __attribute__ ((unused)))
{}

/**
 * Write textures
 *
 * @return			true it all textures are written
 */
bool ObjectSpriteContainer::writeTextures()
{
	if(!isDeleted(this->objectSprites))
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			ObjectSprite::writeTextures(ObjectSprite::safeCast(node->data));
		}
	}

	return true;
}

static void ObjectSpriteContainer::prepareForRendering()
{
	// clear OBJ memory
	for(int32 i = _objectIndex; i < __AVAILABLE_CHAR_OBJECTS; i++)
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

static void ObjectSpriteContainer::writeDRAM()
{
	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegisters[__SPT0 + i] = _vipRegistersCache[i] - _objectIndex;
	}

	Mem::copyWORD((WORD*)(_objectAttributesBaseAddress), (WORD*)(_objectAttributesCache + _objectIndex), sizeof(ObjectAttributes) * (__AVAILABLE_CHAR_OBJECTS - _objectIndex) >> 2);
}
