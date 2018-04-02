/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBgmapSprite.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <Optics.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT		1
#define __GX_LIMIT							511
//#define __GY_LIMIT						511


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	MBgmapSprite
 * @extends BgmapSprite
 * @ingroup graphics-2d-sprites-bgmap
 * @brief	Sprite which holds a texture and a drawing specification.
 */
__CLASS_DEFINITION(MBgmapSprite, BgmapSprite);
__CLASS_FRIEND_DEFINITION(Texture);
__CLASS_FRIEND_DEFINITION(BgmapTexture);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
static void MBgmapSprite_releaseTextures(MBgmapSprite this);
static void MBgmapSprite_loadTextures(MBgmapSprite this);
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition);
static void MBgmapSprite_calculateSize(MBgmapSprite this);
//static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this);
//static void MBgmapSprite_calculateSize(MBgmapSprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBgmapSprite, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
__CLASS_NEW_END(MBgmapSprite, mBgmapSpriteDefinition, owner);

/**
 * Class constructor
 *
 * @memberof							MBgmapSprite
 * @public
 *
 * @param this							Function scope
 * @param mBgmapSpriteDefinition		Definition to use
 * @param owner							Sprite's owner
 */
void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
{
	ASSERT(this, "MBgmapSprite::constructor: null this");

	__CONSTRUCT_BASE(BgmapSprite, &mBgmapSpriteDefinition->bgmapSpriteDefinition, owner);

	this->mBgmapSpriteDefinition = mBgmapSpriteDefinition;

	ASSERT(!this->texture, "MBgmapSprite::constructor: texture already loaded");
	this->textures = NULL;
	MBgmapSprite_loadTextures(this);
	MBgmapSprite_calculateSize(this);
}

/**
 * Class destructor
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
void MBgmapSprite_destructor(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::destructor: null this");

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), __SAFE_CAST(BgmapSprite, this));
	}

	MBgmapSprite_releaseTextures(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Release the loaded textures
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
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

/**
 * Load textures from definition
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
static void MBgmapSprite_loadTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::loadTextures: null this");

	if(this->mBgmapSpriteDefinition)
	{
		if(!this->texture && !this->textures)
		{
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
		else
		{
			NM_ASSERT(this->texture, "MBgmapSprite::loadTextures: textures already loaded");
		}
	}
}

/**
 * Load a texture
 *
 * @memberof					MBgmapSprite
 * @public
 *
 * @param this					Function scope
 * @param textureDefinition		TextureDefinition to use
 */
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBgmapSprite::loadTexture: null this");

	ASSERT(textureDefinition, "MBgmapSprite::loadTexture: null textureDefinition");

	BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition);

	ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
	ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");

	VirtualList_pushBack(this->textures, bgmapTexture);
}

/**
 * Calculate 2D position
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		3D position
 * @param reproject		Force 3D to 2D projection
 */
void MBgmapSprite_position(MBgmapSprite this, const Vector3D* position, bool reproject)
{
	ASSERT(this, "MBgmapSprite::position: null this");

	bool haveToSetPosition = (!this->ready) | reproject;

	__CALL_BASE_METHOD(Sprite, position, this, position, reproject);

	if(haveToSetPosition)
	{
		PixelVector position2D = this->position;
    	MBgmapSprite_setPosition(this, &position2D);
	}
}

/**
 * Set 2D position
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		New 2D position
 */
void MBgmapSprite_setPosition(MBgmapSprite this, const PixelVector* position)
{
	ASSERT(this, "MBgmapSprite::setPosition: null this");

	__CALL_BASE_METHOD(BgmapSprite, setPosition, this, position);

	if(this->mBgmapSpriteDefinition->xLoop)
	{
		this->drawSpec.textureSource.mx = -position->x;
		this->position.x = 0;
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->position.x = position->x;
/*
		if(0 > position->x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= position->x + this->displacement.x;
		}
*/
	}

	if(this->mBgmapSpriteDefinition->yLoop)
	{
		this->position.y = 0;
		this->drawSpec.textureSource.my = -position->y;
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->position.y = position->y;
/*
		if(0 > position->y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= position->y + this->displacement.y;
		}
*/
	}

	fix10_6 previousZPosition = this->position.z;
	this->position.z = position->z;

	if(previousZPosition != this->position.z)
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(Sprite, calculateParallax, this, this->position.z);
	}
}

