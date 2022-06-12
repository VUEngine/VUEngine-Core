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

#include <BgmapSprite.h>
#include <Affine.h>
#include <Game.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <Camera.h>
#include <VIPManager.h>
#include <debugConfig.h>


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
void BgmapSprite::constructor(const BgmapSpriteSpec* bgmapSpriteSpec, Object owner)
{
	Base::constructor((SpriteSpec*)&bgmapSpriteSpec->spriteSpec, owner);

	// create the texture
	if(bgmapSpriteSpec->spriteSpec.textureSpec)
	{
		this->texture = Texture::safeCast(BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), bgmapSpriteSpec->spriteSpec.textureSpec, 0, false, __WORLD_1x1));
		NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::constructor: null texture");
	}

	if(!isDeleted(this->texture))
	{
		Texture::addEventListener(this->texture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);

		// set texture position
		this->drawSpec.textureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
		this->drawSpec.textureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
		this->drawSpec.textureSource.mp = 0;

		this->halfWidth = Texture::getCols(this->texture) << 2;
		this->halfHeight = Texture::getRows(this->texture) << 2;
	}
	else
	{
		this->drawSpec.textureSource.mx = 0;
		this->drawSpec.textureSource.my = 0;
		this->drawSpec.textureSource.mp = 0;
	}

	this->drawSpec.rotation = Rotation::zero();
	this->drawSpec.scale = Scale::unit();

	this->displacement = bgmapSpriteSpec->spriteSpec.displacement;

	this->param = 0;
	this->paramTableRow = 0;

	// set WORLD layer's head according to map's render mode
	this->applyParamTableEffect = bgmapSpriteSpec->applyParamTableEffect;
	BgmapSprite::setMode(this, bgmapSpriteSpec->display, bgmapSpriteSpec->bgmapMode);
}

/**
 * Class denstructor
 *
 * @memberof			BgmapSprite
 * @public
 */
void BgmapSprite::destructor()
{
	if(this->registered)
	{
		SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this), BgmapSprite::hasSpecialEffects(this));
	}

	ASSERT(this, "BgmapSprite::destructor: null cast");

	BgmapSprite::releaseTexture(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

bool BgmapSprite::hasSpecialEffects()
{
	return 0 != ((__WORLD_HBIAS | __WORLD_AFFINE) & this->head);
}

/**
 * Process event
 *
 * @param eventFirer
 */
void BgmapSprite::onTextureRewritten(Object eventFirer __attribute__ ((unused)))
{
	BgmapSprite::processEffects(this);
}

void BgmapSprite::releaseTexture()
{
	// free the texture
	if(!isDeleted(this->texture))
	{
		// if affine or bgmap
		if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
		{
			// free param table space
			ParamTableManager::free(ParamTableManager::getInstance(), this);
		}

		Texture::removeEventListener(this->texture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
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
	return this->drawSpec.scale;
}

/**
 * Calculate with and height
 *
 * @memberof			BgmapSprite
 * @private
 */
void BgmapSprite::computeDimensions()
{
	this->halfWidth = __FIX10_6_TO_I(__ABS(__FIX10_6_MULT(
		__FIX7_9_TO_FIX10_6(__COS(__FIX10_6_TO_I(this->drawSpec.rotation.y))),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int32)this->texture->textureSpec->cols << 2),
			__FIX7_9_TO_FIX10_6(this->drawSpec.scale.x)
		)
	))) + 1;

	this->halfHeight = __FIX10_6_TO_I(__ABS(__FIX10_6_MULT(
		__FIX7_9_TO_FIX10_6(__COS(__FIX10_6_TO_I(this->drawSpec.rotation.x))),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int32)this->texture->textureSpec->rows << 2),
			__FIX7_9_TO_FIX10_6(this->drawSpec.scale.y)
		)
	))) + 1;
}

/**
 * Rotate
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param rotation		Rotation
 */
void BgmapSprite::rotate(const Rotation* rotation)
{
	Base::rotate(this, rotation);
	
	if(this->param)
	{
		this->drawSpec.rotation = *rotation;

		if(this->texture)
		{
			BgmapSprite::computeDimensions(this);
		}

		this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;

		// scale the texture in the next render cycle
		BgmapSprite::invalidateParamTable(this);
	}
	else if(!isDeleted(this->texture))
	{
		Direction direction =
		{
			__QUARTER_ROTATION_DEGREES < __ABS(rotation->y) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z)  ? __LEFT : __RIGHT,
			__QUARTER_ROTATION_DEGREES < __ABS(rotation->x) || __QUARTER_ROTATION_DEGREES < __ABS(rotation->z) ? __UP : __DOWN,
			__FAR,
		};

		if(__LEFT == direction.x)
		{
			BgmapTexture::setHorizontalFlip(this->texture, true);
		}
		else if(__RIGHT == direction.x)
		{
			BgmapTexture::setHorizontalFlip(this->texture, false);
		}

		if(__UP == direction.y)
		{
			BgmapTexture::setVerticalFlip(this->texture, true);
		}
		else if(__DOWN == direction.y)
		{
			BgmapTexture::setVerticalFlip(this->texture, false);
		}		
	}
}

