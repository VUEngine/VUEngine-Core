/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER	256


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ObjectSpriteContainer
__CLASS_DEFINITION(ObjectSpriteContainer, Sprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern Optical* _optical;

// external 
void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this);

// local
static void ObjectSpriteContainer_defragment(ObjectSpriteContainer this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSpriteContainer, u8 spt)
__CLASS_NEW_END(ObjectSpriteContainer, spt);

// class's constructor
void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, u8 spt)
{
	ASSERT(this, "ObjectSpriteContainer::constructor: null this");
	ASSERT(0 <= spt && spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainer::constructor: bad spt");

	__CONSTRUCT_BASE();

	this->head = WRLD_OBJ | WRLD_ON;
	this->spt = spt;
	this->availableObjects = __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
	this->objectSprites = __NEW(VirtualList);
	this->objectSpriteToDefragment = NULL;
	this->freedObjectIndex = 0;
	this->z = 0;
	
	// register to sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

	// clear OBJ memory
	int i = this->spt * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;

	for(; i < (this->spt + 1) * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER; i++)
	{
		OAM[(i << 2) + 3] = 0;
	}

	VIP_REGS[SPT0 + this->spt] = (this->spt + 1) * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER - 1;
}

// class's destructor
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::destructor: null this");

	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

	if(this->objectSprites)
	{
		VirtualNode node = VirtualList_begin(this->objectSprites);

		for(; node; node = VirtualNode_getNext(node))
		{
			ObjectSprite_invalidateObjectSpriteContainer(__UPCAST(ObjectSprite, VirtualNode_getData(node)));
			__DELETE(VirtualNode_getData(node));
		}

		__DELETE(this->objectSprites);
		this->objectSprites = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE;
}

s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::addObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::addObjectSprite: null objectSprite");
	
	if(objectSprite)
	{
		int lastObjectIndex = this->spt * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;

		if(VirtualList_getSize(this->objectSprites))
		{
			ObjectSprite lastObjectSprite = __UPCAST(ObjectSprite, VirtualList_back(this->objectSprites));
			
			ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");
			
			lastObjectIndex = ObjectSprite_getObjectIndex(lastObjectSprite);
			lastObjectIndex += ObjectSprite_getTotalObjects(lastObjectSprite);
		}
		
		VirtualList_pushBack(this->objectSprites, objectSprite);
		
		this->availableObjects -= numberOfObjects;
		
		this->renderFlag = __UPDATE_HEAD;

		return lastObjectIndex;
	}
	
	return -1;
}

void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");
	ASSERT(objectSprite, "ObjectSpriteContainer::removeObjectSprite: not objectSprite");
	ASSERT(VirtualList_find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: not found");
	
	if(this->objectSpriteToDefragment)
	{
		int objectSpritePosition = VirtualList_getNodePosition(this->objectSprites, objectSprite);
		int objectSpriteToDefragmentPosition =  VirtualList_getNodePosition(this->objectSprites, __UPCAST(ObjectSprite, VirtualNode_getData(this->objectSpriteToDefragment)));
		
		if(objectSpritePosition <= objectSpriteToDefragmentPosition)
		{
			this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
			this->freedObjectIndex = ObjectSprite_getObjectIndex(objectSprite);
			
			ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteToDefragment");

			// move forward before deframenting
			this->objectSpriteToDefragment = VirtualNode_getNext(this->objectSpriteToDefragment);
		}
	}
	else
	{
		// find the node to remove to defragment object memory
		this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
		this->freedObjectIndex = ObjectSprite_getObjectIndex(objectSprite);

		ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteToDefragment");

		// move forward before deframenting
		this->objectSpriteToDefragment = VirtualNode_getNext(this->objectSpriteToDefragment);
	}
	
	VirtualList_removeElement(this->objectSprites, objectSprite);
	
	// if was the last node
	if(!this->objectSpriteToDefragment || !VirtualList_begin(this->objectSprites))
	{
		// just update the measures
		this->objectSpriteToDefragment = NULL;
		this->availableObjects += numberOfObjects;
		this->freedObjectIndex = 0;
	}
}

