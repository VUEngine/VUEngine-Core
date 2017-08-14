/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <ObjectSpriteContainerManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>
#include <SpriteManager.h>
#include <VIPManager.h>
#include <Printing.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	ObjectSpriteContainer
 * @extends Sprite
 * @ingroup graphics-2d-sprites-object
 */
__CLASS_DEFINITION(ObjectSpriteContainer, Sprite);
__CLASS_FRIEND_DEFINITION(Sprite);
__CLASS_FRIEND_DEFINITION(Texture);
__CLASS_FRIEND_DEFINITION(ObjectSprite);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals

// external
void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this);

static void ObjectSpriteContainer_defragment(ObjectSpriteContainer this);
static void ObjectSpriteContainer_sortProgressively(ObjectSpriteContainer this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSpriteContainer, int spt, int totalObjects, int firstObjectIndex)
__CLASS_NEW_END(ObjectSpriteContainer, spt, totalObjects, firstObjectIndex);

/**
 * Class constructor
 *
 * @memberof					ObjectSpriteContainer
 * @public
 *
 * @param this					Function scope
 * @param spt					SPT assigned to this container
 * @param totalObjects			Total number of OBJECTs that manages by this container
 * @param firstObjectIndex		The index of the first OBJECT managed by this container
 */
void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, int spt, int totalObjects, int firstObjectIndex)
{
	ASSERT(this, "ObjectSpriteContainer::constructor: null this");
	ASSERT(0 <= spt && spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainer::constructor: bad spt");

	__CONSTRUCT_BASE(Sprite, NULL, NULL);

	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->spt = spt;
	this->totalObjects = totalObjects;
	this->availableObjects = this->totalObjects;
	this->firstObjectIndex = firstObjectIndex;
	this->objectSprites = __NEW(VirtualList);
	this->objectSpriteNodeToDefragment = NULL;
	this->freedObjectIndex = 0;
	this->z = 0;
	this->removingObjectSprite = false;
	this->hidden = false;
	this->visible = true;
	this->transparent = false;

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

	// only request a WORLD layer if can hold any OBJECT
	if(this->totalObjects)
	{
		// register to sprite manager
		SpriteManager_registerSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}
}

/**
 * Class destructor
 *
 * @memberof				ObjectSpriteContainer
 * @public
 *
 * @param this				Function scope
 */
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::destructor: null this");
	ASSERT(this->objectSprites, "ObjectSpriteContainer::destructor: null objectSprites");

	if(this->totalObjects && this->worldLayer)
	{
		// remove from sprite manager
		SpriteManager_unregisterSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		ObjectSprite_invalidateObjectSpriteContainer(__SAFE_CAST(ObjectSprite, node->data));
		__DELETE(node->data);
	}

	__DELETE(this->objectSprites);
	this->objectSprites = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Add an ObjectSprite to this container
 *
 * @memberof				ObjectSpriteContainer
 * @public
 *
 * @param this				Function scope
 * @param objectSprite		Sprite to add
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::addObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");

	if(objectSprite)
	{
		s32 lastObjectIndex = this->firstObjectIndex;

		if(VirtualList_getSize(this->objectSprites))
		{
			ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, VirtualList_back(this->objectSprites));

			ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");

			lastObjectIndex = lastObjectSprite->objectIndex;
			lastObjectIndex += lastObjectSprite->totalObjects;
		}

		VirtualList_pushBack(this->objectSprites, objectSprite);

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
 * @memberof				ObjectSpriteContainer
 * @public
 *
 * @param this				Function scope
 * @param objectSprite		Sprite to remove
 * @param numberOfObjects	The number of OBJECTs used by the Sprite
 */
void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s32 numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::removeObjectSprite: not objectSprite");
	ASSERT(VirtualList_find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: not found");

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
		int objectSpritePosition = VirtualList_getDataPosition(this->objectSprites, objectSprite);
		int objectSpriteNodeToDefragmentPosition = VirtualList_getNodePosition(this->objectSprites, this->objectSpriteNodeToDefragment);
		ASSERT(0 <= objectSpriteNodeToDefragmentPosition, "ObjectSpriteContainer::removeObjectSprite: position not found");

		if(0 <= objectSpriteNodeToDefragmentPosition && objectSpritePosition <= objectSpriteNodeToDefragmentPosition)
		{
			this->objectSpriteNodeToDefragment = VirtualList_find(this->objectSprites, objectSprite);
			this->freedObjectIndex = objectSprite->objectIndex;

			ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteNodeToDefragment");

			// move forward before deframenting
			this->objectSpriteNodeToDefragment = this->objectSpriteNodeToDefragment->next;
		}
	}
	else
	{
		// find the node to remove to defragment object memory
		this->objectSpriteNodeToDefragment = VirtualList_find(this->objectSprites, objectSprite);
		this->freedObjectIndex = objectSprite->objectIndex;

		ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteNodeToDefragment");

		// move forward before deframenting
		this->objectSpriteNodeToDefragment = this->objectSpriteNodeToDefragment->next;
	}

	// remove the sprite to prevent rendering afterwards
	VirtualList_removeElement(this->objectSprites, objectSprite);

	this->node = this->previousNode = NULL;

	// if was the last node
	if(!this->objectSpriteNodeToDefragment | !this->objectSprites->head)
	{
		// just update the measures
		this->objectSpriteNodeToDefragment = NULL;
		this->availableObjects += numberOfObjects;
		this->freedObjectIndex = 0;
	}

	ASSERT(!this->objectSpriteNodeToDefragment || __IS_OBJECT_ALIVE(VirtualNode_getData(this->objectSpriteNodeToDefragment)), "ObjectSpriteContainer::removeObjectSprite: deleted objectSpriteNodeToDefragment data");

	this->removingObjectSprite = false;
}