/**
 * Resize
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void BgmapSprite::resize(Scale scale, fix10_6 z)
{
	if(__WORLD_AFFINE & this->head)
	{
		NM_ASSERT(0 < scale.x, "BgmapSprite::resize: 0 scale x");
		NM_ASSERT(0 < scale.y, "BgmapSprite::resize: 0 scale y");

		if(0 == z + _optical->cameraNearPlane)
		{
			return;
		}

		fix7_9 ratio = __FIX10_6_TO_FIX7_9(Vector3D::getScale(z));

		ratio = 0 > ratio? __1I_FIX10_6 : ratio;
		ratio = __I_TO_FIX7_9(__MAXIMUM_SCALE) < ratio? __I_TO_FIX7_9(__MAXIMUM_SCALE) : ratio;

		this->drawSpec.scale.x = __FIX7_9_MULT(scale.x, ratio);
		this->drawSpec.scale.y = __FIX7_9_MULT(scale.y, ratio);

		NM_ASSERT(0 < this->drawSpec.scale.x, "BgmapSprite::resize: null scale x");
		NM_ASSERT(0 < this->drawSpec.scale.y, "BgmapSprite::resize: null scale y");

		if(this->texture)
		{
			// apply add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
			BgmapSprite::computeDimensions(this);
		}

		if(this->param)
    	{
			this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;
		}
	}
}

/**
 * Retrieve the drawspec
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @return			DrawSpec
 */
DrawSpec BgmapSprite::getDrawSpec()
{
	return this->drawSpec;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param evenFrame
 */
int16 BgmapSprite::doRender(int16 index, bool evenFrame __attribute__((unused)))
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
	int32 mx = this->drawSpec.textureSource.mx;
	int32 my = this->drawSpec.textureSource.my;
	int32 mp = this->drawSpec.textureSource.mp;

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
	worldPointer->param = (uint16)((((this->param + (myDisplacement << 4))) - 0x20000) >> 1) & 0xFFF0;

	return index;
}

void BgmapSprite::processEffects()
{
	// set the world size according to the zoom
	if(0 < this->param && (uint8)__NO_RENDER_INDEX != this->index)
	{
		BgmapSprite::processAffineEffects(this);
		BgmapSprite::processHbiasEffects(this);
	}
}

void BgmapSprite::processAffineEffects()
{
	if((__WORLD_AFFINE & this->head) && this->applyParamTableEffect)
	{
		// provide a little bit of performance gain by only calculation transformation equations
		// for the visible rows, but causes that some sprites not be rendered completely when the
		// camera moves vertically
		// int32 lastRow = height + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: height;
		// this->paramTableRow = this->paramTableRow ? this->paramTableRow : myDisplacement;

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

void BgmapSprite::processHbiasEffects()
{
	if((__WORLD_HBIAS & this->head) && this->applyParamTableEffect)
	{
		if(0 <= this->paramTableRow)
		{
			// apply hbias effects
			this->paramTableRow = this->applyParamTableEffect(this);

			if(0 > this->paramTableRow)
			{
				this->paramTableRow = -1;
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

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), this);

		this->param = 0;
	}

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
			this->applyParamTableEffect = this->applyParamTableEffect ? this->applyParamTableEffect : BgmapSprite::doApplyAffineTransformations;
			break;

		case __WORLD_HBIAS:

			// set map head
			this->head = display | __WORLD_HBIAS;
			this->param = ParamTableManager::allocate(ParamTableManager::getInstance(), this);
			break;
	}

	this->head &= ~__WORLD_END;
}

/**
 * Register
 *
 * @param position		
 */
void BgmapSprite::registerWithManager()
{
	if(!this->registered)
	{
		this->registered = SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this), BgmapSprite::hasSpecialEffects(this));
	}
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
 * Set drawspec
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param drawSpec		New drawSpec
 */
void BgmapSprite::setDrawSpec(const DrawSpec* const drawSpec)
{
	this->drawSpec = *drawSpec;
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

	if(bgmapSprite->param)
	{
		return Affine::applyAll(
			bgmapSprite->param,
			bgmapSprite->paramTableRow,
			// geometrically accurate, but kills the CPU
			// (0 > bgmapSprite->position.x? bgmapSprite->position.x : 0) + bgmapSprite->halfWidth,
			// (0 > bgmapSprite->position.y? bgmapSprite->position.y : 0) + bgmapSprite->halfHeight,
			// don't do translations
			__I_TO_FIX10_6(bgmapSprite->halfWidth),
			__I_TO_FIX10_6(bgmapSprite->halfHeight),
			__I_TO_FIX13_3(bgmapSprite->drawSpec.textureSource.mx),
			__I_TO_FIX13_3(bgmapSprite->drawSpec.textureSource.my),
			__I_TO_FIX10_6(bgmapSprite->texture->textureSpec->cols << 2),
			__I_TO_FIX10_6(bgmapSprite->texture->textureSpec->rows << 2),
			&bgmapSprite->drawSpec.scale,
			&bgmapSprite->drawSpec.rotation
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
