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

#include <Affine.h>
#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <DebugConfig.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <VIPManager.h>

#include "BgmapSprite.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Texture;
friend class BgmapTexture;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof						BgmapSprite
 * @public
 *
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void BgmapSprite::constructor(SpatialObject owner, const BgmapSpriteSpec* bgmapSpriteSpec)
{
	Base::constructor(owner, (SpriteSpec*)&bgmapSpriteSpec->spriteSpec);

	// create the texture
	if(NULL != bgmapSpriteSpec->spriteSpec.textureSpec)
	{
		this->texture = Texture::safeCast(BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), bgmapSpriteSpec->spriteSpec.textureSpec, 0, false, __WORLD_1x1));
		NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::constructor: null texture");
	}

	if(!isDeleted(this->texture))
	{
		// set texture position
		this->bgmapTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
		this->bgmapTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
		this->bgmapTextureSource.mp = 0;

		this->halfWidth = Texture::getCols(this->texture) << 2;
		this->halfHeight = Texture::getRows(this->texture) << 2;
	}
	else
	{
		this->bgmapTextureSource.mx = 0;
		this->bgmapTextureSource.my = 0;
		this->bgmapTextureSource.mp = 0;
	}

	this->displacement = bgmapSpriteSpec->spriteSpec.displacement;

	this->param = 0;
	this->paramTableRow = 0;

	// set WORLD layer's head according to map's render mode
	this->applyParamTableEffect = bgmapSpriteSpec->applyParamTableEffect;
	BgmapSprite::setMode(this, bgmapSpriteSpec->display, bgmapSpriteSpec->bgmapMode);

	if(0 != this->param && !isDeleted(this->texture))
	{
		Texture::addEventListener(this->texture, ListenerObject::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
	}
}

/**
 * Class denstructor
 *
 * @memberof			BgmapSprite
 * @public
 */
void BgmapSprite::destructor()
{
	BgmapSprite::removeFromCache(this);

	ASSERT(this, "BgmapSprite::destructor: null cast");

	BgmapSprite::releaseTexture(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Register
 *
 */
void BgmapSprite::registerWithManager()
{
	SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this), BgmapSprite::hasSpecialEffects(this));
}

/**
 * Unegister
 *
 */
void BgmapSprite::unregisterWithManager()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this), BgmapSprite::hasSpecialEffects(this));
}

void BgmapSprite::removeFromCache()
{	
	if(__NO_RENDER_INDEX != this->index)
	{
		WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
		worldPointer->head = __WORLD_OFF;
	}
}

bool BgmapSprite::hasSpecialEffects()
{
	return NULL != this->applyParamTableEffect && 0 != ((__WORLD_HBIAS | __WORLD_AFFINE ) & this->head);
}

/**
 * Process event
 *
 * @param eventFirer
 */
bool BgmapSprite::onTextureRewritten(ListenerObject eventFirer __attribute__ ((unused)))
{
	BgmapSprite::processEffects(this);

	return true;
}

void BgmapSprite::releaseTexture()
{
	// free the texture
	if(!isDeleted(this->texture))
	{
		// if affine or bgmap
		if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
		{
			// free param table space
			ParamTableManager::free(ParamTableManager::getInstance(), this);
		}

		if(0 != this->param)
		{
			Texture::removeEventListener(this->texture, ListenerObject::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
		}
		
		BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), BgmapTexture::safeCast(this->texture));
	}

	this->texture = NULL;
}

/**
 * Retrieve scale
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @return			Scale
 */
Scale BgmapSprite::getScale()
{
	// return the scale
	return this->transformation->scale;
}

/**
 * Rotate
 *
 * @memberof			BgmapSprite
 * @public
 *
 */
void BgmapSprite::setRotation(const Rotation* rotation)
{
	if(NULL == rotation)
	{
		return;
	}

	this->rotation = *rotation;

	if(0 < this->param)
	{

		this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;

		// scale the texture in the next render cycle
		BgmapSprite::invalidateParamTable(this);
	}
	else if(!isDeleted(this->texture))
	{
		NormalizedDirection normalizedDirection =
		{
			__QUARTER_ROTATION_DEGREES < __ABS(rotation->y) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z)  ? __LEFT : __RIGHT,
			__QUARTER_ROTATION_DEGREES < __ABS(rotation->x) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z) ? __UP : __DOWN,
			__FAR,
		};

		if(__LEFT == normalizedDirection.x)
		{
			BgmapTexture::setHorizontalFlip(this->texture, true);
		}
		else if(__RIGHT == normalizedDirection.x)
		{
			BgmapTexture::setHorizontalFlip(this->texture, false);
		}

		if(__UP == normalizedDirection.y)
		{
			BgmapTexture::setVerticalFlip(this->texture, true);
		}
		else if(__DOWN == normalizedDirection.y)
		{
			BgmapTexture::setVerticalFlip(this->texture, false);
		}		
	}
}

