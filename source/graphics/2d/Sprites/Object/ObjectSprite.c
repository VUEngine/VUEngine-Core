/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
#define __SHOW_MASK						0xC000
#define __HIDE_MASK						0x3FFF

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
extern unsigned int volatile* _xpstts;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectSprite, const ObjectSpriteDefinition* oSpriteDefinition, Object owner)
__CLASS_NEW_END(ObjectSprite, oSpriteDefinition, owner);

// class's constructor
void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* oSpriteDefinition, Object owner)
{
	ASSERT(this, "ObjectSprite::constructor: null this");

	__CONSTRUCT_BASE((SpriteDefinition*)oSpriteDefinition, owner);

	this->head = oSpriteDefinition->display;
	this->objectIndex = -1;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;
	
	// clear position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	this->position.parallax = 0;
	
	this->displacement = oSpriteDefinition->displacement;


	ASSERT(oSpriteDefinition->textureDefinition, "ObjectSprite::constructor: null textureDefinition");

	this->texture = __GET_CAST(Texture, __NEW(ObjectTexture, oSpriteDefinition->textureDefinition, 0));
	this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
	this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
	this->totalObjects = oSpriteDefinition->textureDefinition->cols * oSpriteDefinition->textureDefinition->rows;
	ASSERT(this->texture, "ObjectSprite::constructor: null texture");
}

// class's destructor
void ObjectSprite_destructor(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::destructor: null this");

	if(this->objectSpriteContainer)
	{
		ObjectSpriteContainer_removeObjectSprite(this->objectSpriteContainer, this, this->totalObjects);
	}

	if(this->texture)
	{
		__DELETE(this->texture);
		this->texture = NULL;
	}

	// destroy the super object
	__DESTROY_BASE;
}

void ObjectSprite_setDirection(ObjectSprite this, int axis, int direction)
{
	ASSERT(this, "ObjectSprite::setDirection: null this");

	switch(axis)
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

const VBVec2D* ObjectSprite_getPosition(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getPosition: null this");

	return &this->position;
}

void ObjectSprite_setPosition(ObjectSprite this, const VBVec2D* position)
{
	ASSERT(this, "ObjectSprite::setPosition: null this");

	this->position = *position;

	this->renderFlag |= __UPDATE_G;
}

void ObjectSprite_positione(ObjectSprite this, const VBVec3D* position)
{
	ASSERT(this, "ObjectSprite::positione: null this");

	VBVec3D position3D = *position;
	
	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "ObjectSprite::positione: null texture");

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
	ASSERT(Texture_getCharSet(this->texture), "ObjectSprite::render: null charSet");

	//if render flag is set
	if(this->renderFlag && 0 <= this->objectIndex)
	{
		int cols = Texture_getCols(__GET_CAST(Texture, this->texture));
		int rows = Texture_getRows(__GET_CAST(Texture, this->texture));

		int xDirection = this->head & 0x2000? -1: 1;
		int yDirection = this->head & 0x1000? -1: 1;

		int x = FIX19_13TOI(this->position.x) - FIX19_13TOI(this->halfWidth) * xDirection + this->displacement.x;
		int y = FIX19_13TOI(this->position.y) - FIX19_13TOI(this->halfHeight) * yDirection + this->displacement.y;
		
		int i = 0;
		u16 secondWordValue = (this->head & __SHOW_MASK) | ((this->position.parallax + this->displacement.z) & __HIDE_MASK);
		u16 fourthWordValue = (this->head & 0x3000);
		
		for(; i < rows; i++)
		{
			int j = 0;
			for(; j < cols; j++)
			{
				s32 objectIndex = this->objectIndex + i * cols + j;
				int outputX = x + (j << 3)  * xDirection;
				
				// add 8 to the calculation to avoid char's cut off
				// when screolling
				// hide the object if ouside screen's bounds
				if((unsigned)(outputX + 8) > __SCREEN_WIDTH + 8)
				{
					OAM[(objectIndex << 2) + 1] &= __HIDE_MASK;
					continue;
				}

				int outputY = y + (i << 3)  * yDirection;
				if((unsigned)outputY > __SCREEN_HEIGHT + 8)
				{
					OAM[(objectIndex << 2) + 1] &= __HIDE_MASK;
					continue;
				}

				OAM[(objectIndex << 2)] = outputX;
				OAM[(objectIndex << 2) + 1] = secondWordValue;
				OAM[(objectIndex << 2) + 2] = outputY;
				OAM[(objectIndex << 2) + 3] |= fourthWordValue;
			}
		}
		
		// make sure to not render again
		this->renderFlag = false;
	}
}

s16 ObjectSprite_getTotalObjects(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getTotalObjects: null this");
	ASSERT(0 < this->totalObjects, "ObjectSprite::getTotalObjects: null totalObjects");

	return this->totalObjects;
}

s16 ObjectSprite_getObjectIndex(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getObjectIndex: null this");

	return this->objectIndex;
}

void ObjectSprite_setObjectIndex(ObjectSprite this, s16 objectIndex)
{
	ASSERT(this, "ObjectSprite::setObjectIndex: null this");
	ASSERT(this->texture, "ObjectSprite::setObjectIndex: null texture");

	int previousObjectIndex = this->objectIndex;
	this->objectIndex = objectIndex;

	if(0 <= this->objectIndex)
	{
		// rewrite texture
		ObjectTexture_setObjectIndex(__GET_CAST(ObjectTexture, this->texture), this->objectIndex);
		ObjectTexture_write(__GET_CAST(ObjectTexture, this->texture));

		if(0 <= previousObjectIndex)
		{	
			// if was visible
			if(OAM[((previousObjectIndex) << 2) + 1] & 0xC000)
			{
				// render in the new position to avoid flickering
				this->renderFlag = true;
	
				while (*_xpstts & XPBSYR);
	
				ObjectSprite_render(this);
				
				// turn off previous OBJs' to avoid ghosting
				int i = previousObjectIndex + this->totalObjects - 1;
				for(; i >= this->objectIndex + this->totalObjects; i--)
				{
					OAM[(i << 2) + 1] &= __HIDE_MASK;
				}
			}
			else
			{
				// otherwise hide
				ObjectSprite_hide(this);
			}
		}
		else
		{
			// render on next cycle
			this->renderFlag = true;
		}
	}
}

void ObjectSprite_show(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::show: null this");
	
	Sprite_show(__GET_CAST(Sprite, this));

	if(this->renderFlag)
	{
		while (*_xpstts & XPBSYR);

		ObjectSprite_render(this);
	}
	
	if (0 <= this->objectIndex)
	{
		int i = 0;
		for (; i < this->totalObjects; i++)
		{
			OAM[((this->objectIndex + i) << 2) + 1] |= __SHOW_MASK;
		}
	}
}

// hide
void ObjectSprite_hide(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::hide: null this");

	if(0 > this->objectIndex)
	{
		return;
	}

	if (0 <= this->objectIndex)
	{
		int i = 0;
		for (; i < this->totalObjects; i++)
		{
			OAM[((this->objectIndex + i) << 2) + 1] &= __HIDE_MASK;
		}
	}
}

u8 ObjectSprite_getWorldLayer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getWorldLayer: null this");

	return this->objectSpriteContainer? __VIRTUAL_CALL_UNSAFE(u8, Sprite, getWorldLayer, __GET_CAST(Sprite, this->objectSpriteContainer)): 0;
}

void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::invalidateObjectSpriteContainer: null this");

	this->objectSpriteContainer = NULL;
}
