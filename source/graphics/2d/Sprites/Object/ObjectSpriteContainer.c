/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSpriteContainer.h>
#include <ObjectSpriteContainerManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>
#include <SpriteManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ObjectSpriteContainer
__CLASS_DEFINITION(ObjectSpriteContainer, Sprite);

__CLASS_FRIEND_DEFINITION(ObjectSprite);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern Optical* _optical;

// external 
void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this);

static void ObjectSpriteContainer_defragment(ObjectSpriteContainer this);
static void ObjectSpriteContainer_sort(ObjectSpriteContainer this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSpriteContainer, u8 spt, u16 totalObjects, u16 firstObjectIndex)
__CLASS_NEW_END(ObjectSpriteContainer, spt, totalObjects, firstObjectIndex);

// class's constructor
void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, u8 spt, u16 totalObjects, u16 firstObjectIndex)
{
	ASSERT(this, "ObjectSpriteContainer::constructor: null this");
	ASSERT(0 <= spt && spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainer::constructor: bad spt");

	__CONSTRUCT_BASE();

	this->head = WRLD_OBJ | WRLD_ON;
	this->spt = spt;	
	this->totalObjects = totalObjects;
	this->availableObjects = this->totalObjects;
	this->firstObjectIndex = firstObjectIndex;
	this->objectSprites = __NEW(VirtualList);
	this->objectSpriteToDefragment = NULL;
	this->freedObjectIndex = 0;
	this->z = 0;
	
	this->node = NULL;
	this->previousNode = NULL;
	
	// register to sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));

	// clear OBJ memory
	int i = firstObjectIndex;

	for(; i < this->firstObjectIndex + this->totalObjects; i++)
	{
		OAM[(i << 2) + 0] = 0;
		OAM[(i << 2) + 1] = 0;
		OAM[(i << 2) + 2] = 0;
		OAM[(i << 2) + 3] = 0;
	}

	VIP_REGS[SPT0 + this->spt] = this->firstObjectIndex + this->totalObjects - 1;
}

// class's destructor
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::destructor: null this");
	ASSERT(this->objectSprites, "ObjectSpriteContainer::destructor: null objectSprites");

	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));

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

s16 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::addObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");
	
	if(objectSprite)
	{
		s16 lastObjectIndex = this->firstObjectIndex;

		if(VirtualList_getSize(this->objectSprites))
		{
			ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, VirtualList_back(this->objectSprites));
			
			ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");
			
			lastObjectIndex = lastObjectSprite->objectIndex;
			lastObjectIndex += lastObjectSprite->totalObjects;
		}
		
		VirtualList_pushBack(this->objectSprites, objectSprite);
		
		this->availableObjects -= numberOfObjects;
		
		this->renderFlag = __UPDATE_HEAD;

		this->node = NULL;
		this->previousNode = NULL;

		return lastObjectIndex;
	}
	
	return -1;
}

void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s16 numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::removeObjectSprite: not objectSprite");
	ASSERT(VirtualList_find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: not found");
	
	// hide it
	if (0 <= objectSprite->objectIndex)
	{
		int i = 0;
		for (; i < objectSprite->totalObjects; i++)
		{
			OAM[((objectSprite->objectIndex + i) << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
		}
	}
	
	__VIRTUAL_CALL(void, Sprite, hide, objectSprite);

	if(this->objectSpriteToDefragment)
	{
		int objectSpritePosition = VirtualList_getNodePosition(this->objectSprites, objectSprite);
		int objectSpriteToDefragmentPosition =  VirtualList_getNodePosition(this->objectSprites, __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->objectSpriteToDefragment)));
		
		if(objectSpritePosition <= objectSpriteToDefragmentPosition)
		{
			this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
			this->freedObjectIndex = objectSprite->objectIndex;
			
			ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteToDefragment");

			// move forward before deframenting
			this->objectSpriteToDefragment = this->objectSpriteToDefragment->next;
		}
	}
	else
	{
		// find the node to remove to defragment object memory
		this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
		this->freedObjectIndex = objectSprite->objectIndex;

		ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteToDefragment");

		// move forward before deframenting
		this->objectSpriteToDefragment = this->objectSpriteToDefragment->next;
	}
	
	VirtualList_removeElement(this->objectSprites, objectSprite);

	this->node = this->previousNode = NULL;

	// if was the last node
	if(!this->objectSpriteToDefragment || !this->objectSprites->head)
	{
		// just update the measures
		this->objectSpriteToDefragment = NULL;
		this->availableObjects += numberOfObjects;
		this->freedObjectIndex = 0;
	}
}

bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, s16 numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");

	return this->availableObjects >= numberOfObjects;
}

void ObjectSpriteContainer_setDirection(ObjectSpriteContainer this, int axis, int direction)
{
	ASSERT(this, "ObjectSpriteContainer::setDirection: null this");
}

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

void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, const VBVec2D* position)
{
	ASSERT(this, "ObjectSpriteContainer::setPosition: null this");

	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;
	
		for(; node; node = node->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, node->data);
			
			VBVec2D spritePosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, sprite);
			__VIRTUAL_CALL(void, Sprite, setPosition, sprite, &spritePosition);
		}
	}

	this->z = position->z;
	this->renderFlag |= __UPDATE_G;
	this->initialized = true;
}

void ObjectSpriteContainer_position(ObjectSpriteContainer this, const VBVec3D* position)
{
	ASSERT(this, "ObjectSpriteContainer::position: null this");

	this->initialized = true;
}

void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z)
{
	ASSERT(this, "ObjectSpriteContainer::calculateParallax: null this");
}

static void ObjectSpriteContainer_defragment(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::defragment: null this");
	ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::defragment: null objectSpriteToDefragment");

	// get the next sprite to move
	ObjectSprite objectSprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->objectSpriteToDefragment));
	
	ASSERT(Sprite_getTexture(__SAFE_CAST(Sprite, objectSprite)), "ObjectSpriteContainer::defragment: null texture");
	
	// move sprite back
	ObjectSprite_setObjectIndex(objectSprite, this->freedObjectIndex);

	// set new index to the end of the current sprite
	this->freedObjectIndex += objectSprite->totalObjects;

	// move to the next sprite to move
	this->objectSpriteToDefragment = this->objectSpriteToDefragment->next;	
	
	if(!this->objectSpriteToDefragment)
	{
		this->freedObjectIndex = 0;

		VirtualNode node = this->objectSprites->tail;
		
		if(node)
		{
			ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, node->data);
			this->availableObjects = this->totalObjects - (lastObjectSprite->objectIndex + lastObjectSprite->totalObjects);
		}
		else
		{
			this->availableObjects = this->totalObjects;
		}
		
		return;
	}
}

static void ObjectSpriteContainer_sort(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::sort: null this");

	this->node = this->node ? this->previousNode ? this->node : VirtualNode_getPrevious(this->node): this->objectSprites->tail;

	for(; this->node; this->node = VirtualNode_getPrevious(this->node))
	{
		this->previousNode = VirtualNode_getPrevious(this->node);

		if(this->previousNode)
		{
			ObjectSprite sprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->node));
			ObjectSprite previousSprite = __SAFE_CAST(ObjectSprite, VirtualNode_getData(this->previousNode));
			VBVec2D position = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, __SAFE_CAST(Sprite, sprite));
			VBVec2D previousPosition = __VIRTUAL_CALL_UNSAFE(VBVec2D, Sprite, getPosition, __SAFE_CAST(Sprite, previousSprite));
	
			// check if z positions are swapped
			if(previousPosition.z + Sprite_getDisplacement(__SAFE_CAST(Sprite, previousSprite)).z > position.z + Sprite_getDisplacement(__SAFE_CAST(Sprite, sprite)).z)
			{
				if(this->availableObjects >= sprite->totalObjects)
				{
					// swap
					s16 previousObjectIndex = previousSprite->objectIndex;
	
					ObjectSprite lastObjectSprite = __SAFE_CAST(ObjectSprite, VirtualList_back(this->objectSprites));
					s16 nextFreeObjectIndex = lastObjectSprite->objectIndex + lastObjectSprite->totalObjects;
					
					ObjectSprite_setObjectIndex(previousSprite, nextFreeObjectIndex);
					ObjectSprite_setObjectIndex(sprite, previousObjectIndex);
					ObjectSprite_setObjectIndex(previousSprite, previousObjectIndex + sprite->totalObjects);
					
					int i = 0;
					for (; i < sprite->totalObjects; i++)
					{
						OAM[((nextFreeObjectIndex + i) << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
					}
					
					// swap array entries
					VirtualNode_swapData(this->node, this->previousNode);
		
					this->node = this->previousNode;
					return;
				}
			}
		}
	}
}

