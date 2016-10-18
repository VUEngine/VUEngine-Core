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
#include <BgmapTextureManager.h>
#include <Optics.h>
#include <Screen.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT		1
#define __GX_LIMIT							511
//#define __GY_LIMIT						511


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class       MBgmapSprite
 * @extends     BgmapSprite
 * @brief       Sprite which holds a texture and a drawing specification.
 */

__CLASS_DEFINITION(MBgmapSprite, BgmapSprite);

__CLASS_FRIEND_DEFINITION(Texture);
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
static void MBgmapSprite_calculateSize(MBgmapSprite this);
//static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this);
//static void MBgmapSprite_calculateSize(MBgmapSprite this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBgmapSprite, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
__CLASS_NEW_END(MBgmapSprite, mBgmapSpriteDefinition, owner);

// class's constructor
void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
{
	__CONSTRUCT_BASE(BgmapSprite, &mBgmapSpriteDefinition->bgmapSpriteDefinition, owner);

	this->mBgmapSpriteDefinition = mBgmapSpriteDefinition;

	ASSERT(!this->texture, "MBgmapSprite::constructor: texture already loaded");
	this->textures = NULL;
	MBgmapSprite_loadTextures(this);
	MBgmapSprite_calculateSize(this);

	// TODO: check if this is really needed
/*
	// assume looping
	this->size.x = 0;
	this->size.y = 0;

	this->sizeMultiplier.x = 1;
	this->sizeMultiplier.y = 1;

	MBgmapSprite_calculateSize(this);
*/
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

	if(this->mBgmapSpriteDefinition)
	{
		MBgmapSprite_releaseTextures(this);
		this->textures = __NEW(VirtualList);

		int i = 0;

		for(; this->mBgmapSpriteDefinition->textureDefinitions[i]; i++)
	    {
			MBgmapSprite_loadTexture(this, this->mBgmapSpriteDefinition->textureDefinitions[i]);
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

	ASSERT(textureDefinition, "MBgmapSprite::loadTexture: null textureDefinition");

    BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition);

    ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
    ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");

    VirtualList_pushBack(this->textures, bgmapTexture);
}

// set sprite's position
void MBgmapSprite_position(MBgmapSprite this, const VBVec3D* position)
{
	ASSERT(this, "MBgmapSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	VBVec2D position2D;

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	position2D.x -= this->halfWidth;
	position2D.y -= this->halfHeight;

	MBgmapSprite_setPosition(this, &position2D);
}

void MBgmapSprite_setPosition(MBgmapSprite this, const VBVec2D* position)
{
	ASSERT(this, "MBgmapSprite::setPosition: null this");

	if(this->mBgmapSpriteDefinition->xLoop)
	{
		this->drawSpec.position.x = 0;
		this->drawSpec.textureSource.mx = -FIX19_13TOI(position->x + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->drawSpec.position.x = position->x;

		if(0 > position->x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= FIX19_13TOI(position->x + this->displacement.x + __0_5F_FIX19_13);
		}
	}

	if(this->mBgmapSpriteDefinition->yLoop)
	{
		this->drawSpec.position.y = 0;
		this->drawSpec.textureSource.my = -FIX19_13TOI(position->y + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->drawSpec.position.y = position->y;

		if(0 > position->y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= FIX19_13TOI(position->y + this->displacement.y + __0_5F_FIX19_13);
		}
	}

	fix19_13 previousZPosition = this->drawSpec.position.z;
	this->drawSpec.position.z = position->z;

	if(previousZPosition != this->drawSpec.position.z)
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(Sprite, calculateParallax, __SAFE_CAST(Sprite, this), this->drawSpec.position.z);
	}

	this->renderFlag |= __UPDATE_G | __UPDATE_M;

	if(!this->worldLayer)
	{
		// register with sprite manager
    	Sprite_setWorldLayer(__SAFE_CAST(Sprite, this), SpriteManager_getWorldLayer(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this)));
    }
}

void MBgmapSprite_addDisplacement(MBgmapSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "MBgmapSprite::addDisplacement: null this");

	if(this->mBgmapSpriteDefinition->xLoop)
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
			this->drawSpec.textureSource.mx -= FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
		}
	}

	if(this->mBgmapSpriteDefinition->yLoop)
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
			this->drawSpec.textureSource.my -= FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);
		}
	}

	this->drawSpec.position.z += displacement->z;
	this->drawSpec.position.parallax += displacement->parallax;

	this->renderFlag |= __UPDATE_G;
	this->renderFlag |= __UPDATE_M;
}

