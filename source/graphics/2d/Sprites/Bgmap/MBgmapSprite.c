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

static void MBgmapSprite_releaseTextures(MBgmapSprite this);
static void MBgmapSprite_loadTextures(MBgmapSprite this);
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition);
static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this);
static void MBgmapSprite_calculateSize(MBgmapSprite this);


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
		ASSERT(this->texture, "MBgmapSprite::loadTextures: null texture");

		this->textureXOffset = BgmapTexture_getXOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
		this->textureYOffset = BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
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
	
	VBVec2D position2D;
	
	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);
	
	MBgmapSprite_setPosition(this, &position2D);
}

void MBgmapSprite_setPosition(MBgmapSprite this, const VBVec2D* position)
{
	ASSERT(this, "MBgmapSprite::setPosition: null this");
	
	if(this->mSpriteDefinition->xLoop)
	{
		this->drawSpec.position.x = 0;
		this->drawSpec.textureSource.mx = FIX19_13TOI(-position->x);
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->drawSpec.position.x = position->x;

		if(0 > position->x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= FIX19_13TOI(position->x + this->displacement.x);
		}
	}

	if(this->mSpriteDefinition->yLoop)
	{
		this->drawSpec.position.y = 0;
		this->drawSpec.textureSource.my = FIX19_13TOI(-position->y);
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->drawSpec.position.y = position->y;

		if(0 > position->y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= FIX19_13TOI(position->y + this->displacement.y);
		}
	}
	
	fix19_13 previousZPosition = this->drawSpec.position.z;
	this->drawSpec.position.z = position->z;

	if(previousZPosition != this->drawSpec.position.z)
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(void, Sprite, calculateParallax, __SAFE_CAST(Sprite, this), this->drawSpec.position.z);
	}

	this->renderFlag |= __UPDATE_G | __UPDATE_M;
	this->initialized = true;
}

void MBgmapSprite_addDisplacement(MBgmapSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "MBgmapSprite::addDisplacement: null this");

	if(this->mSpriteDefinition->xLoop)
	{
		this->drawSpec.position.x = 0;
		this->drawSpec.textureSource.mx -= FIX19_13TOI(displacement->x + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->drawSpec.position.x += displacement->x;

		if(0 > this->drawSpec.position.x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
		}
	}
	
	if(this->mSpriteDefinition->yLoop)
	{
		this->drawSpec.position.y = 0;
		this->drawSpec.textureSource.my -= FIX19_13TOI(displacement->y + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->drawSpec.position.y += displacement->y;

		if(0 > this->drawSpec.position.y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);
		}
	}
	
	this->drawSpec.position.z += displacement->z;
	this->drawSpec.position.parallax += displacement->parallax;

	this->renderFlag |= __UPDATE_G;
	this->renderFlag |= __UPDATE_M;
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
	ASSERT(this, "MBgmapSprite::render: null this");

	// if render flag is set
	if(this->texture && this->renderFlag && this->initialized)
	{
		if(this->hidden)
		{
			WORLD_HEAD(this->worldLayer, 0x0000);
			this->renderFlag = 0;
			return;
		}

		static WORLD* worldPointer = NULL;
		worldPointer = &WA[this->worldLayer];

		if(__UPDATE_HEAD == this->renderFlag)
		{
			int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
			worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : gx < 0? 0: gx;
			worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);
			int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);
			worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : gy < 0? 0: gy;

			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;

			// set the world size
			if(!this->mSpriteDefinition->xLoop)
			{
				int w = (((int)Texture_getCols(this->texture))<< 3) - 1 - (worldPointer->mx - this->textureXOffset);
				worldPointer->w = w + worldPointer->gx > __SCREEN_WIDTH? __SCREEN_WIDTH - worldPointer->gx: 0 > w? 0: w;
			}
			else
			{
				worldPointer->gx -= (this->drawSpec.position.parallax);
				worldPointer->w = __SCREEN_WIDTH + (this->drawSpec.position.parallax << 1);
			}

			if(!this->mSpriteDefinition->yLoop)
			{
				int h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->textureYOffset);
				worldPointer->h = h + worldPointer->gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy: 0 > h? 0: h;
			}
			else
			{
				worldPointer->h = __SCREEN_HEIGHT;
			}

			worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__SAFE_CAST(BgmapTexture, this->texture));
			this->renderFlag = false;
			return;
		}
		
		// set the world screen position
		if(this->renderFlag & (__UPDATE_G | __UPDATE_M))
		{
			int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
			worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : gx < 0? 0: gx;
			worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);
			int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);
			worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : gy < 0? 0: gy;

			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;

			// set the world size
			if(!this->mSpriteDefinition->xLoop)
			{
				int w = (((int)Texture_getCols(this->texture))<< 3) - 1 - (worldPointer->mx - this->textureXOffset);
				worldPointer->w = w + worldPointer->gx > __SCREEN_WIDTH? __SCREEN_WIDTH - worldPointer->gx: 0 > w? 0: w;
			}
			else
			{
				worldPointer->gx -= (this->drawSpec.position.parallax);
				worldPointer->w = __SCREEN_WIDTH - 1 + (this->drawSpec.position.parallax << 1);
			}

			if(!this->mSpriteDefinition->yLoop)
			{
				int h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->textureYOffset);
				worldPointer->h = h + worldPointer->gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy: 0 > h? 0: h;
			}
			else
			{
				worldPointer->h = __SCREEN_HEIGHT - 1;
			}
		}

		// make sure to not render again
		this->renderFlag = false;
	}
}

VBVec2D MBgmapSprite_getPosition(MBgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getPosition: null this");
	
	return this->drawSpec.position;
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

