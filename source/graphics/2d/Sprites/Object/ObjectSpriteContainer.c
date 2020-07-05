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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param spt					SPT assigned to this container
 * @param totalObjects			Total number of OBJECTs that manages by this container
 * @param firstObjectIndex		The index of the first OBJECT managed by this container
 */
void ObjectSpriteContainer::constructor(int spt, int totalObjects, int firstObjectIndex)
{
	ASSERT(0 <= spt && spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainer::constructor: bad spt");

	Base::constructor(NULL, NULL);

	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->head &= ~__WORLD_END;
	this->spt = spt;
	this->totalObjects = totalObjects;
	this->availableObjects = this->totalObjects;
	this->firstObjectIndex = firstObjectIndex;
	this->lastRenderedObjectIndex = this->firstObjectIndex + this->totalObjects;
	this->totalObjectsToWriteToDRAM = this->lastRenderedObjectIndex;
	this->objectSprites = new VirtualList();
	this->hidden = false;
	this->visible = true;
	this->transparent = __TRANSPARENCY_NONE;
	this->positioned = true;
	this->lockSpritesLists = false;

	// clear OBJ memory
	int i = firstObjectIndex;

	for(; i < this->firstObjectIndex + this->totalObjects; i++)
	{
		_objectAttributesCache[i].jx = 0;
		_objectAttributesCache[i].head = __OBJECT_CHAR_HIDE_MASK;
		_objectAttributesCache[i].jy = 0;
		_objectAttributesCache[i].tile = 0;
	}

	// must setup the STP registers regardless of the totalObjects
	_vipRegisters[__SPT0 + this->spt] = this->firstObjectIndex + this->totalObjects - 1;

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

	NM_ASSERT(0 == VirtualList::getSize(this->objectSprites), "ObjectSpriteContainer::destructor: sprites list not empty");

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
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
bool ObjectSpriteContainer::registerSprite(ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");

	NM_ASSERT(!VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::registerSprite: already registered");

	if(objectSprite && this->availableObjects >= numberOfObjects)
	{
		this->lockSpritesLists = true;

		VirtualList::pushBack(this->objectSprites, objectSprite);

		this->availableObjects -= numberOfObjects;

		this->lockSpritesLists = false;

		return true;
	}

	NM_ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");
	NM_ASSERT(this->availableObjects >= numberOfObjects, "ObjectSpriteContainer::registerSprite: not enough OBJECTS");
	return false;
}

/**
 * Remove a previously registered ObjectSprite
 *
 * @param objectSprite		Sprite to remove
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
void ObjectSpriteContainer::unregisterSprite(ObjectSprite objectSprite, s32 numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::unregisterSprite: null objectSprite");
	NM_ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::unregisterSprite: null found");

	this->lockSpritesLists = true;

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);

	this->availableObjects += numberOfObjects;

	this->lockSpritesLists = false;

	// Make sure to clean OBJECT memory next cycle	
	this->lastRenderedObjectIndex = this->firstObjectIndex + this->totalObjects;
}

/**
 * Check if this container has enough free OBJECTs
 *
 * @param numberOfObjects	The number of OBJECTs to check
 * @return 					True if there is enough OBJECT space in this container
 */
bool ObjectSpriteContainer::hasRoomFor(s32 numberOfObjects)
{
	return this->availableObjects >= numberOfObjects;
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
			if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
			{
				// swap nodes' data
				VirtualNode::swapData(node, nextNode);

				node = nextNode;
			}
		}
	}
}

void ObjectSpriteContainer::writeDRAM()
{
	Mem::copyWORD((WORD*)(_objectAttributesBaseAddress + this->firstObjectIndex), (WORD*)(_objectAttributesCache + this->firstObjectIndex), sizeof(ObjectAttributes) * (this->totalObjectsToWriteToDRAM) >> 2);
}

/**
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
u16 ObjectSpriteContainer::doRender(s16 index __attribute__((unused)), bool evenFrame __attribute__((unused)))
{
	_worldAttributesCache[index].head = this->head;

	u16 objectIndex = this->firstObjectIndex;

	for(VirtualNode node = this->objectSprites->head; node && objectIndex < this->firstObjectIndex + this->totalObjects; node = node->next)
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

		if(objectIndex == ObjectSprite::render(objectSprite, objectIndex, evenFrame))
		{
			objectIndex += objectSprite->totalObjects;
		}
	}

	u16 lastRenderedObjectIndex = objectIndex;

	for(; objectIndex < this->lastRenderedObjectIndex; objectIndex++)
	{
		_objectAttributesCache[objectIndex].head = __OBJECT_CHAR_HIDE_MASK;
	}


#ifdef __MEDNAFEN_HACK
	// totalObjectsToWriteToDRAM causes graphical glitches on Mednafnen but
	// works just fine on hardware.
	this->totalObjectsToWriteToDRAM = this->totalObjects;
#else
#ifndef __RELEASE
	// totalObjectsToWriteToDRAM causes graphical glitches on Mednafnen but
	// works just fine on hardware.
	this->totalObjectsToWriteToDRAM = this->totalObjects;
#else
	this->totalObjectsToWriteToDRAM = (lastRenderedObjectIndex > this->lastRenderedObjectIndex ? lastRenderedObjectIndex : this->lastRenderedObjectIndex) - this->firstObjectIndex;
#endif
#endif

	this->lastRenderedObjectIndex = lastRenderedObjectIndex;

	return index;
}

/**
 * Retrieve the number of free OBJECTs within the segment assigned to this container
 *
 * @return 		Number of free OBJECTs
 */
int ObjectSpriteContainer::getAvailableObjects()
{
	return this->availableObjects;
}

/**
 * Retrieve the number of used OBJECTs within the segment assigned to this container
 *
 * @return 		Number of used OBJECTs
 */
int ObjectSpriteContainer::getTotalUsedObjects()
{
	int totalUsedObjects = 0;
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			totalUsedObjects += (ObjectSprite::safeCast(node->data))->totalObjects;
		}
	}

	return totalUsedObjects;
}