/**
 * Set scale
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void BgmapSprite::setScale(const PixelScale* scale)
{
	if(NULL == scale)
	{
		return;
	}

	if(__WORLD_AFFINE & this->head)
	{
		this->rendered = false;
		this->scale = *scale;

		if(!isDeleted(this->texture))
		{
			// apply add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
			BgmapSprite::calculateSize(this, scale);
		}

		if(0 < this->param)
    	{
			this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;
		}
	}
}

/**
 * Calculate with and height
 *
 * @memberof			BgmapSprite
 * @private
 */
void BgmapSprite::calculateSize(const PixelScale* scale)
{
	this->halfWidth = __FIXED_TO_I(__ABS(__FIXED_MULT(
		__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation->rotation.y))),
		__FIXED_MULT(
			__I_TO_FIXED((int32)this->texture->textureSpec->cols << 2),
			__FIX7_9_TO_FIXED(scale->x)
		)
	))) + 1;

	this->halfHeight = __FIXED_TO_I(__ABS(__FIXED_MULT(
		__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation->rotation.x))),
		__FIXED_MULT(
			__I_TO_FIXED((int32)this->texture->textureSpec->rows << 2),
			__FIX7_9_TO_FIXED(scale->y)
		)
	))) + 1;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param index
 */
int16 BgmapSprite::doRender(int16 index)
{
	NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::doRender: null texture");

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	// get coordinates
	int16 gx = this->position.x + this->displacement.x - this->halfWidth;
	int16 gy = this->position.y + this->displacement.y - this->halfHeight;
	int16 gp = this->position.parallax + this->displacement.parallax;

	int16 auxGp = __ABS(gp);

	// get sprite's size
	int16 w = this->halfWidth << 1;
	int16 h = this->halfHeight << 1;

	// set the head
	int32 mx = this->bgmapTextureSource.mx;
	int32 my = this->bgmapTextureSource.my;
	int32 mp = this->bgmapTextureSource.mp;

	// cap coordinates to camera space
	if(_cameraFrustum->x0 - auxGp > gx)
	{
		if(0 == this->param)
		{
			mx += (_cameraFrustum->x0 - auxGp - gx);
			w -= (_cameraFrustum->x0 - auxGp - gx);
			gx = _cameraFrustum->x0 - auxGp;
		}
	}

	int16 myDisplacement = 0;

	if(_cameraFrustum->y0 > gy)
	{
		myDisplacement = (_cameraFrustum->y0 - gy);

		my += myDisplacement;
		h -= (_cameraFrustum->y0 - gy);
		gy = _cameraFrustum->y0;
	}

	if(w + gx >= _cameraFrustum->x1 + auxGp)
	{
		w = _cameraFrustum->x1 - gx + auxGp;
	}

	if (__WORLD_SIZE_DISPLACEMENT >= w)
	{
		return __NO_RENDER_INDEX;
	}

/*
	if(_cameraFrustum->y0 > h + gy)
	{
		worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
		worldPointer->w = 0;
		worldPointer->h = 0;
#endif
	}
*/
	if(h + gy >= _cameraFrustum->y1)
	{
		h = _cameraFrustum->y1 - gy;
	}

#ifdef __HACK_BGMAP_SPRITE_HEIGHT
	if (__MINIMUM_BGMAP_SPRITE_HEIGHT >= h && 0 == gy)
	{
		if (__WORLD_SIZE_DISPLACEMENT >= h)
		{
			return __NO_RENDER_INDEX;
		}

		my -= __MINIMUM_BGMAP_SPRITE_HEIGHT - h;
	}
#else
	if (__WORLD_SIZE_DISPLACEMENT >= h)
	{
		return __NO_RENDER_INDEX;
	}
#endif

	worldPointer->gx = gx;
	worldPointer->gy = gy;
	worldPointer->gp = gp;

	worldPointer->mx = mx;
	worldPointer->my = my;
	worldPointer->mp = mp;

	worldPointer->w = w - __WORLD_SIZE_DISPLACEMENT;
	worldPointer->h = h - __WORLD_SIZE_DISPLACEMENT;

	worldPointer->head = this->head | (BgmapTexture::safeCast(this->texture))->segment;

	if(0 < this->param)
	{
		worldPointer->param = (uint16)((((this->param + (myDisplacement << 4))) - 0x20000) >> 1) & 0xFFF0;
	}

	return index;
}

void BgmapSprite::configureMultiframe(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture);
	int16 my = BgmapTexture::getYOffset(this->texture);
	
	int16 cols = Texture::getCols(this->texture);

	int16 allocableFrames = (64 - mx) / cols;
	int16 usableCollsPerRow = allocableFrames * cols;
	int16 usedCollsPerRow = frame * cols;

	int16 col = usedCollsPerRow % usableCollsPerRow;
	int16 row = Texture::getRows(this->texture) * (frame / allocableFrames);

	this->bgmapTextureSource.mx = mx + (col << 3);
	this->bgmapTextureSource.my = my + (row << 3);
	this->rendered = false;
}

