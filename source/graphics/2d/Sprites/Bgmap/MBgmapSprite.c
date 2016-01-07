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

#include <MBgmapSprite.h>
#include <SpriteManager.h>
#include <Optics.h>
#include <Screen.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1
#define __GX_LIMIT						511
//#define __GY_LIMIT						511


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(MBgmapSprite, BgmapSprite);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern Optical* _optical;
extern unsigned int volatile* _xpstts;

static void MBgmapSprite_releaseTextures(MBgmapSprite this);
static void MBgmapSprite_loadTextures(MBgmapSprite this);
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition);
static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this);
static void MBgmapSprite_calculateSize(MBgmapSprite this);
static const Point* const MBgmapSprite_capPosition(MBgmapSprite this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBgmapSprite, const MBgmapSpriteDefinition* mSpriteDefinition, Object owner)
__CLASS_NEW_END(MBgmapSprite, mSpriteDefinition, owner);

// class's constructor
void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mSpriteDefinition, Object owner)
{
	__CONSTRUCT_BASE(&mSpriteDefinition->bSpriteDefinition, owner);
	
	this->mSpriteDefinition = mSpriteDefinition;

	ASSERT(!this->texture, "MBgmapSprite::constructor: texture alrea");
	this->textures = NULL;
	MBgmapSprite_loadTextures(this);
	
	// assume looping
	this->size.x = 0;
	this->size.y = 0;

	this->sizeMultiplier.x = 1;
	this->sizeMultiplier.y = 1;
	
	MBgmapSprite_calculateSize(this);
}

// class's destructor
void MBgmapSprite_destructor(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::destructor: null this");

	MBgmapSprite_releaseTextures(this);
	this->texture = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// release loaded textures
static void MBgmapSprite_releaseTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::releaseTextures: null this");

	if(this->textures)
	{
		VirtualNode node = this->textures->head;
		
		for(; node; node = node->next)
		{
			// free the texture
			BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, node->data));
		}
		
		__DELETE(this->textures);
		this->textures = NULL;
		this->texture = NULL;
	}
}

// load textures
static void MBgmapSprite_loadTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::loadTextures: null this");

	if(this->mSpriteDefinition)
	{
		MBgmapSprite_releaseTextures(this);
		this->textures = __NEW(VirtualList);
		
		int i = 0;
		
		for(; this->mSpriteDefinition->textureDefinitions[i]; i++)
	    {
			MBgmapSprite_loadTexture(this, this->mSpriteDefinition->textureDefinitions[i]);
		}
		
		this->texture = __SAFE_CAST(Texture, VirtualList_front(this->textures));
	}
}

// load texture
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBgmapSprite::loadTexture: null this");

	ASSERT(textureDefinition, "MBgmapSprite::loadTexture: no sprite allocator defined");

	if(textureDefinition)
	{
		BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition);
		
		ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
		
		if(bgmapTexture && this->textures)
		{
			VirtualList_pushBack(this->textures, bgmapTexture);
		}
	}
}

// set sprite's position
void MBgmapSprite_position(MBgmapSprite this, const VBVec3D* position)
{
	ASSERT(this, "MBgmapSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;
	
	fix19_13 previousZPosition = this->drawSpec.position.z;

	VBVec3D position2D;
	
	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);
	
	this->drawSpec.position.x = 0;
	this->drawSpec.position.y = 0;
	this->drawSpec.position.z = position->z;

	this->drawSpec.textureSource.mx = (int)(0.5f + FIX19_13TOF(-position2D.x));
	this->drawSpec.textureSource.my = (int)(0.5f + FIX19_13TOF(-position2D.y));

//	this->drawSpec.textureSource.mx = FIX19_13TOI(-position2D.x);
//	this->drawSpec.textureSource.my = FIX19_13TOI(-position2D.y);
		
	if(previousZPosition != this->drawSpec.position.z)
	{
		this->drawSpec.position.z = position3D.z;

		// calculate sprite's parallax
		__VIRTUAL_CALL(void, Sprite, calculateParallax, __SAFE_CAST(Sprite, this), this->drawSpec.position.z);
	}

	const Point* const axisCapped = MBgmapSprite_capPosition(this);
	
	if(axisCapped->x)
	{
		this->drawSpec.position.x = ITOFIX19_13(-axisCapped->x);
		this->renderFlag |= __UPDATE_G;
	}
	
	if(axisCapped->y)
	{
		this->drawSpec.position.y = ITOFIX19_13(-axisCapped->y);
		this->renderFlag |= __UPDATE_G;
	}
	
	this->renderFlag |= __UPDATE_M;

	this->drawSpec.textureSource.my += 1 == this->sizeMultiplier.y? BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3: 0;
}

void MBgmapSprite_setPosition(MBgmapSprite this, const VBVec2D* position)
{
	ASSERT(this, "MBgmapSprite::setPosition: null this");
	
	this->drawSpec.position.x = 0;
	this->drawSpec.position.y = 0;

	fix19_13 previousZPosition = this->drawSpec.position.z;
	this->drawSpec.position.z = position->z;

	this->drawSpec.textureSource.mx = FIX19_13TOI(-position->x);
	this->drawSpec.textureSource.my = FIX19_13TOI(-position->y);
		
	if(previousZPosition != this->drawSpec.position.z)
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(void, Sprite, calculateParallax, __SAFE_CAST(Sprite, this), this->drawSpec.position.z);
	}

	const Point* const axisCapped = MBgmapSprite_capPosition(this);
	
	if(axisCapped->x)
	{
		this->drawSpec.position.x = ITOFIX19_13(-axisCapped->x);
		this->renderFlag |= __UPDATE_G;
	}
	
	if(axisCapped->y)
	{
		this->drawSpec.position.y = ITOFIX19_13(-axisCapped->y);
		this->renderFlag |= __UPDATE_G;
	}
	
	this->renderFlag |= __UPDATE_M;

	this->drawSpec.textureSource.my += 1 == this->sizeMultiplier.y? BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3: 0;

}

