/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

	this->checkIfWithinScreenSpace = false;

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
	// free the texture
	if(!isDeleted(this->texture))
	{
		// if affine or bgmap
		if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
		{
			// free param table space
			ParamTableManager::free(ParamTableManager::getInstance(), BgmapSprite::safeCast(this));
		}
	}

	this->texture = NULL;

	if(!isDeleted(this->textures))
	{
		VirtualNode node = this->textures->head;

		for(; node; node = node->next)
		{
			BgmapTexture bgmapTexture = BgmapTexture::safeCast(node->data);
			BgmapTexture::removeEventListener(bgmapTexture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
			BgmapTextureManager::releaseTextures(BgmapTextureManager::getInstance(), bgmapTexture);
		}

		delete this->textures;
		this->textures = NULL;
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

			for(int32 i = 0; this->mBgmapSpriteSpec->textureSpecs[i]; i++)
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

	int16 minimumSegment = 0;

	if(VirtualList::getSize(this->textures))
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(VirtualList::back(this->textures));
		minimumSegment = BgmapTexture::getSegment(bgmapTexture);
	}

	BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), textureSpec, minimumSegment, isFirstTextureAndHasMultipleTextures);

	ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
	ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");
	NM_ASSERT(!isFirstTextureAndHasMultipleTextures || 0 == (BgmapTexture::getSegment(bgmapTexture) % 2), "MBgmapSprite::loadTexture: first texture not loaded in even segment");

	BgmapTexture::addEventListener(bgmapTexture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);

	VirtualList::pushBack(this->textures, bgmapTexture);
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param evenFrame
 */
int16 MBgmapSprite::doRender(int16 index, bool evenFrame __attribute__((unused)))
{
	NM_ASSERT(!isDeleted(this->texture), "MBgmapSprite::doRender: null texture");

	TextureSource textureSource = this->drawSpec.textureSource;

	PixelVector position = this->position;

	if(this->mBgmapSpriteSpec->xLoop)
	{
		textureSource.mx = -this->position.x;
		position.x = 0;
	}
	else
 	{
 		textureSource.mx = this->textureXOffset;
	}

	if(this->mBgmapSpriteSpec->yLoop)
	{
		textureSource.my = -this->position.y;
		position.y = 0;
	}
	else
 	{
 		textureSource.my = this->textureYOffset;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	// get coordinates
	int16 gx = position.x + this->displacement.x - this->halfWidth;
	int16 gy = position.y + this->displacement.y - this->halfHeight;
	int16 gp = position.parallax + this->displacement.parallax;

	int32 mxDisplacement = 0;
	if(_cameraFrustum->x0 > gx)
	{
		mxDisplacement = _cameraFrustum->x0 - gx;
		gx = _cameraFrustum->x0;
	}

	int32 myDisplacement = 0;
	if(_cameraFrustum->y0 > gy)
	{
		myDisplacement = _cameraFrustum->y0 - gy;
		gy = _cameraFrustum->y0;
	}

	int16 mx = textureSource.mx + mxDisplacement;
	int16 my = textureSource.my + myDisplacement;
	int16 mp = textureSource.mp;

	int16 w = 0;
	int16 h = 0;

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
			return __NO_RENDER_INDEX;
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
				return __NO_RENDER_INDEX;
			}

			my -= __MINIMUM_BGMAP_SPRITE_HEIGHT - h;
		}
#else
		if (0 >= h)
		{
			return __NO_RENDER_INDEX;
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

	if(this->param)
	{
		MBgmapSprite::processHbiasEffects(this);
	}

	return index;
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

	int32 cols = 0;
	int32 rows = 0;

	for(; node; node = node->next)
	{
		// free the texture
		int32 textureCols = (Texture::safeCast(node->data))->textureSpec->cols;
		int32 textureRows = (Texture::safeCast(node->data))->textureSpec->rows;

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
void MBgmapSprite::setMode(uint16 display, uint16 mode __attribute__ ((unused)))
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

		if(kTexturePendingWriting == texture->status)
		{
			Texture::write(texture);
			break;
		}
	}

	return !node;
}


