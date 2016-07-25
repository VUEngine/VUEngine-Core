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

#include <ObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectSpriteContainerManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Screen.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __FLIP_X_DISPLACEMENT	6
#define __FLIP_Y_DISPLACEMENT	6


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ObjectSprite
__CLASS_DEFINITION(ObjectSprite, Sprite);

__CLASS_FRIEND_DEFINITION(Texture);


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
__CLASS_NEW_DEFINITION(ObjectSprite, const ObjectSpriteDefinition* oSpriteDefinition, Object owner)
__CLASS_NEW_END(ObjectSprite, oSpriteDefinition, owner);

// class's constructor
void ObjectSprite_constructor(ObjectSprite this, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner)
{
	ASSERT(this, "ObjectSprite::constructor: null this");

	__CONSTRUCT_BASE(Sprite, (SpriteDefinition*)objectSpriteDefinition, owner);

	this->head = objectSpriteDefinition->display;
	this->objectIndex = -1;
	this->objectSpriteContainer = NULL;
	this->totalObjects = 0;

	// clear position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	this->position.parallax = 0;

	this->displacement = objectSpriteDefinition->displacement;

	ASSERT(objectSpriteDefinition->textureDefinition, "ObjectSprite::constructor: null textureDefinition");

	this->texture = __SAFE_CAST(Texture, __NEW(ObjectTexture, objectSpriteDefinition->textureDefinition, 0));
	this->halfWidth = ITOFIX19_13((int)this->texture->textureDefinition->cols << 2);
	this->halfHeight = ITOFIX19_13((int)this->texture->textureDefinition->rows << 2);
	this->totalObjects = objectSpriteDefinition->textureDefinition->cols * objectSpriteDefinition->textureDefinition->rows;
	ASSERT(this->texture, "ObjectSprite::constructor: null texture");
}

// class's destructor
void ObjectSprite_destructor(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::destructor: null this");

	// make sure I'm hidden
	__VIRTUAL_CALL(Sprite, hide, this);

	// remove from sprite container before I become invalid
	// and the VPU triggers a new render cycle
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
	// must always be called at the end of the destructor
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

	this->texture->written = false;
}

VBVec2D ObjectSprite_getPosition(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getPosition: null this");

	return this->position;
}

void ObjectSprite_setPosition(ObjectSprite this, const VBVec2D* position)
{
	ASSERT(this, "ObjectSprite::setPosition: null this");

	this->position = *position;

	this->renderFlag |= __UPDATE_G;
	this->initialized = true;
}

