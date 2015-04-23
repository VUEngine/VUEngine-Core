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

#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectSpriteContainerManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ObjectSprite
__CLASS_DEFINITION(ObjectSprite, Sprite);


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
__CLASS_NEW_DEFINITION(ObjectSprite, const ObjectSpriteDefinition* mSpriteDefinition)
__CLASS_NEW_END(ObjectSprite, mSpriteDefinition);

// class's constructor
void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* oSpriteDefinition)
{
	ASSERT(this, "ObjectSprite::constructor: null this");

	__CONSTRUCT_BASE();

	this->head = oSpriteDefinition->display;
	this->objectIndex = -1;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;
	
	// clear position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	this->position.parallax = 0;

	ASSERT(oSpriteDefinition->textureDefinition, "ObjectSprite::constructor: null textureDefinition");

	if(oSpriteDefinition->textureDefinition)
	{
		this->texture = __UPCAST(Texture, __NEW(ObjectTexture, oSpriteDefinition->textureDefinition, 0));
		this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
		this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
		this->totalObjects = oSpriteDefinition->textureDefinition->cols * oSpriteDefinition->textureDefinition->rows;
	}
}

// class's destructor
void ObjectSprite_destructor(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::destructor: null this");

	if(this->objectSpriteContainer)
	{
		ObjectSpriteContainer_removeObjectSprite(this->objectSpriteContainer, this, this->totalObjects);
	}

	// destroy the super object
	__DESTROY_BASE;
}

void ObjectSprite_setDirection(ObjectSprite this, int axis, int direction)
{
	ASSERT(this, "ObjectSprite::setDirection: null this");

	switch (axis)
	{
		case __XAXIS:

			if(__LEFT == direction)
			{
				this->head |= 0x2000;
			}
			else if(__RIGHT == direction)
			{
				this->head &= 0xDFFF;
			}
			break;

		case __YAXIS:

			if(__UP == direction)
			{
				this->head |= 0x1000;
			}
			else if(__DOWN == direction)
			{
				this->head &= 0xEFFF;
			}
			break;
	}
}

VBVec2D ObjectSprite_getPosition(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getPosition: null this");

	return this->position;
}

void ObjectSprite_setPosition(ObjectSprite this, VBVec2D position)
{
	ASSERT(this, "ObjectSprite::setPosition: null this");

	this->position.x = position.x;
	this->position.y = position.y;
	this->position.z = position.z;

	this->renderFlag |= __UPDATE_G;
}

void ObjectSprite_synchronizePosition(ObjectSprite this, VBVec3D position3D)
{
	ASSERT(this, "ObjectSprite::synchronizePosition: null this");

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "ObjectSprite::setPosition: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->position);

	if(0 > this->objectIndex)
	{
		this->objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager_getInstance(), this->totalObjects, this->position.z);
		ObjectSprite_setObjectIndex(this, ObjectSpriteContainer_addObjectSprite(this->objectSpriteContainer, this, this->totalObjects));
	}
	
	this->renderFlag |= __UPDATE_G;
}

void ObjectSprite_calculateParallax(ObjectSprite this, fix19_13 z)
{
	ASSERT(this, "ObjectSprite::calculateParallax: null this");

	this->position.z = z - _screenPosition->z;
	this->position.parallax = Optics_calculateParallax(this->position.x, z);
}

// render a world layer with the map's information
void ObjectSprite_render(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::render: null this");
	ASSERT(this->texture, "ObjectSprite::render: null texture");
	ASSERT(0 <= this->objectIndex, "ObjectSprite::render: 0 > this->objectIndex");

	//if render flag is set
	if (this->renderFlag && 0 <= this->objectIndex)
	{
		int cols = Texture_getCols(__UPCAST(Texture, this->texture));
		int rows = Texture_getRows(__UPCAST(Texture, this->texture));

		int xDirection = this->head & 0x2000? -1: 1;
		int yDirection = this->head & 0x1000? -1: 1;
		int x = FIX19_13TOI(this->position.x) - FIX19_13TOI(this->halfWidth) * xDirection;
		int y = FIX19_13TOI(this->position.y) - FIX19_13TOI(this->halfHeight) * yDirection;
		
		int i = 0;
		for (; i < rows; i++)
		{
			int j = 0;
			for (; j < cols; j++)
			{
				s32 objectIndex = this->objectIndex + i * cols + j;
				OAM[objectIndex << 2] = x + (8 * j)  * xDirection;
				OAM[(objectIndex << 2) + 1] = (this->head & 0xC000) | (this->position.parallax & 0x3FFF);
				OAM[(objectIndex << 2) + 2] = y + (8 * i)  * yDirection;
				OAM[(objectIndex << 2) + 3] = (OAM[(objectIndex << 2) + 3] & 0xCFFF) | (this->head & 0x3000);
			}
		}
		
		// make sure to not render again
		this->renderFlag = false;
	}
}

u8 ObjectSprite_getTotalObjects(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getTotalObjects: null this");

	return this->totalObjects;
}

int ObjectSprite_getObjectIndex(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getObjectIndex: null this");

	return this->objectIndex;
}

void ObjectSprite_setObjectIndex(ObjectSprite this, int objectIndex)
{
	ASSERT(this, "ObjectSprite::setObjectIndex: null this");

	if(0 <= this->objectIndex)
	{
		ObjectSprite_hide(this);
	}
	
	this->objectIndex = objectIndex;

	if(0 <= this->objectIndex)
	{
		// rewrite texture
		ObjectTexture_setObjectIndex(__UPCAST(ObjectTexture, this->texture), this->objectIndex);
		ObjectTexture_write(__UPCAST(ObjectTexture, this->texture));
	
		// force rendering 
		this->renderFlag = true;
	}
}

void ObjectSprite_show(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::show: null this");
	
	Sprite_show(__UPCAST(Sprite, this));

	ObjectTexture_write(__UPCAST(ObjectTexture, this->texture));
}

// hide
void ObjectSprite_hide(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::hide: null this");

	int cols = Texture_getCols(__UPCAST(Texture, this->texture));
	int rows = Texture_getRows(__UPCAST(Texture, this->texture));

	int i = 0;
	for (; i < rows; i++)
	{
		int j = 0;
		for (; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			OAM[(objectIndex << 2) + 3] = 0;
		}
	}
}
