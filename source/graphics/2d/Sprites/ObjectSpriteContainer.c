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
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>
#include <SpriteManager.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_OBJECTS_PER_O_MEGA_SPRITE	256


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



//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSpriteContainer, u8 spt)
__CLASS_NEW_END(ObjectSpriteContainer, spt);

// class's constructor
void ObjectSpriteContainer_constructor(ObjectSpriteContainer this, u8 spt)
{
	__CONSTRUCT_BASE();

	this->head = WRLD_OBJ | WRLD_ON;
	this->availableObjects = __AVAILABLE_OBJECTS_PER_O_MEGA_SPRITE;
	this->nextAvailableObject = 0;
	this->oSprites = __NEW(VirtualList);
	this->spt = spt;
	
	VIP_REGS[this->spt] = __AVAILABLE_OBJECTS_PER_O_MEGA_SPRITE;

	// register to sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));
}

// class's destructor
void ObjectSpriteContainer_destructor(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::destructor: null this");

	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

	if(this->oSprites)
	{
		__DELETE(this->oSprites);
		this->oSprites = NULL;
	}
	// destroy the super object
	__DESTROY_BASE;
}

s32 ObjectSpriteContainer_addObjectSprite(ObjectSpriteContainer this, ObjectSprite oSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::addObjectSprite: null this");
	ASSERT(oSprite, "ObjectSpriteContainer::addObjectSprite: null oSprite");
	
	if(oSprite)
	{
		VirtualList_pushBack(this->oSprites, oSprite);
		
		this->availableObjects -= numberOfObjects;
		this->nextAvailableObject += numberOfObjects;
		
		this->renderFlag = __UPDATE_HEAD;

		return this->nextAvailableObject - numberOfObjects;
	}
	
	return -1;
}

void ObjectSpriteContainer_removeObjectSprite(ObjectSpriteContainer this, ObjectSprite oSprite, int numberOfObjects)
{
	ASSERT(this, "ObjectSpriteContainer::removeObjectSprite: null this");
	ASSERT(VirtualList_find(this->oSprites, oSprite), "ObjectSpriteContainer::removeObjectSprite: not found");
	
	if(oSprite)
	{
		VirtualList_removeElement(this->oSprites, oSprite);
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
			0, 0
	};
	
	return position;
}

void ObjectSpriteContainer_setPosition(ObjectSpriteContainer this, VBVec2D position)
{
	ASSERT(this, "ObjectSpriteContainer::setPosition: null this");
}

void ObjectSpriteContainer_synchronizePosition(ObjectSpriteContainer this, VBVec3D position3D)
{
	ASSERT(this, "ObjectSpriteContainer::synchronizePosition: null this");
}

void ObjectSpriteContainer_calculateParallax(ObjectSpriteContainer this, fix19_13 z)
{
	ASSERT(this, "ObjectSpriteContainer::calculateParallax: null this");
}

// render a world layer with the map's information
void ObjectSpriteContainer_render(ObjectSpriteContainer this)
{
	ASSERT(this, "ObjectSpriteContainer::render: null this");

	//if render flag is set
	if (this->renderFlag)
	{
		// make sure to not render again
		WA[this->worldLayer].head = this->head | WRLD_OVR;
		
		// make sure to not render again
		this->renderFlag = false;
	}
	
	VirtualNode node = VirtualList_begin(this->oSprites);

	for(; node; node = VirtualNode_getNext(node))
	{
		ObjectSprite_render(__UPCAST(ObjectSprite, VirtualNode_getData(node)));
	}
}