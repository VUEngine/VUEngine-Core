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

#include <OMegaSprite.h>
#include <OTexture.h>
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

// define the OMegaSprite
__CLASS_DEFINITION(OMegaSprite, Sprite);


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
__CLASS_NEW_DEFINITION(OMegaSprite, u8 spt)
__CLASS_NEW_END(OMegaSprite, spt);

// class's constructor
void OMegaSprite_constructor(OMegaSprite this, u8 spt)
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
void OMegaSprite_destructor(OMegaSprite this)
{
	ASSERT(this, "OMegaSprite::destructor: null this");

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

s32 OMegaSprite_addOSprite(OMegaSprite this, OSprite oSprite, int numberOfObjects)
{
	ASSERT(this, "OMegaSprite::addOSprite: null this");
	ASSERT(oSprite, "OMegaSprite::addOSprite: null oSprite");
	
	if(oSprite)
	{
		VirtualList_pushBack(this->oSprites, oSprite);
		
		this->availableObjects -= numberOfObjects;
		this->nextAvailableObject += numberOfObjects;
		
		return this->nextAvailableObject - numberOfObjects;
	}
	
	return -1;
}

void OMegaSprite_removeOSprite(OMegaSprite this, OSprite oSprite, int numberOfObjects)
{
	ASSERT(this, "OMegaSprite::removeOSprite: null this");
	ASSERT(VirtualList_find(this->oSprites, oSprite), "OMegaSprite::removeOSprite: not found");
	
	if(oSprite)
	{
		VirtualList_removeElement(this->oSprites, oSprite);
	}
}

bool OMegaSprite_hasRoomFor(OMegaSprite this, int numberOfObjects)
{
	ASSERT(this, "OMegaSprite::removeOSprite: null this");

	return this->availableObjects >= numberOfObjects;
}

void OMegaSprite_setDirection(OMegaSprite this, int axis, int direction)
{
	ASSERT(this, "OMegaSprite::setDirection: null this");
}

VBVec2D OMegaSprite_getPosition(OMegaSprite this)
{
	ASSERT(this, "OMegaSprite::getPosition: null this");

	VBVec2D position =
	{
			0, 0
	};
	
	return position;
}

void OMegaSprite_setPosition(OMegaSprite this, VBVec2D position)
{
	ASSERT(this, "OMegaSprite::setPosition: null this");
}

void OMegaSprite_synchronizePosition(OMegaSprite this, VBVec3D position3D)
{
	ASSERT(this, "OMegaSprite::synchronizePosition: null this");
}

void OMegaSprite_calculateParallax(OMegaSprite this, fix19_13 z)
{
	ASSERT(this, "OMegaSprite::calculateParallax: null this");
}

// render a world layer with the map's information
void OMegaSprite_render(OMegaSprite this)
{
	ASSERT(this, "OMegaSprite::render: null this");

	this->renderFlag = __UPDATE_HEAD;
	//if render flag is set
	if (this->renderFlag)
	{
		if (__UPDATE_HEAD == this->renderFlag)
		{
			// make sure to not render again
			WA[this->worldLayer].head = this->head;
		}
		
		// make sure to not render again
		this->renderFlag = false;
	}
	
	VirtualNode node = VirtualList_begin(this->oSprites);

	for(; node; node = VirtualNode_getNext(node))
	{
		OSprite_render(__UPCAST(OSprite, VirtualNode_getData(node)));
	}

}