/**
 * Add displacement to position
 *
 * @memberof				MBgmapSprite
 * @public
 *
 * @param this				Function scope
 * @param displacement		2D position displacement
 */
void MBgmapSprite_addDisplacement(MBgmapSprite this, const PixelVector* displacement)
{
	ASSERT(this, "MBgmapSprite::addDisplacement: null this");
	this->ready = true;

	if(this->mBgmapSpriteDefinition->xLoop)
	{
		this->position.x = 0;
		this->drawSpec.textureSource.mx -= displacement->x;
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->position.x += displacement->x;
/*
		if(0 > this->position.x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= this->position.x + this->displacement.x;
		}
*/
	}

	if(this->mBgmapSpriteDefinition->yLoop)
	{
		this->position.y = 0;
		this->drawSpec.textureSource.my -= displacement->y;
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->position.y += displacement->y;
/*
		if(0 > this->position.y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= this->position.y + this->displacement.y;
		}
*/
	}

	this->position.z += displacement->z;
	this->position.parallax += displacement->parallax;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 * @param evenFrame
 */
void MBgmapSprite_render(MBgmapSprite this, bool evenFrame)
{
	ASSERT(this, "MBgmapSprite::render: null this");

	__CALL_BASE_METHOD(Sprite, render, this, evenFrame);

	if(!this->ready)
	{
		return;
	}

	// if render flag is set
	if(!this->texture | !this->worldLayer)
	{
		return;
	}

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

	// set the head
	worldPointer->head = this->head | (__SAFE_CAST(BgmapTexture, this->texture))->segment | this->mBgmapSpriteDefinition->scValue;

	int xPositionDisplacement = 0;
	int mxPositionDisplacement = 0;
	if(this->mBgmapSpriteDefinition->xLoop)
	{
    	mxPositionDisplacement = this->displacementRelativeToCamera.x;
	}
	else
	{
    	xPositionDisplacement = this->displacementRelativeToCamera.x;
	}

	int yPositionDisplacement = 0;
	int myPositionDisplacement = 0;
	if(this->mBgmapSpriteDefinition->yLoop)
	{
		myPositionDisplacement = this->displacementRelativeToCamera.y;
	}
	else
	{
		yPositionDisplacement = this->displacementRelativeToCamera.y;
	}

	// get coordinates
	int gx = this->position.x + this->displacement.x - this->halfWidth + xPositionDisplacement;
	int gy = this->position.y + this->displacement.y - this->halfHeight + yPositionDisplacement;
	worldPointer->gx = gx;
	worldPointer->gy = gy;

	int mxDisplacement = 0;
	if(_cameraFrustum->x0 > gx)
	{
		worldPointer->gx = _cameraFrustum->x0;
		mxDisplacement = _cameraFrustum->x0 - gx;
	}

	int myDisplacement = 0;
	if(_cameraFrustum->y0 > gy)
	{
		worldPointer->gy = _cameraFrustum->y0;
		myDisplacement = _cameraFrustum->y0 - gy;
	}

	worldPointer->gp = this->position.parallax + this->displacement.parallax;

	worldPointer->mx = this->drawSpec.textureSource.mx + mxDisplacement - mxPositionDisplacement;
	worldPointer->my = this->drawSpec.textureSource.my + myDisplacement - myPositionDisplacement;
	worldPointer->mp = this->drawSpec.textureSource.mp;

	// set the world size
	if(!this->mBgmapSpriteDefinition->xLoop)
	{
    	int w = (this->halfWidth << 1) - mxDisplacement;

		if(w + worldPointer->gx >= _cameraFrustum->x1)
		{
			w = _cameraFrustum->x1 - worldPointer->gx;
		}

		if(0 >= w)
		{
			worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
			worldPointer->w = 0;
			worldPointer->h = 0;
#endif
			return;
		}

		worldPointer->w = w - __WORLD_SIZE_DISPLACEMENT;
	}
	else
	{
		if(!_cameraFrustum->x0 && __SCREEN_WIDTH == _cameraFrustum->x1)
		{
			worldPointer->gx = worldPointer->gx - worldPointer->gp;
        	worldPointer->w = _cameraFrustum->x1 + (worldPointer->gp << 1) - _cameraFrustum->x0 - __WORLD_SIZE_DISPLACEMENT;
		}
		else
		{
			int w = _cameraFrustum->x1 - mxDisplacement - __WORLD_SIZE_DISPLACEMENT;

			if(0 > w || w + worldPointer->gx >= _cameraFrustum->x1)
			{
				w = _cameraFrustum->x1 - worldPointer->gx;
			}

			worldPointer->w = w;
		}
	}

	if(!this->mBgmapSpriteDefinition->yLoop)
	{
    	int h = (this->halfHeight << 1) - myDisplacement;

		if(h + worldPointer->gy >= _cameraFrustum->y1)
		{
			h = _cameraFrustum->y1 - worldPointer->gy;
		}

		if(0 >= h)
		{
			worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
			worldPointer->w = 0;
			worldPointer->h = 0;
#endif
			return;
		}

		worldPointer->h = h - __WORLD_SIZE_DISPLACEMENT;
	}
	else
	{
		int h = _cameraFrustum->y1 - myDisplacement - __WORLD_SIZE_DISPLACEMENT;

		if(0 > h || h + worldPointer->gy >= _cameraFrustum->y1)
		{
			h = _cameraFrustum->y1 - worldPointer->gy;
		}

		worldPointer->h = h;
	}

	BgmapSprite_processHbiasEffects(__SAFE_CAST(BgmapSprite, this));
}

/**
 * Resize
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void MBgmapSprite_resize(MBgmapSprite this, Scale scale, fix10_6 z)
{
	ASSERT(this, "MBgmapSprite::resize: null this");

	__CALL_BASE_METHOD(BgmapSprite, resize, this, scale, z);

	MBgmapSprite_calculateSize(this);
}

/**
 * Calculate Sprite's size
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 */
static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

	VirtualNode node = this->textures->head;

	int cols = 0;
	int rows = 0;

	for(; node; node = node->next)
	{
		// free the texture
		int textureCols = (__SAFE_CAST(Texture, node->data))->textureDefinition->cols;
		int textureRows = (__SAFE_CAST(Texture, node->data))->textureDefinition->rows;

		if(cols < textureCols)
		{
			cols = textureCols;
		}

		if(rows < textureRows)
		{
			rows = textureRows;
		}
	}

	this->halfWidth = cols << 2;
	this->halfHeight = rows << 2;
}

/**
 * Set Sprite's render mode
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void MBgmapSprite_setMode(MBgmapSprite this __attribute__ ((unused)), u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
{
	ASSERT(this, "MBgmapSprite::setMode: null this");
}

/**
 * Write textures
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			true it all textures are written
 */
bool MBgmapSprite_writeTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::writeTextures: null this");
	ASSERT(this->texture, "MBgmapSprite::writeTextures: null texture");

	VirtualNode node = this->textures->head;

	for(; node; node = node->next)
	{
		Texture texture = __SAFE_CAST(Texture, node->data);

		if(!texture->written)
		{
			__VIRTUAL_CALL(Texture, write, texture);
			break;
		}
	}

	return !node;
}

/**
 * Check if all textures are written
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			true it all textures are written
 */
bool MBgmapSprite_areTexturesWritten(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::areTexturesWritten: null this");

	if(!this->textures)
	{
		return true;
	}

	VirtualNode node = this->textures->head;

	for(; node; node = node->next)
	{
		if(!(__SAFE_CAST(Texture, node->data))->written)
		{
			return false;
		}
	}

	return true;
}

