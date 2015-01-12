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

#include <MSprite.h>
#include <Game.h>
#include <SpriteManager.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <HardwareManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the MSprite
__CLASS_DEFINITION(MSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MSprite_loadTextures(MSprite this);
static void MSprite_loadTexture(MSprite this, TextureDefinition* textureDefinition);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MSprite, __PARAMETERS(const MSpriteDefinition* mSpriteDefinition))
__CLASS_NEW_END(MSprite, __ARGUMENTS(mSpriteDefinition));

// class's constructor
void MSprite_constructor(MSprite this, const MSpriteDefinition* mSpriteDefinition)
{
	__CONSTRUCT_BASE(Sprite, __ARGUMENTS(&mSpriteDefinition->spriteDefinition));
	
	this->mSpriteDefinition = mSpriteDefinition;
	//this->textures = NULL;
	
	//MSprite_loadTextures(this);
}

// class's destructor
void MSprite_destructor(MSprite this)
{
	ASSERT(this, "MSprite::destructor: null this");

	if(this->textures)
	{
		VirtualNode node = VirtualList_begin(this->textures);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			// free the texture
			TextureManager_free(TextureManager_getInstance(), (Texture)VirtualNode_getData(node));
		}
		
		__DELETE(this->textures);
		this->textures = NULL;
	}

	// destroy the super object
	__DESTROY_BASE(Sprite);
}

// load textures
static void MSprite_loadTextures(MSprite this)
{
	ASSERT(this, "MSprite::loadTextures: null this");

	if (this->mSpriteDefinition)
	{
		if(NULL == this->textures)
		{
			this->textures = __NEW(VirtualList);
		}
		
		int i = 0;
		
		for (; this->mSpriteDefinition->textureDefinitions[i]; i++)
	    {
			MSprite_loadTexture(this, this->mSpriteDefinition->textureDefinitions[i]);
		}
	}
}

// load texture
static void MSprite_loadTexture(MSprite this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MSprite::loadTexture: null this");

	ASSERT(textureDefinition, "MSprite::loadTexture: no sprite allocator defined");

	if (textureDefinition)
	{
		Texture texture = TextureManager_get(TextureManager_getInstance(), textureDefinition);
		
		ASSERT(texture, "MSprite::loadTexture: texture not loaded");
		
		if(texture)
		{
			VirtualList_pushBack(this->textures, texture);
		}
	}
}

// set sprite's position
void MSprite_setPosition(MSprite this, const VBVec3D* const position)
{
	ASSERT(this, "MSprite::setPosition: null this");

	// normalize the position to screen coordinates
	VBVec3D position3D = Optics_normalizePosition(position);

	ASSERT(this->texture, "MSprite::setPosition: null texture");

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;

	fix19_13 previousZPosition = this->drawSpec.position.z;

	// project position to 2D space
	Optics_projectTo2D(&this->drawSpec.position, &position3D);

	if (previousZPosition != this->drawSpec.position.z)
	{
		this->drawSpec.position.z = position->z;

		// calculate sprite's parallax
		Sprite_calculateParallax((Sprite)this, this->drawSpec.position.z);

		SpriteManager_spriteChangedPosition(SpriteManager_getInstance());
	}

	this->renderFlag |= __UPDATE_G;
}

// render a world layer with the map's information
void MSprite_render(MSprite this)
{
	ASSERT(this, "MSprite::render: null this");
	/*
#define	WRLD_1x1	0x0000
#define	WRLD_1x2	0x0100
#define	WRLD_1x4	0x0200
#define	WRLD_1x8	0x0300
#define	WRLD_2x1	0x0400
#define	WRLD_2x2	0x0500
#define	WRLD_2x4	0x0600
#define	WRLD_4x2	0x0900
#define	WRLD_4x1	0x0800
#define	WRLD_8x1	0x0C00
*/
	static int x = 0;
	
	Printing_int(Printing_getInstance(), x, 1, 10, NULL);
	//if render flag is set
	if (this->renderFlag)
	{
		DrawSpec drawSpec = this->drawSpec;
		WORLD* worldPointer = &WA[this->worldLayer];
		
		// if head is modified, render everything
		if (__UPDATE_HEAD == this->renderFlag)
		{
			//create an independant of software variable to point XPSTTS register
			unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

			//wait for screen to idle
			while (*xpstts & XPBSYR);

			worldPointer->head = this->head | WRLD_2x1 | this->mSpriteDefinition->scValue | Texture_getBgmapSegment(this->texture);
			worldPointer->mx = x++;
			worldPointer->mp = 0;
			worldPointer->my = this->texturePosition.y << 3;
			worldPointer->gx = 0;
			worldPointer->gp = drawSpec.position.parallax + this->parallaxDisplacement;
			worldPointer->gy = 0;

			//set the world size according to the zoom
			if (WRLD_AFFINE & this->head)
			{

				//				worldPointer->param = VPUManager_getParamDisplacement(VPUManager_getInstance(), this->param);
				worldPointer->w = ((int)Texture_getCols(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.x)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = ((int)Texture_getRows(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.y)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}

			this->renderFlag = false;

			return;
		}

		//create an independant of software variable to point XPSTTS register
		unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

		//wait for screen to idle
		while (*xpstts & XPBSYR);

		//set the world screen position
		if (this->renderFlag & __UPDATE_G )
		{
			worldPointer->mx = x++;
			worldPointer->mp = 0;
			worldPointer->my = this->texturePosition.y << 3;
			worldPointer->gx = 0;
			worldPointer->gp = drawSpec.position.parallax + this->parallaxDisplacement;
			worldPointer->gy = 0;
		}

		if (this->renderFlag & __UPDATE_SIZE)
		{
			//set the world size according to the zoom
			if (WRLD_AFFINE & this->head)
			{
				//				worldPointer->param = VPUManager_getParamDisplacement(VPUManager_getInstance(), this->param);
				worldPointer->w = ((int)Texture_getCols(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.x)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = ((int)Texture_getRows(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.y)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}
		}

		// make sure to not render again
		this->renderFlag = false;
	}
}

