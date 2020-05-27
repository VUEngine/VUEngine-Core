/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
			BgmapTexture bgmapTexture = BgmapTexture::safeCast(node->data);
			BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), bgmapTexture);
			BgmapTexture::removeEventListener(bgmapTexture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
		}

		delete this->textures;
		this->textures = NULL;
		this->texture = NULL;
	}
}

void MBgmapSprite::releaseTexture()
{
	MBgmapSprite::releaseTextures(this);

	this->texture = NULL;
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
				MBgmapSprite::loadTexture(this, this->mBgmapSpriteSpec->textureSpecs[i], 0 == i && this->mBgmapSpriteSpec->textureSpecs[i + 1]);
			}

			this->texture = Texture::safeCast(VirtualList::front(this->textures));
			NM_ASSERT(this->texture, "MBgmapSprite::loadTextures: null texture");

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
 * @memberof										MBgmapSprite
 * @public
 *
 * @param textureSpec								TextureSpec to use
 * @param isFirstTextureAndHasMultipleTextures		To force loading in an even bgmap segment for the first texture
 */
void MBgmapSprite::loadTexture(TextureSpec* textureSpec, bool isFirstTextureAndHasMultipleTextures)
{
	ASSERT(textureSpec, "MBgmapSprite::loadTexture: null textureSpec");

	s16 minimumSegment = 0;

	if(VirtualList::getSize(this->textures))
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(VirtualList::back(this->textures));
		minimumSegment = BgmapTexture::getSegment(bgmapTexture);
	}

	BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), textureSpec, minimumSegment, isFirstTextureAndHasMultipleTextures);

	ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
	ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");

	BgmapTexture::addEventListener(bgmapTexture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);

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
bool MBgmapSprite::doRender(u16 index __attribute__((unused)), bool evenFrame __attribute__((unused)))
{
	static WorldAttributes* worldPointer = NULL;
	worldPointer = &_worldAttributesBaseAddress[this->index];

	// TODO: check if required, causes that the sprite is turned off
	// when changing the texture spec
/*
	if(!this->texture->written)
	{
		worldPointer->head = 0x0000;
		return;
	}
*/

	// get coordinates
	s16 gx = this->position.x + this->displacement.x - this->halfWidth;
	s16 gy = this->position.y + this->displacement.y - this->halfHeight;
	s16 gp = this->position.parallax + this->displacement.parallax;

	int mxDisplacement = 0;
	if(_cameraFrustum->x0 > gx)
	{
		mxDisplacement = _cameraFrustum->x0 - gx;
		gx = _cameraFrustum->x0;
	}

	int myDisplacement = 0;
	if(_cameraFrustum->y0 > gy)
	{
		myDisplacement = _cameraFrustum->y0 - gy;
		gy = _cameraFrustum->y0;
	}

	s16 mx = this->drawSpec.textureSource.mx + mxDisplacement;
	s16 my = this->drawSpec.textureSource.my + myDisplacement;
	s16 mp = this->drawSpec.textureSource.mp;

	s16 w = 0;
	s16 h = 0;

	// set the world size
	if(!this->mBgmapSpriteSpec->xLoop)
	{
    	w = (this->halfWidth << 1) - mxDisplacement;

		if(w + gx >= _cameraFrustum->x1)
		{
			w = _cameraFrustum->x1 - gx;
		}

		if(0 >= w)
		{
#ifdef __PROFILE_GAME
			worldPointer->w = 0;
			worldPointer->h = 0;
#endif
			return false;
		}
	}
	else
	{
		mp = - gp;
		gp = 0;
		gx = _cameraFrustum->x0;
		w = _cameraFrustum->x1 - _cameraFrustum->x0 - __WORLD_SIZE_DISPLACEMENT;
	}

	if(!this->mBgmapSpriteSpec->yLoop)
	{
    	h = (this->halfHeight << 1) - myDisplacement;

		if(h + gy >= _cameraFrustum->y1)
		{
			h = _cameraFrustum->y1 - gy;
		}

#ifdef __HACK_BGMAP_SPRITE_HEIGHT
		if (__MINIMUM_BGMAP_SPRITE_HEIGHT >= h)
		{
			if (0 >= h)
			{
				worldPointer->head = __WORLD_OFF;

#ifdef __PROFILE_GAME
				worldPointer->w = 0;
				worldPointer->h = 0;
#endif
				return false;
			}

			my -= __MINIMUM_BGMAP_SPRITE_HEIGHT - h;
		}
#else
		if (0 >= h)
		{
#ifdef __PROFILE_GAME
			worldPointer->w = 0;
			worldPointer->h = 0;
#endif
			return false;
		}
#endif
	}
	else
	{
		h = _cameraFrustum->y1 - myDisplacement - __WORLD_SIZE_DISPLACEMENT;

		if(0 > h || h + gy >= _cameraFrustum->y1)
		{
			h = _cameraFrustum->y1 - gy - __WORLD_SIZE_DISPLACEMENT;
		}
	}

	worldPointer->gx = gx;
	worldPointer->gy = gy;
	worldPointer->gp = gp;

	worldPointer->mx = mx;
	worldPointer->my = my;
	worldPointer->mp = mp;

	worldPointer->w = w - __WORLD_SIZE_DISPLACEMENT;
	worldPointer->h = h - __WORLD_SIZE_DISPLACEMENT;

	worldPointer->head = this->head | (BgmapTexture::safeCast(this->texture))->segment | this->mBgmapSpriteSpec->scValue;

	BgmapSprite::processHbiasEffects(this);

	return true;
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

	if(0 < this->mBgmapSpriteSpec->width)
	{
		this->halfWidth = this->mBgmapSpriteSpec->width >> 1;
	}

	if(0 < this->mBgmapSpriteSpec->height)
	{
		this->halfHeight = this->mBgmapSpriteSpec->height >> 1;
	}
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

