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

friend class Texture;
friend class BgmapTexture;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof							MBgmapSprite
 * @public
 *
 * @param mBgmapSpriteSpec		Spec to use
 * @param owner							Sprite's owner
 */
void MBgmapSprite::constructor(const MBgmapSpriteSpec* mBgmapSpriteSpec, Object owner)
{
	Base::constructor(&mBgmapSpriteSpec->bgmapSpriteSpec, owner);

	this->mBgmapSpriteSpec = mBgmapSpriteSpec;

	ASSERT(!this->texture, "MBgmapSprite::constructor: texture already loaded");
	this->textures = NULL;
	MBgmapSprite::loadTextures(this);
	MBgmapSprite::calculateSize(this);
}

/**
 * Class destructor
 *
 * @memberof		MBgmapSprite
 * @public
 */
void MBgmapSprite::destructor()
{
	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), BgmapSprite::safeCast(this));
	}

	MBgmapSprite::releaseTextures(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Release the loaded textures
 *
 * @memberof		MBgmapSprite
 * @public
 */
void MBgmapSprite::releaseTextures()
{
	if(this->textures)
	{
		VirtualNode node = this->textures->head;

		for(; node; node = node->next)
		{
			// free the texture
			BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), BgmapTexture::safeCast(node->data));
		}

		delete this->textures;
		this->textures = NULL;
		this->texture = NULL;
	}
}

/**
 * Load textures from spec
 *
 * @memberof		MBgmapSprite
 * @public
 */
void MBgmapSprite::loadTextures()
{
	if(this->mBgmapSpriteSpec)
	{
		if(!this->texture && !this->textures)
		{
			this->textures = new VirtualList();

			int i = 0;

			for(; this->mBgmapSpriteSpec->textureSpecs[i]; i++)
			{
				MBgmapSprite::loadTexture(this, this->mBgmapSpriteSpec->textureSpecs[i], 0 == i);
			}

			this->texture = Texture::safeCast(VirtualList::front(this->textures));
			ASSERT(this->texture, "MBgmapSprite::loadTextures: null texture");

			this->textureXOffset = BgmapTexture::getXOffset(this->texture) << 3;
			this->textureYOffset = BgmapTexture::getYOffset(this->texture) << 3;
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
 * @param textureSpec		TextureSpec to use
 * @param isFirstTexture		To force loading in an even bgmap segment
 */
void MBgmapSprite::loadTexture(TextureSpec* textureSpec, bool isFirstTexture)
{
	ASSERT(textureSpec, "MBgmapSprite::loadTexture: null textureSpec");

	s16 minimumSegment = 0;

	if(VirtualList::getSize(this->textures))
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(VirtualList::back(this->textures));
		minimumSegment = BgmapTexture::getSegment(bgmapTexture);
	}

	BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), textureSpec, minimumSegment, isFirstTexture);

	ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
	ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");

	VirtualList::pushBack(this->textures, bgmapTexture);
}

/**
 * Calculate 2D position
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param position		3D position
 */
void MBgmapSprite::position(const Vector3D* position)
{
	Base::position(this, position);

	MBgmapSprite::setPosition(this, &this->position);
}

/**
 * Set 2D position
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param position		New 2D position
 */
void MBgmapSprite::setPosition(const PixelVector* position)
{
	PixelVector auxPosition = *position;

	Base::setPosition(this, position);

	if(this->mBgmapSpriteSpec->xLoop)
	{
		this->position.x = 0;
		this->drawSpec.textureSource.mx = -auxPosition.x;
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->position.x = auxPosition.x;
/*
		if(0 > auxPosition.x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= auxPosition.x + this->displacement.x;
		}
*/
	}

	if(this->mBgmapSpriteSpec->yLoop)
	{
		this->position.y = 0;
		this->drawSpec.textureSource.my = -auxPosition.y;
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->position.y = auxPosition.y;
/*
		if(0 > auxPosition.y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= auxPosition.y + this->displacement.y;
		}
*/
	}

	fix10_6 previousZPosition = this->position.z;
	this->position.z = auxPosition.z;

	if(previousZPosition != this->position.z)
	{
		// calculate sprite's parallax
		Sprite::calculateParallax(this, this->position.z);
	}
}

/**
 * Add displacement to position
 *
 * @memberof				MBgmapSprite
 * @public
 *
 * @param displacement		2D position displacement
 */
void MBgmapSprite::addDisplacement(const PixelVector* displacement)
{
	this->positioned = true;

	if(this->mBgmapSpriteSpec->xLoop)
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

	if(this->mBgmapSpriteSpec->yLoop)
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
 * @param evenFrame
 */
void MBgmapSprite::render(bool evenFrame __attribute__ ((unused)))
{
	if(!this->positioned)
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
	// when changing the texture spec
/*
	if(!this->texture->written)
	{
		worldPointer->head = 0x0000;
		return;
	}
*/

	// set the head
	worldPointer->head = this->head | (BgmapTexture::safeCast(this->texture))->segment | this->mBgmapSpriteSpec->scValue;

	// get coordinates
	int gx = this->position.x + this->displacement.x - this->halfWidth;
	int gy = this->position.y + this->displacement.y - this->halfHeight;
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

	worldPointer->mx = this->drawSpec.textureSource.mx + mxDisplacement;
	worldPointer->my = this->drawSpec.textureSource.my + myDisplacement;
	worldPointer->mp = this->drawSpec.textureSource.mp;

	// set the world size
	if(!this->mBgmapSpriteSpec->xLoop)
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

	if(!this->mBgmapSpriteSpec->yLoop)
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

	BgmapSprite::processHbiasEffects(this);
}

/**
 * Resize
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void MBgmapSprite::resize(Scale scale, fix10_6 z)
{
	Base::resize(this, scale, z);

	MBgmapSprite::calculateSize(this);
}

/**
 * Calculate Sprite's size
 *
 * @memberof			MBgmapSprite
 * @public
 */
void MBgmapSprite::calculateSize()
{
	VirtualNode node = this->textures->head;

	int cols = 0;
	int rows = 0;

	for(; node; node = node->next)
	{
		// free the texture
		int textureCols = (Texture::safeCast(node->data))->textureSpec->cols;
		int textureRows = (Texture::safeCast(node->data))->textureSpec->rows;

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
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void MBgmapSprite::setMode(u16 display, u16 mode __attribute__ ((unused)))
{
	this->head = display | __WORLD_BGMAP;
}

/**
 * Write textures
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @return			true it all textures are written
 */
bool MBgmapSprite::writeTextures()
{
	ASSERT(this->texture, "MBgmapSprite::writeTextures: null texture");

	VirtualNode node = this->textures->head;

	for(; node; node = node->next)
	{
		Texture texture = Texture::safeCast(node->data);

		if(!texture->written)
		{
			Texture::write(texture);
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
 * @return			true it all textures are written
 */
bool MBgmapSprite::areTexturesWritten()
{
	if(!this->textures)
	{
		return true;
	}

	VirtualNode node = this->textures->head;

	for(; node; node = node->next)
	{
		if(!(Texture::safeCast(node->data))->written)
		{
			return false;
		}
	}

	return true;
}

