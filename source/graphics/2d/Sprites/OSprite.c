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

#include <OSprite.h>
#include <OMegaSprite.h>
#include <OMegaSpriteManager.h>
#include <OTexture.h>
#include <Optics.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the OSprite
__CLASS_DEFINITION(OSprite, Sprite);


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
__CLASS_NEW_DEFINITION(OSprite, const OSpriteDefinition* mSpriteDefinition)
__CLASS_NEW_END(OSprite, mSpriteDefinition);

// class's constructor
void OSprite_constructor(OSprite this, const OSpriteDefinition* oSpriteDefinition)
{
	ASSERT(this, "OSprite::constructor: null this");

	__CONSTRUCT_BASE();

	this->head = oSpriteDefinition->display;
	this->objectIndex = -1;
	this->oMegaSprite = NULL;
	this->totalObjects = 0;
	
	// clear position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	this->position.parallax = 0;

	ASSERT(oSpriteDefinition->textureDefinition, "OSprite::constructor: null textureDefinition");

	if(oSpriteDefinition->textureDefinition)
	{
		this->texture = __UPCAST(Texture, __NEW(OTexture, oSpriteDefinition->textureDefinition, 0));
		
		this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
		this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);

		this->totalObjects = oSpriteDefinition->textureDefinition->cols * oSpriteDefinition->textureDefinition->rows;
		this->oMegaSprite = OMegaSpriteManager_getOMegaSprite(OMegaSpriteManager_getInstance(), this->totalObjects);
		
		this->objectIndex = OMegaSprite_addOSprite(this->oMegaSprite, this, this->totalObjects);
		OTexture_setObjectIndex(__UPCAST(OTexture, this->texture), this->objectIndex);
		OTexture_write(__UPCAST(OTexture, this->texture));
	}
}

// class's destructor
void OSprite_destructor(OSprite this)
{
	ASSERT(this, "OSprite::destructor: null this");

	if(this->oMegaSprite)
	{
		OMegaSprite_removeOSprite(this->oMegaSprite, this, this->totalObjects);
	}

	// destroy the super object
	__DESTROY_BASE;
}

void OSprite_setDirection(OSprite this, int axis, int direction)
{
	ASSERT(this, "OSprite::setDirection: null this");

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

VBVec2D OSprite_getPosition(OSprite this)
{
	ASSERT(this, "OSprite::getPosition: null this");

	VBVec2D position =
	{
			0, 0
	};
	
	return position;
}

void OSprite_setPosition(OSprite this, VBVec2D position)
{
	ASSERT(this, "OSprite::setPosition: null this");

	this->position.x = position.x;
	this->position.y = position.y;
	this->position.z = position.z;
}

void OSprite_synchronizePosition(OSprite this, VBVec3D position3D)
{
	ASSERT(this, "OSprite::synchronizePosition: null this");

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "OSprite::setPosition: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->position);

	this->renderFlag |= __UPDATE_G;
}

void OSprite_calculateParallax(OSprite this, fix19_13 z)
{
	ASSERT(this, "OSprite::calculateParallax: null this");

	this->position.z = z - _screenPosition->z;
	this->position.parallax = Optics_calculateParallax(this->position.x, z);
}

// render a world layer with the map's information
void OSprite_render(OSprite this)
{
	ASSERT(this, "OSprite::render: null this");
	ASSERT(this->texture, "OSprite::render: null texture");

	//if render flag is set
	if (this->renderFlag)
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
				OAM[ objectIndex << 2] = x + (8 * j)  * xDirection;
				OAM[(objectIndex << 2) + 1] = (this->head & 0xC000) | (this->position.parallax & 0x3FFF);
				OAM[(objectIndex << 2) + 2] = y + (8 * i)  * yDirection;
				OAM[(objectIndex << 2) + 3] = (OAM[(objectIndex << 2) + 3] & 0xCFFF) | (this->head & 0x3000);
			}
		}
		
		// make sure to not render again
		this->renderFlag = false;
	}
}

int OSprite_getObjectIndex(OSprite this)
{
	ASSERT(this, "OSprite::getObjectIndex: null this");

	return this->objectIndex;
}