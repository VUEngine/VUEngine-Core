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
	this->nextAvailableObject = this->spt * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
	this->objectSprites = __NEW(VirtualList);
	this->objectSpriteToDefragment = NULL;
	this->objectIndexFreed = 0;
	this->z = 0;
	
	// register to sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

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
		VirtualList_pushBack(this->objectSprites, objectSprite);
		
		this->availableObjects -= numberOfObjects;
		this->nextAvailableObject += numberOfObjects;
		
		this->renderFlag = __UPDATE_HEAD;

		return this->nextAvailableObject - numberOfObjects;
	}
	
	return -1;
}

void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");
	ASSERT(VirtualList_find(this->objectSprites, objectSprite), "ObjectSpriteContainer::removeObjectSprite: not found");
	
	if(objectSprite)
	{
		if(this->objectSpriteToDefragment)
		{
			int objectSpritePosition = VirtualList_getNodePosition(this->objectSprites, objectSprite);
			int objectSpriteToDefragmentPosition =  VirtualList_getNodePosition(this->objectSprites, __UPCAST(ObjectSprite, VirtualNode_getData(this->objectSpriteToDefragment)));
			
			if(objectSpritePosition < objectSpriteToDefragmentPosition)
			{
				this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
				this->objectIndexFreed = ObjectSprite_getObjectIndex(objectSprite);
			}
		}
		else
		{
			// find the node to remove to defragment object memory
			this->objectSpriteToDefragment = VirtualList_find(this->objectSprites, objectSprite);
			this->objectIndexFreed = ObjectSprite_getObjectIndex(objectSprite);
	
			ASSERT(this->objectSpriteToDefragment, "ObjectSpriteContainer::removeObjectSprite: null objectSpriteToDefragment");
		}
		
		// move forward before deframenting
		this->objectSpriteToDefragment = VirtualNode_getNext(this->objectSpriteToDefragment);

		// if was the last node
		if(!this->objectSpriteToDefragment)
		{
			// just update the measures
			this->availableObjects += numberOfObjects;
			this->nextAvailableObject = this->objectIndexFreed;
			this->objectIndexFreed = 0;
		}
		
		VirtualList_removeElement(this->objectSprites, objectSprite);
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

void ObjectSpriteContainer_synchronizePosition(ObjectSpriteContainer this, VBVec3D position3D)
{
	ASSERT(this, "ObjectSpriteContainer::synchronizePosition: null this");
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
	// save its index for the next sprite to move
	int objectIndexFreed = ObjectSprite_getObjectIndex(objectSprite);
	ObjectSprite_setObjectIndex(objectSprite, this->objectIndexFreed);
	this->objectIndexFreed = objectIndexFreed;

	// move to the next sprite to move
	this->objectSpriteToDefragment = VirtualNode_getNext(this->objectSpriteToDefragment);	

	if(!this->objectSpriteToDefragment)
	{
		this->objectIndexFreed = 0;

		VirtualNode node = VirtualList_end(this->objectSprites);
		
		if(node)
		{
			ObjectSprite objectSprite = __UPCAST(ObjectSprite, VirtualNode_getData(node));
			this->nextAvailableObject = ObjectSprite_getObjectIndex(objectSprite) + ObjectSprite_getTotalObjects(objectSprite);
			this->availableObjects = __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER - this->nextAvailableObject;
		}
		else
		{
			this->availableObjects = __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
			this->nextAvailableObject = this->spt * __AVAILABLE_OBJECTS_PER_OBJECT_SPRITE_CONTAINER;
		}
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