bool ObjectSpriteContainer_hasRoomFor(ObjectSpriteContainer this, int numberOfObjects)
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
		0, 0, this->z, 0
	};

	return position;
}

void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, VBVec2D position)
{
	ASSERT(this, "ObjectSpriteContainer::setPosition: null this");

	this->z = position.z;
	this->renderFlag |= __UPDATE_G;
}

void ObjectSpriteContainer_positione(ObjectSpriteContainer this, VBVec3D position3D)
{
	ASSERT(this, "ObjectSpriteContainer::positione: null this");
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
	ObjectSprite objectSprite = __UPCAST(ObjectSprite, VirtualNode_getData(this->objectSpriteToDefragment));
	
	ASSERT(Sprite_getTexture(__UPCAST(Sprite, objectSprite)), "ObjectSpriteContainer::defragment: null texture");
	
	// move sprite back
	ObjectSprite_setObjectIndex(objectSprite, this->freedObjectIndex);
	
	// set new index to the end of the current sprite
	this->freedObjectIndex += ObjectSprite_getTotalObjects(objectSprite);

	// move to the next sprite to move
	this->objectSpriteToDefragment = VirtualNode_getNext(this->objectSpriteToDefragment);	
	
	if(!this->objectSpriteToDefragment)
	{
		this->freedObjectIndex = 0;

		VirtualNode node = VirtualList_end(this->objectSprites);
		
		if(node)
		{
			ObjectSprite lastObjectSprite = __UPCAST(ObjectSprite, VirtualNode_getData(node));
			this->availableObjects = __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER - (ObjectSprite_getObjectIndex(lastObjectSprite) + ObjectSprite_getTotalObjects(lastObjectSprite));
		}
		else
		{
			this->availableObjects = __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
		}
		
		return;
	}
}

// render a world layer with the map's information
void ObjectSpriteContainer_render(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::render: null this");
	
	if(this->objectSpriteToDefragment)
	{
		ObjectSpriteContainer_defragment(this);
	}
	
	//if render flag is set
	if (this->renderFlag)
	{
		// make sure to not render again
		WA[this->worldLayer].head = this->head | WRLD_OVR;
		
		// make sure to not render again
		this->renderFlag = false;
	}
	
	VirtualNode node = VirtualList_begin(this->objectSprites);

	for(; node; node = VirtualNode_getNext(node))
	{
		ObjectSprite_render(__UPCAST(ObjectSprite, VirtualNode_getData(node)));
	}
}

void ObjectSpriteContainer_show(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::show: null this");

	VirtualNode node = VirtualList_begin(this->objectSprites);

	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Sprite, show, __UPCAST(Sprite, VirtualNode_getData(node)));
	}
}

void ObjectSpriteContainer_hide(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::hide: null this");

	// must check list, because the Sprite's destructor calls this method
	if(this->objectSprites)
	{
		VirtualNode node = VirtualList_begin(this->objectSprites);
	
		for(; node; node = VirtualNode_getNext(node))
		{
			__VIRTUAL_CALL(void, Sprite, hide, __UPCAST(Sprite, VirtualNode_getData(node)));
		}
	}
}

u16 ObjectSpriteContainer_getAvailableObjects(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return this->availableObjects;
}

int ObjectSpriteContainer_getTotalUsedObjects(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return VirtualList_getSize(this->objectSprites);
}

int ObjectSpriteContainer_getNextFreeObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	if(VirtualList_begin(this->objectSprites))
	{
		ObjectSprite lastObjectSprite = __UPCAST(ObjectSprite, VirtualList_back(this->objectSprites));
		
		ASSERT(lastObjectSprite, "ObjectSpriteContainer::addObjectSprite: null lastObjectSprite");
		
		return ObjectSprite_getObjectIndex(lastObjectSprite) + ObjectSprite_getTotalObjects(lastObjectSprite);
	}
	
	return 0;
}

int ObjectSpriteContainer_getFirstObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return this->spt * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
}

int ObjectSpriteContainer_getLastObjectIndex(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::getAvailableObjects: null this");
	
	return (this->spt + 1) * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER - 1;
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
