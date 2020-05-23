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
	this->lastRenderedObjectIndex = this->firstObjectIndex;
	this->objectSprites = new VirtualList();
	this->hidden = false;
	this->visible = true;
	this->transparent = __TRANSPARENCY_NONE;

	// clear OBJ memory
	int i = firstObjectIndex;

	for(; i < this->firstObjectIndex + this->totalObjects; i++)
	{
		_objectAttributesBaseAddress[(i << 2) + 0] = 0;
		_objectAttributesBaseAddress[(i << 2) + 1] = 0;
		_objectAttributesBaseAddress[(i << 2) + 2] = 0;
		_objectAttributesBaseAddress[(i << 2) + 3] = 0;
	}

	// must setup the STP registers regardless of the totalObjects
	_vipRegisters[__SPT0 + this->spt] = this->firstObjectIndex + this->totalObjects - 1;
}

/**
 * Class destructor
 */
void ObjectSpriteContainer::destructor()
{
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
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
bool ObjectSpriteContainer::addObjectSprite(ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");

	if(objectSprite && this->availableObjects >= numberOfObjects)
	{

		VirtualList::pushBack(this->objectSprites, objectSprite);

		this->availableObjects -= numberOfObjects;

		return true;
	}

	NM_ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");
	NM_ASSERT(this->availableObjects >= numberOfObjects, "ObjectSpriteContainer::addObjectSprite: not enough OBJECTS");
	return false;
}

/**
 * Remove a previously registered ObjectSprite
 *
 * @param objectSprite		Sprite to remove
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
void ObjectSpriteContainer::removeObjectSprite(ObjectSprite objectSprite, s32 numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::removeObjectSprite: null objectSprite");
	ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: null found");

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);

	this->availableObjects += numberOfObjects;
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
	VirtualNode node = this->objectSprites->tail;

	for(; node; node = node->previous)
	{
		VirtualNode previousNode = node->previous;

		if(previousNode)
		{
			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);
			ObjectSprite previousSprite = ObjectSprite::safeCast(previousNode->data);

			// check if z positions are swapped
			if(previousSprite->position.z + (Sprite::safeCast(previousSprite))->displacement.z > objectSprite->position.z + (Sprite::safeCast(objectSprite))->displacement.z)
			{
				if(this->availableObjects >= objectSprite->totalObjects)
				{
					VirtualNode::swapData(node, previousNode);

					node = previousNode;
				}
			}
		}
	}
}

/**
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
bool ObjectSpriteContainer::render(u16 index, bool evenFrame)
{
	// if render flag is set
	this->index = index;

	_worldAttributesBaseAddress[this->index].head = this->head;
#ifdef __PROFILE_GAME
	_worldAttributesBaseAddress[this->index].w = __SCREEN_WIDTH;
	_worldAttributesBaseAddress[this->index].h = __SCREEN_HEIGHT;
#endif

	if(!VIPManager::hasFrameStarted(VIPManager::getInstance()))
	{
		ObjectSpriteContainer::sortProgressively(this);
	}

	VirtualNode node = this->objectSprites->head;

	u16 objectIndex = this->firstObjectIndex;

	for(; node && objectIndex < this->firstObjectIndex + this->totalObjects; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite->hidden | objectSprite->disposed)
		{
			continue;
		}
		else
		{
			if((u32)objectSprite->animationController)
			{
				ObjectSprite::update(objectSprite);
			}

			if(ObjectSprite::render(objectSprite, objectIndex, evenFrame))
			{
				objectIndex += objectSprite->totalObjects;
			}
		}
	}

	u16 lastRenderedObjectIndex = objectIndex;

	for(; objectIndex < this->lastRenderedObjectIndex; objectIndex++)
	{
		_objectAttributesBaseAddress[((objectIndex) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
	}

	this->lastRenderedObjectIndex = lastRenderedObjectIndex;

	return true;
}

/**
 * Show
 */
void ObjectSpriteContainer::show()
{
	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		Sprite::show(node->data);
	}

	this->hidden = false;
}

/**
 * Hide
 */
void ObjectSpriteContainer::hide()
{
	// must check list, because the Sprite's destructor calls this method
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			Sprite::hide(node->data);
		}
	}

	// I can never be hidden, otherwise the OBJ rendering gets messed up
	this->hidden = false;
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
	Printing::text(Printing::getInstance(), "Layer: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->index, x + 18, y, NULL);
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
	if(0 <= (s8)this->index)
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
	return true;
}

/**
 * Check if all textures are written
 *
 * @return			true it all textures are written
 */
bool ObjectSpriteContainer::areTexturesWritten()
{
	return true;
}