// render a world layer with the map's information
void MBgmapSprite_render(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::render: null this");

	// if render flag is set
	if(this->texture && this->renderFlag && this->worldLayer)
	{
		static WorldAttributes* worldPointer = NULL;
		worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		// TODO: check if required, causes that the sprite is turned off
		// when changing the texture definition
/*
        if(!this->texture->written)
        {
            worldPointer->head = 0x0000;
            return;
        }
*/
		if(this->hidden)
		{
			worldPointer->head = 0x0000;
            this->renderFlag = false;
			return;
		}
		else
		{
            worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__SAFE_CAST(BgmapTexture, this->texture));
		}

        int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
        int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);

        worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : gx < 0? 0: gx;
        worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : gy < 0? 0: gy;

        worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);

        worldPointer->mx = this->drawSpec.textureSource.mx;
        worldPointer->my = this->drawSpec.textureSource.my;
        worldPointer->mp = this->drawSpec.textureSource.mp;

        // set the world size
        if(!this->mBgmapSpriteDefinition->xLoop)
        {
            int w = (((int)Texture_getCols(this->texture))<< 3) - 1 - (worldPointer->mx - this->textureXOffset);
            worldPointer->w = w + worldPointer->gx > __SCREEN_WIDTH? __SCREEN_WIDTH - worldPointer->gx: 0 > w? 0: w;

            if(!worldPointer->w)
            {
    			worldPointer->head = 0x0000;
                this->renderFlag = false;
                return;
            }
        }
        else
        {
            worldPointer->gx -= (this->drawSpec.position.parallax);
            worldPointer->w = __SCREEN_WIDTH + (this->drawSpec.position.parallax << 1);
        }

        if(!this->mBgmapSpriteDefinition->yLoop)
        {
            int h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->textureYOffset);
            worldPointer->h = h + worldPointer->gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy: 0 > h? 0: h;

            if(!worldPointer->h)
            {
    			worldPointer->head = 0x0000;
                this->renderFlag = false;
                return;
            }
        }
        else
        {
            worldPointer->h = __SCREEN_HEIGHT;
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

void MBgmapSprite_resize(MBgmapSprite this, Scale scale, fix19_13 z)
{
	ASSERT(this, "MBgmapSprite::resize: null this");

    BgmapSprite_resize(__SAFE_CAST(BgmapSprite, this), scale, z);

    MBgmapSprite_calculateSize(this);
}

static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

    VirtualNode node = this->textures->head;

    int cols = 0;
    int rows = 0;

    for(; node; node = node->next)
    {
        // free the texture
        int textureCols = Texture_getCols(__SAFE_CAST(Texture, node->data));
        int textureRows = Texture_getRows(__SAFE_CAST(Texture, node->data));

        if(cols < textureCols)
        {
            cols = textureCols;
        }

        if(rows < textureRows)
        {
            rows = textureRows;
        }
    }

    this->halfWidth = ITOFIX19_13(cols << 2);
    this->halfHeight = ITOFIX19_13(rows << 2);
}

/*
// calculate total sprite's size
static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

	MBgmapSprite_calculateSizeMultiplier(this);

	Texture texture = __SAFE_CAST(Texture, VirtualList_front(this->textures));

	if(!this->mBgmapSpriteDefinition->xLoop)
	{
		this->size.x = this->sizeMultiplier.x << 9;
	}

	if(!this->mBgmapSpriteDefinition->yLoop)
	{
		this->size.y = ((texture ? Texture_getRows(texture) : 64) << 3) * this->sizeMultiplier.x;
	}
}


// calculate the size multiplier
static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSizeMultiplier: null this");

	switch(this->mBgmapSpriteDefinition->scValue)
	{
		case __WORLD_1x1:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_1x2:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_1x4:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 4;
			break;

		case __WORLD_1x8:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 8;
			break;

		case __WORLD_2x1:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_2x2:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_2x4:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 4;
			break;

		case __WORLD_4x1:

			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_4x2:

			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_8x1:

			this->sizeMultiplier.x = 8;
			this->sizeMultiplier.y = 1;
			break;
	}
}
*/