/**
 * Check if this container has enough free OBJECTs
 *
 * @memberof				ObjectSpriteContainer
 * @public
 *
 * @param this				Function scope
 * @param numberOfObjects	The number of OBJECTs to check
 *
 * @return 					True if there is enough OBJECT space in this container
 */
bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, s32 numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");

	return this->availableObjects >= numberOfObjects;
}

/**
 * Retrieve 2D position
 *
 * @memberof		ObjectSpriteContainer
 * @public
 *
 * @param this		Function scope
 *
 * @return			2D position
 */
VBVec2D ObjectSpriteContainer_getPosition(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getPosition: null this");

	VBVec2D position =
	{
		0, 0, 0, 0
	};

	position.z = this->z;

	return position;
}

/**
 * Set 2D position
 *
 * @memberof			ObjectSpriteContainer
 * @public
 *
 * @param this			Function scope
 * @param position		New 2D position
 */
void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, const VBVec2D* position)
{
	ASSERT(this, "ObjectSpriteContainer::setPosition: null this");

	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);

			VBVec2D spritePosition = __VIRTUAL_CALL(Sprite, getPosition, sprite);
			__VIRTUAL_CALL(Sprite, setPosition, sprite, &spritePosition);
		}
	}

	this->z = position->z;
}

/**
 * Defragment the ObjectSpriteContainer's OBJECT segment
 *
 * @memberof		ParamTableManager
 * @private
 *
 * @param this		Function scope
 */
static void ObjectSpriteContainer_defragment(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::defragment: null this");
	ASSERT(this->objectSpriteNodeToDefragment, "ObjectSpriteContainer::defragment: null objectSpriteNodeToDefragment");
	NM_ASSERT(__IS_OBJECT_ALIVE(VirtualNode_getData(this->objectSpriteNodeToDefragment)), "ObjectSpriteContainer::defragment: deleted objectSpriteNodeToDefragment data");

	// get the next sprite to move
	ObjectSprite objectSprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->objectSpriteNodeToDefragment));

	ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, objectSprite)), "ObjectSpriteContainer::defragment: null texture");

	// move sprite back
	ObjectSprite_setObjectIndex(objectSprite, this->freedObjectIndex);

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
			ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, node->data);
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
 * @memberof		ParamTableManager
 * @private
 *
 * @param this		Function scope
 */
static void ObjectSpriteContainer_sortProgressively(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::sort: null this");

	this->node = this->node ? this->previousNode ? this->node : VirtualNode_getPrevious(this->node) : this->objectSprites->tail;

	for(; this->node; )
	{
		this->previousNode = VirtualNode_getPrevious(this->node);

		if(this->previousNode)
		{
			ObjectSprite sprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->node));
			ObjectSprite previousSprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->previousNode));
			VBVec2D position = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, sprite));
			VBVec2D previousPosition = __VIRTUAL_CALL(Sprite, getPosition, __SAFE_CAST(Sprite, previousSprite));

			// check if z positions are swapped
			if(previousPosition.z + (__SAFE_CAST(Sprite, previousSprite))->displacement.z > position.z + (__SAFE_CAST(Sprite, sprite))->displacement.z)
			{
				if(this->availableObjects >= sprite->totalObjects)
				{
					// swap
					s16 previousObjectIndex = previousSprite->objectIndex;

					ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, VirtualList_back(this->objectSprites));
					s16 nextFreeObjectIndex = lastObjectSprite->objectIndex + lastObjectSprite->totalObjects;

//					ObjectSprite_setObjectIndex(previousSprite, nextFreeObjectIndex);
					ObjectSprite_setObjectIndex(sprite, previousObjectIndex);
					ObjectSprite_setObjectIndex(previousSprite, previousObjectIndex + sprite->totalObjects);

					int i = 0;
					for(; i < sprite->totalObjects; i++)
					{
						_objectAttributesBaseAddress[((nextFreeObjectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
					}

					// swap array entries
					VirtualNode_swapData(this->node, this->previousNode);

					this->node = this->previousNode;
					return;
				}
			}
		}

		this->node = VirtualNode_getPrevious(this->node);
		break;
	}
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		ObjectSpriteContainer
 * @public
 *
 * @param this		Function scope
 */
