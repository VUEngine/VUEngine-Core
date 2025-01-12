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

#include <Affine.h>
#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <DebugConfig.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>

#include "BgmapSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class BgmapTexture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 BgmapSprite::doApplyAffineTransformations(BgmapSprite bgmapSprite, int32 maximumParamTableRowsToComputePerCall)
{
	ASSERT(bgmapSprite->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if(0 < bgmapSprite->param)
	{
		return Affine::transform
		(
			maximumParamTableRowsToComputePerCall,
			bgmapSprite->param,
			bgmapSprite->paramTableRow,
			// Geometrically accurate, but kills the CPU
			// (0 > bgmapSprite->position.x? bgmapSprite->position.x : 0) + bgmapSprite->halfWidth,
			// (0 > bgmapSprite->position.y? bgmapSprite->position.y : 0) + bgmapSprite->halfHeight,
			// Don't do translations
			// Provide a little bit of performance gain by only calculation transformation equations
			// For the visible rows, but causes that some sprites not be rendered completely when the
			// Camera moves vertically
			// Int32 lastRow = height + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: height;
			// This->paramTableRow = this->paramTableRow ? this->paramTableRow : myDisplacement;
			__I_TO_FIXED(bgmapSprite->halfWidth),
			__I_TO_FIXED(bgmapSprite->halfHeight),
			__I_TO_FIX13_3(bgmapSprite->bgmapTextureSource.mx),
			__I_TO_FIX13_3(bgmapSprite->bgmapTextureSource.my),
			__I_TO_FIXED(bgmapSprite->texture->textureSpec->cols << 2),
			__I_TO_FIXED(bgmapSprite->texture->textureSpec->rows << 2),
			&bgmapSprite->rotation
		);
	}

	return bgmapSprite->paramTableRow;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::constructor(Entity owner, const BgmapSpriteSpec* bgmapSpriteSpec)
{
	NM_ASSERT(NULL != bgmapSpriteSpec, "BgmapSprite::constructor: NULL bgmapSpriteSpec");

	// Always explicitly call the base's constructor 
	Base::constructor(owner, (SpriteSpec*)&bgmapSpriteSpec->spriteSpec);

	this->bgmapTextureSource.mx = 0;
	this->bgmapTextureSource.my = 0;
	this->bgmapTextureSource.mp = 0;

	this->displacement = bgmapSpriteSpec->spriteSpec.displacement;

	this->param = 0;
	this->paramTableRow = 0;

	this->applyParamTableEffect = bgmapSpriteSpec->applyParamTableEffect;

	BgmapSprite::configureTexture(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::destructor()
{
	// Make sure that the texture is not loaded again
	this->componentSpec = NULL;

	BgmapSprite::removeFromCache(this);

	ASSERT(this, "BgmapSprite::destructor: null cast");

	BgmapSprite::releaseTexture(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ClassPointer BgmapSprite::getManagerClass()
{
	return typeofclass(SpriteManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::hasSpecialEffects()
{
	return NULL != this->applyParamTableEffect && 0 != ((__WORLD_HBIAS | __WORLD_AFFINE ) & this->head);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::processEffects(int32 maximumParamTableRowsToComputePerCall)
{
	// Set the world size according to the zoom
	if(0 < this->param && (uint8)__NO_RENDER_INDEX != this->index)
	{
		if(0 != ((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && NULL != this->applyParamTableEffect)
		{
			if(0 <= this->paramTableRow)
			{
				// Apply affine transformation
				this->paramTableRow = this->applyParamTableEffect(this, maximumParamTableRowsToComputePerCall);

				if(0 > this->paramTableRow)
				{
					this->paramTableRow = -1;
				}
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::doRender(int16 index)
{
	if(isDeleted(this->texture))
	{
		BgmapSprite::configureTexture(this);

		return __NO_RENDER_INDEX;
	}

	NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::doRender: null texture");

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	// Get coordinates
	int16 gx = this->position.x + this->displacement.x - this->halfWidth;
	int16 gy = this->position.y + this->displacement.y - this->halfHeight;
	int16 gp = this->position.parallax + this->displacement.parallax;

	int16 auxGp = __ABS(gp);

	// Get sprite's size
	int16 w = this->halfWidth << 1;
	int16 h = this->halfHeight << 1;

	// Set the head
	int32 mx = this->bgmapTextureSource.mx;
	int32 my = this->bgmapTextureSource.my;
	int32 mp = this->bgmapTextureSource.mp;

	// Cap coordinates to camera space
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setMultiframe(uint16 frame)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		// Scale the texture in the next render cycle
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setScale(const PixelScale* scale)
{
	if(NULL == scale)
	{
		return;
	}

	this->scale = *scale;

	if(0 != (__WORLD_AFFINE & this->head))
	{
		PixelScale scaleHelper = *scale;

		NM_ASSERT(0 < scaleHelper.x, "Sprite::scale: 0 scale x");
		NM_ASSERT(0 < scaleHelper.y, "Sprite::scale: 0 scale y");

		extern Rotation _previousCameraInvertedRotation;

		Vector3D vector = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation->position), _previousCameraInvertedRotation);

		fix7_9 ratio = __FIXED_TO_FIX7_9(Vector3D::getScale(vector.z, true));

		ratio = 0 > ratio? __1I_FIX7_9 : ratio;
		ratio = __I_TO_FIX7_9(__MAXIMUM_SCALE) < ratio? __I_TO_FIX7_9(__MAXIMUM_SCALE) : ratio;

		scaleHelper.x = __FIX7_9_MULT(scaleHelper.x, ratio);
		scaleHelper.y = __FIX7_9_MULT(scaleHelper.y, ratio);

		this->rendered = false;

		if(!isDeleted(this->texture))
		{
			// Add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
			this->halfWidth = 
				__FIXED_TO_I
				(
					__ABS
					(
						__FIXED_MULT
						(
							__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation->rotation.y))),
							__FIXED_MULT
							(
							__I_TO_FIXED((int32)this->texture->textureSpec->cols << 2),
							__FIX7_9_TO_FIXED(scaleHelper.x)
							)
						)
					)
				) + 1;

			this->halfHeight = 
				__FIXED_TO_I
				(
					__ABS
					(
						__FIXED_MULT
						(
							__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation->rotation.x))),
							__FIXED_MULT
							(
							__I_TO_FIXED((int32)this->texture->textureSpec->rows << 2),
							__FIX7_9_TO_FIXED(scaleHelper.y)
							)
						)
					)
				) + 1;
		}

		if(0 < this->param)
		{
			this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 BgmapSprite::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return Sprite::getEffectiveWidth(this) * Sprite::getEffectiveHeight(this);
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::configureTexture()
{
	if(!isDeleted(this->texture))
	{
		return;
	}

	TextureSpec* textureSpec =((BgmapSpriteSpec*)this->componentSpec)->spriteSpec.textureSpec;

	if(NULL == textureSpec)
	{
		return;
	}

	this->texture = Texture::get(typeofclass(BgmapTexture), textureSpec, 0, false, __WORLD_1x1);
	
	if(isDeleted(this->texture))
	{
		return;
	}

	this->bgmapTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
	this->bgmapTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
	this->bgmapTextureSource.mp = 0;

	BgmapSprite::setMode(this, ((BgmapSpriteSpec*)this->componentSpec)->display, ((BgmapSpriteSpec*)this->componentSpec)->bgmapMode);

	if(0 != this->param && !isDeleted(this->texture))
	{
		Texture::addEventListener
		(
			this->texture, ListenerObject::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setMode(uint16 display, uint16 mode)
{
	this->head &= ~(__WORLD_BGMAP | __WORLD_AFFINE | __WORLD_HBIAS);

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
	{
		// Free param table space
		ParamTableManager::free(this);

		this->param = 0;
	}

	if(0 == this->param && !isDeleted(this->texture))
	{
		switch(mode)
		{
			case __WORLD_BGMAP:

				// Set map head
				this->head = display | __WORLD_BGMAP;
				break;

			case __WORLD_AFFINE:

				this->head = display | __WORLD_AFFINE;
				this->param = ParamTableManager::allocate(this);
				this->applyParamTableEffect = 
					NULL != this->applyParamTableEffect ? this->applyParamTableEffect : BgmapSprite::doApplyAffineTransformations;
				break;

			case __WORLD_HBIAS:

				// Set map head
				this->head = display | __WORLD_HBIAS;

				if(NULL != this->applyParamTableEffect)
				{
					this->param = ParamTableManager::allocate(this);
				}

				break;
		}
	}

	this->head &= ~__WORLD_END;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setParam(uint32 param)
{
	this->param = param;

	// Set flag to rewrite texture's param table
	BgmapSprite::invalidateParamTable(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 BgmapSprite::getParam()
{
	return this->param;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getParamTableRow()
{
	return this->paramTableRow;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::onTextureRewritten(ListenerObject eventFirer __attribute__ ((unused)))
{
	BgmapSprite::processEffects(this, SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance()));

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::applyAffineTransformations()
{
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	this->paramTableRow = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::applyHbiasEffects()
{
	ASSERT(this->texture, "BgmapSprite::applyHbiasEffects: null texture");

	this->paramTableRow = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::removeFromCache()
{	
	if(__NO_RENDER_INDEX != this->index)
	{
		WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
		worldPointer->head = __WORLD_OFF;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::releaseTexture()
{
	// Free the texture
	if(!isDeleted(this->texture))
	{
		// If affine or bgmap
		if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
		{
			// Free param table space
			ParamTableManager::free(this);
		}

		if(0 != this->param)
		{
			Texture::removeEventListener
			(
				this->texture, ListenerObject::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten
			);
		}
		
		Texture::release(this->texture);
	}

	this->texture = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