void BgmapSprite::processEffects()
{
	// set the world size according to the zoom
	if(0 < this->param && (uint8)__NO_RENDER_INDEX != this->index)
	{
		if(0 != ((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && NULL != this->applyParamTableEffect)
		{
			if(0 <= this->paramTableRow)
			{
				// apply affine transformation
				this->paramTableRow = this->applyParamTableEffect(this);

				if(0 > this->paramTableRow)
				{
					this->paramTableRow = -1;
				}
			}
		}
	}
}

/**
 * Set Sprite's render mode
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void BgmapSprite::setMode(uint16 display, uint16 mode)
{
	this->head &= ~(__WORLD_BGMAP | __WORLD_AFFINE | __WORLD_HBIAS);

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
	{
		// free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), this);

		this->param = 0;
	}

	if(0 == this->param && !isDeleted(this->texture))
	{
		switch(mode)
		{
			case __WORLD_BGMAP:

				// set map head
				this->head = display | __WORLD_BGMAP;
				break;

			case __WORLD_AFFINE:

				// set map head
				this->head = display | __WORLD_AFFINE;
				this->param = ParamTableManager::allocate(ParamTableManager::getInstance(), this);
				this->applyParamTableEffect = NULL != this->applyParamTableEffect ? this->applyParamTableEffect : BgmapSprite::doApplyAffineTransformations;
				break;

			case __WORLD_HBIAS:

				// set map head
				this->head = display | __WORLD_HBIAS;

				if(NULL != this->applyParamTableEffect)
				{
					this->param = ParamTableManager::allocate(ParamTableManager::getInstance(), this);
				}

				break;
		}
	}

	this->head &= ~__WORLD_END;
}

/**
 * Retrieve param table address
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @return			Param table address
 */
uint32 BgmapSprite::getParam()
{
	return this->param;
}

/**
 * Set param table address
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param param		Net param table address
 */
void BgmapSprite::setParam(uint32 param)
{
	this->param = param;

	// set flag to rewrite texture's param table
	BgmapSprite::invalidateParamTable(this);
}

/**
 * Force to refresh param table in the next render cycle
 *
 * @memberof		BgmapSprite
 * @public
 */
void BgmapSprite::invalidateParamTable()
{
	if(__WORLD_AFFINE & this->head)
	{
		BgmapSprite::applyAffineTransformations(this);
	}
	else if(__WORLD_HBIAS & this->head)
	{
		BgmapSprite::applyHbiasEffects(this);
	}
}

/**
 * Retrieve the next param table row to calculate
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @return				Next param table row to calculate
 */
int16 BgmapSprite::getParamTableRow()
{
	return this->paramTableRow;
}

/**
 * Get the total amount of pixels displayed by the sprite
 *
 * @return		Total pixels
 */
int32 BgmapSprite::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return Sprite::getEffectiveWidth(this) * Sprite::getEffectiveHeight(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------
//										MAP FXs
//---------------------------------------------------------------------------------------------------------

/**
 * Start affine calculations
 *
 * @memberof			BgmapSprite
 * @private
 * @return 				last computed row
 */
static int16 BgmapSprite::doApplyAffineTransformations(BgmapSprite bgmapSprite)
{
	ASSERT(bgmapSprite->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if(0 < bgmapSprite->param)
	{
		return Affine::applyAll(
			bgmapSprite->param,
			bgmapSprite->paramTableRow,
			// geometrically accurate, but kills the CPU
			// (0 > bgmapSprite->position.x? bgmapSprite->position.x : 0) + bgmapSprite->halfWidth,
			// (0 > bgmapSprite->position.y? bgmapSprite->position.y : 0) + bgmapSprite->halfHeight,
			// don't do translations
			// Provide a little bit of performance gain by only calculation transformation equations
			// for the visible rows, but causes that some sprites not be rendered completely when the
			// camera moves vertically
			// int32 lastRow = height + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: height;
			// this->paramTableRow = this->paramTableRow ? this->paramTableRow : myDisplacement;
			__I_TO_FIXED(bgmapSprite->halfWidth),
			__I_TO_FIXED(bgmapSprite->halfHeight),
			__I_TO_FIX13_3(bgmapSprite->bgmapTextureSource.mx),
			__I_TO_FIX13_3(bgmapSprite->bgmapTextureSource.my),
			__I_TO_FIXED(bgmapSprite->texture->textureSpec->cols << 2),
			__I_TO_FIXED(bgmapSprite->texture->textureSpec->rows << 2),
			&bgmapSprite->scale,
			&bgmapSprite->rotation
		);
	}

	return bgmapSprite->paramTableRow;
}

/**
 * Trigger affine calculations
 *
 * @memberof			BgmapSprite
 * @public
 */
void BgmapSprite::applyAffineTransformations()
{
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	this->paramTableRow = 0;
}

/**
 * Trigger h-bias calculations
 *
 * @memberof			BgmapSprite
 * @public
 */
void BgmapSprite::applyHbiasEffects()
{
	ASSERT(this->texture, "BgmapSprite::applyHbiasEffects: null texture");

	this->paramTableRow = 0;
}