void ObjectSprite_position(ObjectSprite this, const VBVec3D* position)
{
	ASSERT(this, "ObjectSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "ObjectSprite::position: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->position);
	this->position.z = position->z;

	if(0 > this->objectIndex)
	{
		this->objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager_getInstance(), this->totalObjects, this->position.z);
		ObjectSprite_setObjectIndex(this, ObjectSpriteContainer_addObjectSprite(this->objectSpriteContainer, this, this->totalObjects));
		ASSERT(0 <= this->objectIndex, "ObjectSprite::position: 0 > this->objectIndex");
	}

	this->renderFlag |= __UPDATE_G;
	this->initialized = true;
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
	ASSERT(Texture_getCharSet(this->texture), "ObjectSprite::render: null charSet");

	// if render flag is set
	if(this->renderFlag)
	{
		if(0 > this->objectIndex)
		{
			this->objectSpriteContainer = ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager_getInstance(), this->totalObjects, this->position.z);
			ObjectSprite_setObjectIndex(this, ObjectSpriteContainer_addObjectSprite(this->objectSpriteContainer, this, this->totalObjects));
			ASSERT(0 <= this->objectIndex, "ObjectSprite::position: 0 > this->objectIndex");
		}

		if(this->hidden)
		{
			if(0 <= this->objectIndex)
			{
				int i = 0;
				for(; i < this->totalObjects; i++)
				{
					OAM[((this->objectIndex + i) << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
				}
			}

			this->renderFlag = 0;
			return;
		}

		if(!this->texture->written)
		{
			ObjectTexture_write(__SAFE_CAST(ObjectTexture, this->texture));
		}

		if(!this->initialized)
		{
		//	return;
		}

		int cols = this->texture->textureDefinition->cols;
		int rows = this->texture->textureDefinition->rows;

		int xDirection = this->head & 0x2000 ? -1 : 1;
		int yDirection = this->head & 0x1000 ? -1 : 1;

		int x = FIX19_13TOI(this->position.x - this->halfWidth * xDirection + this->displacement.x) - (__LEFT == xDirection? __FLIP_X_DISPLACEMENT : 0);
		int y = FIX19_13TOI(this->position.y - this->halfHeight * yDirection + this->displacement.y) - (__UP == yDirection? __FLIP_Y_DISPLACEMENT : 0);

		int i = 0;
		u16 secondWordValue = (this->head & __OBJECT_CHAR_SHOW_MASK) | ((this->position.parallax + FIX19_13TOI(this->displacement.z)) & __OBJECT_CHAR_HIDE_MASK);
		u16 fourthWordValue = (this->head & 0x3000);

		for(; i < rows; i++)
		{
			int j = 0;
			for(; j < cols; j++)
			{
				s32 objectIndex = this->objectIndex + i * cols + j;
				int outputX = x + (j << 3)  * xDirection;

				// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
				// screen's bounds
				if((unsigned)(outputX + 8) > __SCREEN_WIDTH + 8)
				{
					OAM[(objectIndex << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
					continue;
				}

				int outputY = y + (i << 3)  * yDirection;

				if((unsigned)outputY > __SCREEN_HEIGHT + 8)
				{
					OAM[(objectIndex << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
					continue;
				}

				OAM[(objectIndex << 2)] = outputX;
				OAM[(objectIndex << 2) + 1] = secondWordValue | __OBJECT_CHAR_SHOW_MASK;
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
		ObjectTexture_setObjectIndex(__SAFE_CAST(ObjectTexture, this->texture), this->objectIndex);

		if(0 <= previousObjectIndex)
		{
			// hide the previously used objects
			int j = previousObjectIndex;
			for(; j < previousObjectIndex + this->totalObjects; j++)
			{
				OAM[(j << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
			}

			if(!this->hidden)
			{
				__VIRTUAL_CALL(Sprite, show, this);

				// turn off previous OBJs' to avoid ghosting
				if(this->objectIndex < previousObjectIndex)
				{
					int counter = 0;
					int j = previousObjectIndex + this->totalObjects - 1;
					for(; j >= this->objectIndex + this->totalObjects && counter < this->totalObjects; j--, counter++)
					{
						OAM[(j << 2) + 1] &= __OBJECT_CHAR_HIDE_MASK;
					}
				}
				else
				{
				}
			}
		}
		else
		{
			// render on next cycle
			this->renderFlag = !this->hidden;
		}
	}
}

void ObjectSprite_show(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::show: null this");

	Sprite_show(__SAFE_CAST(Sprite, this));
}

// hide
void ObjectSprite_hide(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::hide: null this");

	this->renderFlag = true;
	this->hidden = true;
}

u8 ObjectSprite_getWorldLayer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::getWorldLayer: null this");
	ASSERT(this->objectSpriteContainer, "ObjectSprite::getWorldLayer: null objectSpriteContainer");

	return this->objectSpriteContainer? __VIRTUAL_CALL_UNSAFE(Sprite, getWorldLayer, __SAFE_CAST(Sprite, this->objectSpriteContainer)): 0;
}

void ObjectSprite_addDisplacement(ObjectSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "ObjectSprite::addDisplacement: null this");

	this->position.x += displacement->x;
	this->position.y += displacement->y;
	this->position.z += displacement->z;
	this->position.parallax += displacement->parallax;

	Sprite_setRenderFlag(__SAFE_CAST(Sprite, this), __UPDATE_G);
}


void ObjectSprite_invalidateObjectSpriteContainer(ObjectSprite this)
{
	ASSERT(this, "ObjectSprite::invalidateObjectSpriteContainer: null this");

	this->objectSpriteContainer = NULL;
}
