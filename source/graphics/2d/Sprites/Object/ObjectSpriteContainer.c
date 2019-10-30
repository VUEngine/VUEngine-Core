/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
	this->spt = spt;
	this->totalObjects = totalObjects;
	this->availableObjects = this->totalObjects;
	this->firstObjectIndex = firstObjectIndex;
	this->objectSprites = new VirtualList();
	this->objectSpriteNodeToDefragment = NULL;
	this->freedObjectIndex = 0;
	this->removingObjectSprite = false;
	this->hidden = false;
	this->visible = true;
	this->transparent = __TRANSPARENCY_NONE;

	this->node = NULL;
	this->previousNode = NULL;

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
s32 ObjectSpriteContainer::addObjectSprite(ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");

	if(objectSprite)
	{
		s32 lastObjectIndex = this->firstObjectIndex;

		if(VirtualList::getSize(this->objectSprites))
		{
			ObjectSprite lastObjectSprite = ObjectSprite::safeCast(VirtualList::back(this->objectSprites));

			ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");

			lastObjectIndex = lastObjectSprite->objectIndex;
			lastObjectIndex += lastObjectSprite->totalObjects;
		}

		VirtualList::pushBack(this->objectSprites, objectSprite);

		this->availableObjects -= numberOfObjects;

		this->node = NULL;
		this->previousNode = NULL;

		return lastObjectIndex;
	}

	return -1;
}

/**
 * Remove a previously registered ObjectSprite
 *
 * @param objectSprite		Sprite to remove
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
void ObjectSpriteContainer::removeObjectSprite(ObjectSprite objectSprite, s32 numberOfObjects)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::removeObjectSprite: not objectSprite");
	ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: not found");

	this->removingObjectSprite = true;

	// hide it immdiately
	if(0 <= objectSprite->objectIndex)
	{
		int i = 0;
		for(; i < objectSprite->totalObjects; i++)
		{
			_objectAttributesBaseAddress[((objectSprite->objectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
		}
	}

	if(this->objectSpriteNodeToDefragment)
	{
		int objectSpritePosition = VirtualList::getDataPosition(this->objectSprites, objectSprite);
		int objectSpriteNodeToDefragmentPosition = VirtualList::getNodePosition(this->objectSprites, this->objectSpriteNodeToDefragment);
		ASSERT(0 <= objectSpriteNodeToDefragmentPosition, "ObjectSpriteContainer::removeObjectSprite: position not found");

		if(0 <= objectSpriteNodeToDefragmentPosition && objectSpritePosition <= objectSpriteNodeToDefragmentPosition)
		{
			this->objectSpriteNodeToDefragment = VirtualList::find(this->objectSprites, objectSprite);
			this->freedObjectIndex = objectSprite->objectIndex;

			ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteNodeToDefragment");

			// move forward before deframenting
			this->objectSpriteNodeToDefragment = this->objectSpriteNodeToDefragment->next;
		}
	}
	else
	{
		// find the node to remove to defragment object memory
		this->objectSpriteNodeToDefragment = VirtualList::find(this->objectSprites, objectSprite);
		this->freedObjectIndex = objectSprite->objectIndex;

		ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteNodeToDefragment");

		// move forward before deframenting
		this->objectSpriteNodeToDefragment = this->objectSpriteNodeToDefragment->next;
	}

	// remove the sprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);

	this->node = this->previousNode = NULL;

	// if was the last node
	if(!this->objectSpriteNodeToDefragment | !this->objectSprites->head)
	{
		// just update the measures
		this->objectSpriteNodeToDefragment = NULL;
		this->availableObjects += numberOfObjects;
		this->freedObjectIndex = 0;
	}

	ASSERT(!this->objectSpriteNodeToDefragment || !isDeleted(VirtualNode::getData(this->objectSpriteNodeToDefragment)), "ObjectSpriteContainer::removeObjectSprite: deleted objectSpriteNodeToDefragment data");

	this->removingObjectSprite = false;
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
			Sprite sprite = Sprite::safeCast(node->data);

			Sprite::setPosition(sprite, &sprite->position);
		}
	}

	this->position.z = position->z;
}

/**
 * Defragment the ObjectSpriteContainer's OBJECT segment
 *
 * @private
 */
