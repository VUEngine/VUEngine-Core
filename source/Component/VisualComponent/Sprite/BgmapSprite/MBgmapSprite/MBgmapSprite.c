/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTexture.h>
#include <DebugConfig.h>
#include <ParamTableManager.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "MBgmapSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class BgmapTexture;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ACCOUNT_FOR_BGMAP_PLACEMENT		1
#define __GX_LIMIT							511

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::constructor(Entity owner, const MBgmapSpriteSpec* mBgmapSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &mBgmapSpriteSpec->bgmapSpriteSpec);

	this->checkIfWithinScreenSpace = false;

	if(!isDeleted(this->texture))
	{
		Texture::release(this->texture);
		this->texture = NULL;
	}

	this->textures = NULL;
	MBgmapSprite::loadTextures(this);
	MBgmapSprite::calculateSize(this);

	if(0 == (__WORLD_BGMAP & this->head) && 0 == this->param)
	{
		MBgmapSprite::setMode(this, mBgmapSpriteSpec->bgmapSpriteSpec.display, mBgmapSpriteSpec->bgmapSpriteSpec.bgmapMode);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::destructor()
{
	MBgmapSprite::releaseTextures(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::releaseResources()
{
	MBgmapSprite::releaseTextures(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 MBgmapSprite::doRender(int16 index)
{
	NM_ASSERT(!isDeleted(this->texture), "MBgmapSprite::doRender: null texture");

	BgmapTextureSource bgmapTextureSource = this->bgmapTextureSource;

	PixelVector position = this->position;

	if(((MBgmapSpriteSpec*)this->componentSpec)->xLoop)
	{
		bgmapTextureSource.mx = -this->position.x;
		position.x = 0;
	}
	else
 	{
 		bgmapTextureSource.mx = this->textureXOffset;
	}

	if(((MBgmapSpriteSpec*)this->componentSpec)->yLoop)
	{
		bgmapTextureSource.my = -this->position.y;
		position.y = 0;
	}
	else
 	{
 		bgmapTextureSource.my = this->textureYOffset;
	}

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	// Get coordinates
	int16 gx = position.x - this->halfWidth;
	int16 gy = position.y - this->halfHeight;
	int16 gp = position.parallax;

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

	int16 mx = bgmapTextureSource.mx + mxDisplacement;
	int16 my = bgmapTextureSource.my + myDisplacement;
	int16 mp = bgmapTextureSource.mp;

	int16 w = 0;
	int16 h = 0;

	// Set the world size
	if(!((MBgmapSpriteSpec*)this->componentSpec)->xLoop)
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

	if(!((MBgmapSpriteSpec*)this->componentSpec)->yLoop)
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

	worldPointer->mx = mx - this->displacement.x;
	worldPointer->my = my - this->displacement.y;
	worldPointer->mp = mp - this->displacement.parallax;

	worldPointer->w = w - __WORLD_SIZE_DISPLACEMENT;
	worldPointer->h = h - __WORLD_SIZE_DISPLACEMENT;

	worldPointer->head = this->head | (BgmapTexture::safeCast(this->texture))->segment | ((MBgmapSpriteSpec*)this->componentSpec)->scValue;

	if(0 < this->param)
	{
		worldPointer->param = (uint16)((((this->param)) - 0x20000) >> 1) & 0xFFF0;
	}

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::setMultiframe(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture);
	int16 my = BgmapTexture::getYOffset(this->texture);
	int32 totalColumns = 64 - mx;
	int32 frameColumn = Texture::getCols(this->texture) * frame;
	this->bgmapTextureSource.mx = (mx + (frameColumn % totalColumns)) << 3;
	this->bgmapTextureSource.my = (my + (frameColumn % totalColumns)) << 3;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::loadTextures()
{
	if(NULL != ((MBgmapSpriteSpec*)this->componentSpec))
	{
		if(NULL == this->texture && NULL == this->textures)
		{
			this->textures = new VirtualList();

			for(int32 i = 0; NULL != ((MBgmapSpriteSpec*)this->componentSpec)->textureSpecs[i]; i++)
			{
				MBgmapSprite::loadTexture(this, ((MBgmapSpriteSpec*)this->componentSpec)->textureSpecs[i], 0 == i && ((MBgmapSpriteSpec*)this->componentSpec)->textureSpecs[i + 1]);
			}

			this->textureXOffset = BgmapTexture::getXOffset(this->texture) << 3;
			this->textureYOffset = BgmapTexture::getYOffset(this->texture) << 3;
		}
		else
		{
			NM_ASSERT(this->texture, "MBgmapSprite::loadTextures: textures already loaded");
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::loadTexture(TextureSpec* textureSpec, bool isFirstTextureAndHasMultipleTextures)
{
	ASSERT(textureSpec, "MBgmapSprite::loadTexture: null textureSpec");

	int16 minimumSegment = 0;

	if(VirtualList::getCount(this->textures))
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(VirtualList::back(this->textures));

		minimumSegment = BgmapTexture::getSegment(bgmapTexture);

		// This allows to have bgmaps sprites that are smaller than 512 pixels high
		// But depends on all the segments being free
		if(((MBgmapSpriteSpec*)this->componentSpec)->xLoop && 64 > Texture::getRows(bgmapTexture))
		{
			minimumSegment += 1;
		}
	}

	BgmapTexture bgmapTexture = 
		BgmapTexture::safeCast
		(
			Texture::get
			(
				typeofclass(BgmapTexture), textureSpec, minimumSegment, 
				isFirstTextureAndHasMultipleTextures, ((MBgmapSpriteSpec*)this->componentSpec)->scValue
			)
		);

	NM_ASSERT(!isDeleted(bgmapTexture), "MBgmapSprite::loadTexture: texture not loaded");
	NM_ASSERT(!isDeleted(this->textures), "MBgmapSprite::loadTexture: null textures list");
	NM_ASSERT
	(
		!isFirstTextureAndHasMultipleTextures || 0 == (BgmapTexture::getSegment(bgmapTexture) % 2), 
		"MBgmapSprite::loadTexture: first texture not loaded in even segment"
	);

	if(!isDeleted(bgmapTexture))
	{
		BgmapTexture::addEventListener(bgmapTexture, ListenerObject::safeCast(this), kEventTextureRewritten);

		VirtualList::pushBack(this->textures, bgmapTexture);

		this->texture = Texture::safeCast(bgmapTexture);
		NM_ASSERT(this->texture, "MBgmapSprite::loadTexture: null texture");
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::releaseTextures()
{
	// Free the texture
	if(!isDeleted(this->texture))
	{
		// If affine or bgmap
		if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
		{
			// Free param table space
			ParamTableManager::free(ParamTableManager::getInstance(), BgmapSprite::safeCast(this));
		}
	}

	this->texture = NULL;

	if(!isDeleted(this->textures))
	{
		VirtualNode node = this->textures->head;

		for(; NULL != node; node = node->next)
		{
			BgmapTexture bgmapTexture = BgmapTexture::safeCast(node->data);

			if(!isDeleted(bgmapTexture))
			{
				BgmapTexture::removeEventListener(bgmapTexture, ListenerObject::safeCast(this), kEventTextureRewritten);
				
				Texture::release(Texture::safeCast(bgmapTexture));
			}
		}

		delete this->textures;
		this->textures = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapSprite::calculateSize()
{
	VirtualNode node = this->textures->head;

	int32 cols = 0;
	int32 rows = 0;

	for(; NULL != node; node = node->next)
	{
		// Free the texture
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

	if(0 < ((MBgmapSpriteSpec*)this->componentSpec)->width)
	{
		this->halfWidth = ((MBgmapSpriteSpec*)this->componentSpec)->width >> 1;
	}

	if(0 < ((MBgmapSpriteSpec*)this->componentSpec)->height)
	{
		this->halfHeight = ((MBgmapSpriteSpec*)this->componentSpec)->height >> 1;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