// render a world layer with the map's information
void ObjectSpriteContainer_render(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::render: null this");

	//if render flag is set
	if(this->renderFlag)
	{
		// make sure to not render again
		WA[this->worldLayer].head = this->head | WRLD_OVR;
		
		// make sure to not render again
		this->renderFlag = false;
	}

	// defragmentation takes priority over z sorting
	if(this->objectSpriteToDefragment)
	{
		ObjectSpriteContainer_defragment(this);
	}
	else
	{
		ObjectSpriteContainer_sort(this);
	}

	VirtualNode node = this->objectSprites->head;
	for(; node; node = node->next)
	{
		ObjectSprite_render(__SAFE_CAST(ObjectSprite, node->data));
	}
}

void ObjectSpriteContainer_show(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::show: null this");

	VirtualNode node = this->objectSprites->head;

	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(void, Sprite, show, __SAFE_CAST(Sprite, node->data));
	}
	
	this->renderFlag = true;
	this->hidden = false;
	this->initialized = false;
}

void ObjectSpriteContainer_hide(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::hide: null this");

	// must check list, because the Sprite's destructor calls this method
	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;
	
		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Sprite, hide, __SAFE_CAST(Sprite, node->data));
		}
	}
	
	this->hidden = true;
}

u16 ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return this->availableObjects;
}

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

int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return this->firstObjectIndex;
}

int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return this->firstObjectIndex + this->totalObjects;
}

void ObjectSpriteContainer_addDisplacement(ObjectSpriteContainer this, const VBVec2D* displacement)
{
	ASSERT(this, "BgmapSprite::addDisplacement: null this");

	if(this->objectSprites)
	{
		VirtualNode node = this->objectSprites->head;

		for(; node; node = node->next)
		{
			__VIRTUAL_CALL(void, Sprite, addDisplacement, __SAFE_CAST(Sprite, node->data), displacement);
		}
	}
	
	Sprite_setRenderFlag(__SAFE_CAST(Sprite, this), __UPDATE_G);
}


void ObjectSpriteContainer_print(ObjectSpriteContainer this, int x, int y)
{
	ASSERT(this, "ObjectSpriteContainer::print: null this");

	Printing_text(Printing_getInstance(), "Segment: ", x, y, NULL);
	Printing_int(Printing_getInstance(), this->spt, x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Available objects: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getAvailableObjects(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Total used objects: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getTotalUsedObjects(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Next free object index: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getNextFreeObjectIndex(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "Object index range: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getFirstObjectIndex(this), x + 24, y, NULL);
	Printing_text(Printing_getInstance(), "-", x  + 24 + Utilities_intLength(ObjectSpriteContainer_getFirstObjectIndex(this)), y, NULL);
	Printing_int(Printing_getInstance(), ObjectSpriteContainer_getLastObjectIndex(this), x  + 24 + Utilities_intLength(ObjectSpriteContainer_getFirstObjectIndex(this)) + 1, y, NULL);
	Printing_text(Printing_getInstance(), "Z Position: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(this->z), x + 24, y, NULL);
}