// calculate the size multiplier
static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSizeMultiplier: null this");

	switch(this->mSpriteDefinition->scValue)
	{
		case WRLD_1x1:
			
			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 1;
			break;

		case WRLD_1x2:
			
			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 2;
			break;

		case WRLD_1x4:
			
			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 4;
			break;
			
		case WRLD_1x8:
			
			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 8;
			break;

		case WRLD_2x1:
			
			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 1;
			break;
			
		case WRLD_2x2:
			
			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 2;
			break;

		case WRLD_2x4:
			
			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 4;
			break;

		case WRLD_4x1:
			
			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 1;
			break;

		case WRLD_4x2:
			
			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 2;
			break;
			
		case WRLD_8x1:
			
			this->sizeMultiplier.x = 8;
			this->sizeMultiplier.y = 1;
			break;
	}
}

// render a world layer with the map's information
void MBgmapSprite_render(MBgmapSprite this)
{
	ASSERT(this, "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

	// if render flag is set
	if(this->renderFlag)
	{
		static WORLD* worldPointer = NULL;
		worldPointer = &WA[this->worldLayer];

		ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "BgmapSprite::render: freeLayer >= this->worldLayer");

		if(__UPDATE_HEAD == this->renderFlag)
		{
			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;
			int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
			worldPointer->gx = gx > __GX_LIMIT? __GX_LIMIT : gx < -__GX_LIMIT? -__GX_LIMIT : gx;
			worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);
			worldPointer->gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);

			// set the world size according to the zoom
			if(!this->mSpriteDefinition->xLoop)
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - 1 - worldPointer->mx;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - 1;
			}
			
			if(!this->mSpriteDefinition->yLoop)
			{
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->drawSpec.textureSource.mx);
			}
			else
			{
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - 1;
			}
			// make sure to not render again
			while (*_xpstts & XPBSYR);
			worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__SAFE_CAST(BgmapTexture, this->texture));
			this->renderFlag = 0 < this->paramTableRow? __UPDATE_SIZE: false;
			return;
		}
		
		// set the world screen position
		if(this->renderFlag & __UPDATE_M)
		{
			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;

			// set the world size according to the zoom
			if(!this->mSpriteDefinition->xLoop)
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - 1 - worldPointer->mx;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3) - 1;
			}
			
			if(!this->mSpriteDefinition->yLoop)
			{
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->drawSpec.textureSource.mx);
			}
			else
			{
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - 1;
			}
			
			worldPointer->h = (((int)Texture_getRows(this->texture))<< 3) - 1;
		}
		
		// set the world screen position
		if(this->renderFlag & __UPDATE_G)
		{
			int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
			worldPointer->gx = gx > __GX_LIMIT? __GX_LIMIT : gx < -__GX_LIMIT? -__GX_LIMIT : gx;
			worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);
			worldPointer->gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);
		}

		// make sure to not render again
		this->renderFlag = false;
	}
}

// calculate total sprite's size
static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

	MBgmapSprite_calculateSizeMultiplier(this);
	
	Texture texture = __SAFE_CAST(Texture, VirtualList_front(this->textures));
	
	if(!this->mSpriteDefinition->xLoop)
	{
		this->size.x = 64 * 8 * this->sizeMultiplier.x;
	}

	if(!this->mSpriteDefinition->yLoop)
	{
		this->size.y = (texture? Texture_getRows(texture): 64) * 8 * this->sizeMultiplier.x;
	}
}

// calculate the position
static const Point* const MBgmapSprite_capPosition(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::capPosition: null this");

	static Point axisCapped = 
	{
		0, 0
	};
	
	axisCapped.x = 0;
	axisCapped.y = 0;

	if(!this->mSpriteDefinition->xLoop)
	{
		if(this->drawSpec.textureSource.mx > this->size.x - __SCREEN_WIDTH)
		{
			axisCapped.x = this->drawSpec.textureSource.mx - (this->size.x - __SCREEN_WIDTH);
			this->drawSpec.textureSource.mx = this->size.x - __SCREEN_WIDTH;
		}
		else if(0 > this->drawSpec.textureSource.mx)
		{
			axisCapped.x = this->drawSpec.textureSource.mx;
			this->drawSpec.textureSource.mx = 0;
		}
	}

	if(!this->mSpriteDefinition->yLoop)
	{
		int height = Texture_getRows(this->texture) << 3;
		
		if(this->drawSpec.textureSource.my > this->size.y - height)
		{
			axisCapped.y = this->drawSpec.textureSource.my - (this->size.y - height);
			this->drawSpec.textureSource.my = this->size.y - height;
		}
		else if(0 > this->drawSpec.textureSource.my)
		{
			axisCapped.y = this->drawSpec.textureSource.my;
			this->drawSpec.textureSource.my = 0;
		}
	}

	return &axisCapped;
}


