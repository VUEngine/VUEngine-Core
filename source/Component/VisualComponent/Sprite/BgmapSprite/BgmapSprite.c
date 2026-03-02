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
#include <AnimationController.h>
#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <DebugConfig.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <Printer.h>

#include "BgmapSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class BgmapTexture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

WorldAttributes _worldAttributesCache[__TOTAL_LAYERS] __attribute__((section(".dram_bss")));

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
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

	BgmapSprite::loadTexture(this, typeofclass(BgmapTexture));		
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::destructor()
{
	// If affine or bgmap
	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
	{
		// Free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), this);
	}

	BgmapSprite::removeFromCache(this);

	ASSERT(this, "BgmapSprite::destructor: null cast");

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventTextureRewritten:
		{
			if(NULL != this->texture)
			{
				BgmapSprite::processEffects(this, -1);
			}

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ClassPointer BgmapSprite::getBasicType()
{
	return typeofclass(BgmapSprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::loadTexture(ClassPointer textureClass __attribute__((unused)))
{
	Base::loadTexture(this, typeofclass(BgmapTexture));

	BgmapSprite::configureTexture(this);
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
	NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::doRender: null texture");

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	int16 cameraFrustumX0 = _cameraFrustum->x0, cameraFrustumY0 = _cameraFrustum->y0;
	int16 cameraFrustumX1 = _cameraFrustum->x1, cameraFrustumY1 = _cameraFrustum->y1;

	int16 gx = this->position.x + this->displacement.x - this->halfWidth;
	int16 gy = this->position.y + this->displacement.y - this->halfHeight;
	int16 gp = this->position.parallax + this->displacement.parallax;
	
	int16 auxGp = __ABS(gp);
	uint32 param = this->param;

	// Get sprite's size
	int16 w = this->halfWidth << 1;
	int16 h = this->halfHeight << 1;

	int32 mx = this->bgmapTextureSource.mx;
	int32 my = this->bgmapTextureSource.my;
	int32 mp = this->bgmapTextureSource.mp;

	// Horizontal clip
	int16 leftLimit = cameraFrustumX0 - auxGp;
	
	if (gx < leftLimit && param == 0)
	{
		int16 deltaX = leftLimit - gx;
		mx += deltaX;
		w -= deltaX;
		gx = leftLimit;
	}

	int16 myDisplacement = 0;

	// Vertical clip
	if (gy < cameraFrustumY0)
	{
		myDisplacement = cameraFrustumY0 - gy;
		my += myDisplacement;
		h -= myDisplacement;
		gy  = cameraFrustumY0;
	}

	int16 rightLimit = cameraFrustumX1 + auxGp;
	
	if (gx + w >= rightLimit)
	{
		w = rightLimit - gx;
	}

	if (__WORLD_SIZE_DISPLACEMENT >= w)
	{		
		return __NO_RENDER_INDEX;
	}

	if (gy + h >= cameraFrustumY1)
	{
		h = cameraFrustumY1 - gy;
	}

#ifdef __HACK_BGMAP_SPRITE_HEIGHT
	if (__MINIMUM_BGMAP_SPRITE_HEIGHT >= h && 0 == gy)
	{
		if (__WORLD_SIZE_DISPLACEMENT >= h)
		{
			return __NO_RENDER_INDEX;
		}
		my -= (__MINIMUM_BGMAP_SPRITE_HEIGHT - h);
	}
#else
	if (__WORLD_SIZE_DISPLACEMENT >= h)
	{
		return __NO_RENDER_INDEX;
	}
#endif

	w -= __WORLD_SIZE_DISPLACEMENT;
	h -= __WORLD_SIZE_DISPLACEMENT;

	// Sequential writes for memory bus efficiencameraFrustumY
	worldPointer->gx = gx;
	worldPointer->gy = gy;
	worldPointer->gp = gp;
	worldPointer->mx = mx;
	worldPointer->my = my;
	worldPointer->mp = mp;
	worldPointer->w = w;
	worldPointer->h = h;
	worldPointer->head = this->head | (BgmapTexture::safeCast(this->texture))->segment;

	if(0 < param)
	{
		worldPointer->param = (uint16)((((param + (myDisplacement << 4))) - 0x20000) >> 1) & 0xFFF0;
	}

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setMultiframe(uint16 frame)
{
	if(NULL == this->texture)
	{
		return;
	}

	int16 mx = BgmapTexture::getXOffset(this->texture) << 3;
	int16 my = BgmapTexture::getYOffset(this->texture) << 3;
	
	int16 cols = Texture::getCols(this->texture);

	int16 allocableFrames = (64 - mx) / cols;
	int16 usableCollsPerRow = allocableFrames * cols;
	int16 usedCollsPerRow = frame * cols;

	int16 col = usedCollsPerRow % usableCollsPerRow;
	int16 row = Texture::getRows(this->texture) * (frame / allocableFrames);

	this->bgmapTextureSource.mx = mx + (col << 3);
	this->bgmapTextureSource.my = my + (row << 3);
	this->rendered = false;

	BgmapSprite::invalidateParamTable(this);
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

		Vector3D vector = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation->position), *_cameraInvertedRotation);

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
		return BgmapSprite::getEffectiveWidth(this) * BgmapSprite::getEffectiveHeight(this);
	}

	return 0;
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::print(int32 x, int32 y)
{
	// Allow normal rendering once for WORLD values to populate properly
	uint8 transparency = this->transparency;
	this->transparency = __TRANSPARENCY_NONE;

	Printer::text("SPRITE ", x, y++, NULL);
	Printer::text("Class: ", x, ++y, NULL);
	Printer::text(__GET_CLASS_NAME(this), x + 18, y, NULL);
	Printer::text("Mode:", x, ++y, NULL);

	if(BgmapSprite::isAffine(this))
	{
		Printer::text("AFFINE   ", x + 18, y, NULL);
	}
	else if(BgmapSprite::isHBias(this))
	{
		Printer::text("H-BIAS   ", x + 18, y, NULL);
	}
	else if(BgmapSprite::isBgmap(this))
	{
		Printer::text("BGMAP    ", x + 18, y, NULL);
	}

	Printer::text("Index: ", x, ++y, NULL);
	Printer::int32(this->index, x + 18, y, NULL);
	Printer::text("Head:                         ", x, ++y, NULL);
	Printer::hex(BgmapSprite::getEffectiveHead(this), x + 18, y, 8, NULL);
	Printer::text("Transparent:                         ", x, ++y, NULL);
	Printer::text(transparency > 0 ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);
	Printer::text(transparency == 1 ? "(Even)" : (transparency == 2) ? "(Odd)" : "", x + 20, y, NULL);
	Printer::text("Shown:                         ", x, ++y, NULL);
	Printer::text(__HIDE != this->show ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL);

	Printer::text("Pos. (x,y,z,p):                      ", x, ++y, NULL);
	Printer::int32(this->position.x, x + 18, y, NULL);
	Printer::int32(this->position.y, x + 24, y, NULL);
	Printer::int32(this->position.z, x + 30, y, NULL);
	Printer::int32(this->position.parallax, x + 36, y, NULL);
	Printer::text("Displ. (x,y,z,p):                    ", x, ++y, NULL);
	Printer::int32(this->displacement.x, x + 18, y, NULL);
	Printer::int32(this->displacement.y, x + 24, y, NULL);
	Printer::int32(this->displacement.z, x + 30, y, NULL);
	Printer::int32(this->displacement.parallax, x + 36, y, NULL);
	Printer::text("FPos. (x,y,z,p):                      ", x, ++y, NULL);
	Printer::int32(this->position.x + this->displacement.x, x + 18, y, NULL);
	Printer::int32(this->position.y + this->displacement.y, x + 24, y, NULL);
	Printer::int32(this->position.z + this->displacement.z, x + 30, y, NULL);
	Printer::int32(this->position.parallax + this->displacement.parallax, x + 36, y, NULL);
	Printer::text("G (x,y,p):                           ", x, ++y, NULL);
	Printer::int32(BgmapSprite::getEffectiveX(this), x + 18, y, NULL);
	Printer::int32(BgmapSprite::getEffectiveY(this), x + 24, y, NULL);
	Printer::int32(BgmapSprite::getEffectiveP(this), x + 30, y, NULL);
	Printer::text("M (x,y,p):                           ", x, ++y, NULL);
	Printer::int32(BgmapSprite::getEffectiveMX(this), x + 18, y, NULL);
	Printer::int32(BgmapSprite::getEffectiveMY(this), x + 24, y, NULL);
	Printer::int32(BgmapSprite::getEffectiveMP(this), x + 30, y, NULL);
	Printer::text("Size (w,h):                          ", x, ++y, NULL);
	Printer::int32(BgmapSprite::getEffectiveWidth(this), x + 18, y, NULL);
	Printer::int32(BgmapSprite::getEffectiveHeight(this), x + 24, y++, NULL);
	Printer::text("Pixels:                      ", x, y, NULL);
	Printer::int32(BgmapSprite::getTotalPixels(this), x + 18, y++, NULL);

	if(NULL != BgmapSprite::getTexture(this))
	{
		y++;
		Printer::text("TEXTURE                          ", x, ++y, NULL);
		y++;
		Printer::text("Spec:                      ", x, ++y, NULL);
		Printer::hex((int32)Texture::getSpec(BgmapSprite::getTexture(this)), x + 18, y, 8, NULL);
		Printer::text("Size (w,h):                      ", x, ++y, NULL);
		Printer::int32(this->halfWidth * 2, x + 18, y, NULL);
		Printer::int32(this->halfHeight * 2, x + 24, y, NULL);

		if(BgmapSprite::getTexture(this))
		{
			BgmapTexture bgmapTexture = __GET_CAST(BgmapTexture, BgmapSprite::getTexture(this));

			Printer::text("Segment:                         ", x, ++y, NULL);
			Printer::int32(BgmapTexture::getSegment(bgmapTexture), x + 18, y, NULL);
			Printer::text("Written:                         ", x, ++y, NULL);
			Printer::text
			(
				
				Texture::isWritten(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 18, y, NULL
			);

			Printer::text("Rows remaining:                  ", x, ++y, NULL);
			Printer::int32(BgmapTexture::getRemainingRowsToBeWritten(bgmapTexture), x + 18, y, NULL);
		}
	}

	this->transparency = transparency;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::configureTexture()
{
	if(NULL != this->texture)
	{
		this->bgmapTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
		this->bgmapTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
		this->bgmapTextureSource.mp = 0;

		BgmapSprite::setMode(this, ((BgmapSpriteSpec*)this->componentSpec)->display, ((BgmapSpriteSpec*)this->componentSpec)->bgmapMode);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapSprite::setMode(uint16 display, uint16 mode)
{
	this->head &= ~(__WORLD_BGMAP | __WORLD_AFFINE | __WORLD_HBIAS);

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && 0 != this->param)
	{
		// Free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), this);

		this->param = 0;
	}

	if(0 == this->param && !isDeleted(this->texture))
	{
		switch(mode)
		{
			case __WORLD_BGMAP:
			{
				// Set map head
				this->head = display | __WORLD_BGMAP;
				break;
			}

			case __WORLD_AFFINE:
			{
				this->head = display | __WORLD_AFFINE;
				this->param = ParamTableManager::allocate(ParamTableManager::getInstance(), this);
				this->applyParamTableEffect = 
					NULL != this->applyParamTableEffect ? this->applyParamTableEffect : BgmapSprite::doApplyAffineTransformations;
				break;
			}

			case __WORLD_HBIAS:
			{
				// Set map head
				this->head = display | __WORLD_HBIAS;

				if(NULL != this->applyParamTableEffect)
				{
					this->param = ParamTableManager::allocate(ParamTableManager::getInstance(), this);
				}

				break;
			}
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

	this->rendered = false;
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

uint32 BgmapSprite::getEffectiveHead()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 BgmapSprite::getEffectiveWidth()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->w ? 0 : worldPointer->w;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 BgmapSprite::getEffectiveHeight()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return 0 > (int16)worldPointer->h ? 0 : worldPointer->h;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveX()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gx;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveY()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gy;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveP()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->gp;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveMX()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mx;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveMY()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->my;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapSprite::getEffectiveMP()
{
	WorldAttributes* worldPointer = &_worldAttributesCache[this->index];
	return worldPointer->mp;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::isBgmap()
{
	return (__WORLD_BGMAP == (this->head & __WORLD_BGMAP)) || BgmapSprite::isAffine(this) || BgmapSprite::isHBias(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::isAffine()
{
	return __WORLD_AFFINE == (this->head & __WORLD_AFFINE);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool BgmapSprite::isHBias()
{
	return __WORLD_HBIAS == (this->head & __WORLD_HBIAS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