void ObjectSpriteContainer::defragment()
{
	ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::defragment: null objectSpriteNodeToDefragment");
	NM_ASSERT(!isDeleted(VirtualNode::getData(this->objectSpriteNodeToDefragment)), "ObjectSpriteContainer::defragment: deleted objectSpriteNodeToDefragment data");

	// get the next sprite to move
	ObjectSprite objectSprite = ObjectSprite::safeCast(VirtualNode::getData(this->objectSpriteNodeToDefragment));

	ASSERT(Sprite::getTexture(objectSprite), "ObjectSpriteContainer::defragment: null texture");

	// move sprite back
	ObjectSprite::setObjectIndex(objectSprite, this->freedObjectIndex);

	// set new index to the end of the current sprite
	this->freedObjectIndex += objectSprite->totalObjects;

	// move to the next sprite to move
	this->objectSpriteNodeToDefragment = this->objectSpriteNodeToDefragment->next;

	if(!this->objectSpriteNodeToDefragment)
	{
		this->freedObjectIndex = 0;

		VirtualNode node = this->objectSprites->tail;

		if(node)
		{
			ObjectSprite lastObjectSprite = ObjectSprite::safeCast(node->data);
			this->availableObjects = this->totalObjects - (-this->firstObjectIndex + lastObjectSprite->objectIndex + lastObjectSprite->totalObjects);
		}
		else
		{
			this->availableObjects = this->totalObjects;
		}
	}
}

/**
 * Sort the object sprites within this container according to their z coordinates
 *
 * @private
 */
void ObjectSpriteContainer::sortProgressively()
{
	this->node = this->node ? this->previousNode ? this->node : VirtualNode::getPrevious(this->node) : this->objectSprites->tail;

	this->previousNode = VirtualNode::getPrevious(this->node);

	if(this->node)
	{
		if(this->previousNode)
		{
			ObjectSprite sprite = ObjectSprite::safeCast(VirtualNode::getData(this->node));
			ObjectSprite previousSprite = ObjectSprite::safeCast(VirtualNode::getData(this->previousNode));

			// check if z positions are swapped
			if(previousSprite->position.z + (Sprite::safeCast(previousSprite))->displacement.z > sprite->position.z + (Sprite::safeCast(sprite))->displacement.z)
			{
				if(this->availableObjects >= sprite->totalObjects)
				{
					// swap
					s16 previousObjectIndex = previousSprite->objectIndex;

					ObjectSprite lastObjectSprite = ObjectSprite::safeCast(VirtualList::back(this->objectSprites));
					s16 nextFreeObjectIndex = lastObjectSprite->objectIndex + lastObjectSprite->totalObjects;

					ObjectSprite::setObjectIndex(sprite, previousObjectIndex);
					ObjectSprite::setObjectIndex(previousSprite, previousObjectIndex + sprite->totalObjects);

					int i = 0;
					for(; i < sprite->totalObjects; i++)
					{
						_objectAttributesBaseAddress[((nextFreeObjectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
					}

					// swap array entries
					VirtualNode::swapData(this->node, this->previousNode);

					this->node = this->previousNode;
				}
			}
		}

		this->node = VirtualNode::getPrevious(this->node);
	}
}

/**
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
void ObjectSpriteContainer::render(bool evenFrame, const PixelVector* displacement)
{
	// if render flag is set
	if(!this->worldLayer)
	{
		return;
	}

	_worldAttributesBaseAddress[this->worldLayer].head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
#ifdef __PROFILE_GAME
	_worldAttributesBaseAddress[this->worldLayer].w = __SCREEN_WIDTH;
	_worldAttributesBaseAddress[this->worldLayer].h = __SCREEN_HEIGHT;
#endif

	// defragmentation takes priority over z sorting
	if(!this->removingObjectSprite && this->objectSpriteNodeToDefragment)
	{
		ObjectSpriteContainer::defragment(this);
	}
	else
	{
		ObjectSpriteContainer::sortProgressively(this);
	}

	PixelVector finalDisplacement = this->displacement;

	if(displacement)
	{
		finalDisplacement = PixelVector::sum(finalDisplacement, *displacement);
	}

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		ObjectSprite sprite = ObjectSprite::safeCast(node->data);

		if(sprite->hidden || sprite->disposed)
		{
			if(!sprite->didHide)
			{
				int i = 0;
				for(; i < sprite->totalObjects; i++)
				{
					_objectAttributesBaseAddress[((sprite->objectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
				}

				sprite->didHide = true;
			}
		}
		else
		{
			if((u32)sprite->animationController)
			{
				Sprite::update(sprite);
			}

			Sprite::render(sprite, evenFrame, &finalDisplacement);

			if(!sprite->visible)
			{
				int i = 0;
				for(; i < sprite->totalObjects; i++)
				{
					_objectAttributesBaseAddress[((sprite->objectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
				}
			}
		}
	}
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
	if(this->objectSprites->head)
	{
		ObjectSprite lastObjectSprite = ObjectSprite::safeCast(VirtualList::back(this->objectSprites));

		ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");

		return lastObjectSprite->objectIndex + lastObjectSprite->totalObjects;
	}

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
	Printing::int(Printing::getInstance(), this->worldLayer, x + 18, y, NULL);
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
	Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[this->worldLayer].head, x + 18, y, 4, NULL);
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
	if(0 <= (s8)this->worldLayer)
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