void ObjectSpriteContainer_render(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::render: null this");

	//if render flag is set
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
		ObjectSpriteContainer_defragment(this);
	}
	else
	{
		ObjectSpriteContainer_sortProgressively(this);
	}

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		ObjectSprite sprite = __SAFE_CAST(ObjectSprite, node->data);

		if(!sprite->hidden && ((sprite->texture && sprite->texture->written && sprite->animationController) || sprite->transparent))
		{
			Sprite_update(__SAFE_CAST(Sprite, sprite));
		}

		if((sprite->hidden | !sprite->visible) && 0 <= sprite->objectIndex)
		{
			int i = 0;
			for(; i < sprite->totalObjects; i++)
			{
				_objectAttributesBaseAddress[((sprite->objectIndex + i) << 2) + 1] = __OBJECT_CHAR_HIDE_MASK;
			}
		}
		else
		{
			__VIRTUAL_CALL(Sprite, render, sprite);
		}
	}
}

/**
 * Show
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 */
void ObjectSpriteContainer_show(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::show: null this");

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Sprite, show, __SAFE_CAST(Sprite, node->data));
	}

	this->hidden = false;
}

/**
 * Hide
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 */
void ObjectSpriteContainer_hide(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::hide: null this");

	// must check list, because the Sprite's destructor calls this method
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Sprite, hide, __SAFE_CAST(Sprite, node->data));
		}
	}

	// I can never be hidden, otherwise the OBJ rendering gets messed up
	this->hidden = false;
}

/**
 * Retrieve the number of free OBJECTs within the segment assigned to this container
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Number of free OBJECTs
 */
int ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");

	return this->availableObjects;
}

/**
 * Retrieve the number of used OBJECTs within the segment assigned to this container
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Number of used OBJECTs
 */
int ObjectSpriteContainer_getTotalUsedObjects(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getTotalUsedObjects: null this");

	int totalUsedObjects = 0;
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			totalUsedObjects += (__SAFE_CAST(ObjectSprite, node->data))->totalObjects;
		}
	}

	return totalUsedObjects;
}

/**
 * Retrieve the index of the next free OBJECT within the segment assigned to this container
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Index of the next free OBJECT
 */
int ObjectSpriteContainer_getNextFreeObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");

	if(this->objectSprites->head)
	{
		ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, VirtualList_back(this->objectSprites));

		ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");

		return lastObjectSprite->objectIndex + lastObjectSprite->totalObjects;
	}

	return 0;
}

/**
 * Retrieve the index of the first OBJECT within the segment assigned to this container
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Index of the first OBJECT
 */
int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");

	return this->firstObjectIndex;
}

/**
 * Retrieve the index of the last OBJECT within the segment assigned to this container
 *
 * @memberof	ObjectSpriteContainer
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Index of the last OBJECT
 */
int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");

	return this->firstObjectIndex + this->totalObjects;
}

/**
 * Add displacement to position
 *
 * @memberof				ObjectSpriteContainer
 * @public
 *
 * @param this				Function scope
 * @param displacement		2D position displacement
 */
void ObjectSpriteContainer_addDisplacement(ObjectSpriteContainer this, const VBVec2D* displacement)
{
	ASSERT(this, "BgmapSprite::addDisplacement: null this");

	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(Sprite, addDisplacement, __SAFE_CAST(Sprite, node->data), displacement);
		}
	}
}

/**
 * Print the container's status
 *
 * @memberof		ObjectSpriteContainer
 * @public
 *
 * @param this		Function scope
 * @param x			Screen x coordinate
 * @param y			Screen y coordinate
 */
void ObjectSpriteContainer_print(ObjectSpriteContainer this, int x, int y)
{
	ASSERT(this, "ObjectSpriteContainer::print: null this");

	Printing_text(Printing_getInstance(), "Segment:                ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->spt, x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "STP value:                ", x, y, NULL);
	Printing_int(Printing_getInstance(), _vipRegisters[__SPT0 + this->spt], x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "WORLD:                  ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->worldLayer, x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "HEAD:                   ", x, ++y, NULL);
	Printing_hex(Printing_getInstance(), _worldAttributesBaseAddress[this->worldLayer].head, x + 24, y, 4, NULL);
	Printing_text(Printing_getInstance(), "Total OBJECTs:           ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->totalObjects, x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Available OBJECTs:       ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getAvailableObjects(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Total used OBJECTs:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getTotalUsedObjects(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Next free OBJECT index:  ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getNextFreeObjectIndex(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Object index range:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getFirstObjectIndex(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "-", x  + 24 + Utilities_intLength(ObjectSpriteContainer_getFirstObjectIndex(this)), y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getLastObjectIndex(this), x  + 24 + Utilities_intLength(ObjectSpriteContainer_getFirstObjectIndex(this)) + 1, y, NULL);
	Printing_text(Printing_getInstance(), "Z Position: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(this->z), x + 24, y, NULL);
}

/**
 * Set Sprite's render mode
 *
 * @memberof		ObjectSpriteContainer
 * @public
 *
 * @param this		Function scope
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSpriteContainer_setMode(ObjectSpriteContainer this __attribute__ ((unused)), u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
{
	ASSERT(this, "ObjectSpriteContainer::setMode: null this");
}