/**
 * Retrieve the index of the next free OBJECT within the segment assigned to this container
 *
 * @return 		Index of the next free OBJECT
 */
int ObjectSpriteContainer::getNextFreeObjectIndex()
{
	return 0;
}

/**
 * Retrieve the index of the first OBJECT within the segment assigned to this container
 *
 * @return 		Index of the first OBJECT
 */
int ObjectSpriteContainer::getFirstObjectIndex()
{
	return this->firstObjectIndex;
}

/**
 * Retrieve the index of the last OBJECT within the segment assigned to this container
 *
 * @return 		Index of the last OBJECT
 */
int ObjectSpriteContainer::getLastObjectIndex()
{
	return this->firstObjectIndex + this->totalObjects;
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
void ObjectSpriteContainer::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), SpriteManager::getSpritePosition(SpriteManager::getInstance(), Sprite::safeCast(this)), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getWorldHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "OBJECT   ", x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Segment:                ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->spt, x + 18, y++, NULL);
	Printing::text(Printing::getInstance(), "STP value:                ", x, y, NULL);
	Printing::int(Printing::getInstance(), _vipRegisters[__SPT0 + this->spt], x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "HEAD:                   ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[this->index].head, x + 18, y, 4, NULL);
	Printing::text(Printing::getInstance(), "Total OBJs:           ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->totalObjects, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Available OBJs:       ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getAvailableObjects(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Total used OBJs:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getTotalUsedObjects(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Next free OBJ:  ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getNextFreeObjectIndex(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "OBJ index range:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getFirstObjectIndex(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "-", x  + 18 + Utilities::intLength(ObjectSpriteContainer::getFirstObjectIndex(this)), y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getLastObjectIndex(this), x  + 18 + Utilities::intLength(ObjectSpriteContainer::getFirstObjectIndex(this)) + 1, y, NULL);
	Printing::text(Printing::getInstance(), "Z Position: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->position.z, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Pixels: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), ObjectSpriteContainer::getTotalPixels(this), x + 18, y, NULL);
}

int ObjectSpriteContainer::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return ObjectSpriteContainer::getAvailableObjects(this) * 8 * 8;
	}

	return 0;
}


/**
 * Set Sprite's render mode
 *
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSpriteContainer::setMode(u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
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
