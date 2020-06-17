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
		this->texture = Texture::safeCast(BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), bgmapSpriteSpec->spriteSpec.textureSpec, 0, false));
		NM_ASSERT(this->texture, "BgmapSprite::constructor: null texture");
	}

	if(this->texture)
	{
		Object::addEventListener(this->texture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);

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

	this->drawSpec.scale.x = __1I_FIX7_9;
	this->drawSpec.scale.y = __1I_FIX7_9;

	this->drawSpec.rotation.x = 0;
	this->drawSpec.rotation.y = 0;
	this->drawSpec.rotation.z = 0;

	this->displacement = bgmapSpriteSpec->spriteSpec.displacement;

	this->param = 0;
	this->paramTableRow = 0;

	// set WORLD layer's head according to map's render mode
	this->applyParamTableEffect = bgmapSpriteSpec->applyParamTableEffect;
	BgmapSprite::setMode(this, bgmapSpriteSpec->display, bgmapSpriteSpec->bgmapMode);

	SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this));
}

/**
 * Class denstructor
 *
 * @memberof			BgmapSprite
 * @public
 */
void BgmapSprite::destructor()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this));

	ASSERT(this, "BgmapSprite::destructor: null cast");

	// if affine or bgmap
	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager::free(ParamTableManager::getInstance(), this);
	}

	BgmapSprite::releaseTexture(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Process event
 *
 * @param eventFirer
 */
void BgmapSprite::onTextureRewritten(Object eventFirer __attribute__ ((unused)))
{
	BgmapSprite::applyAffineTransformations(this);
	BgmapSprite::applyHbiasEffects(this);
	this->writeAnimationFrame = true;
}

void BgmapSprite::releaseTexture()
{
	// free the texture
	if(!isDeleted(this->texture))
	{
		Object::removeEventListener(this->texture, Object::safeCast(this), (EventListener)BgmapSprite::onTextureRewritten, kEventTextureRewritten);
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
		__FIX7_9_TO_FIX10_6(__COS(this->drawSpec.rotation.y)),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int)this->texture->textureSpec->cols << 2),
			__FIX7_9_TO_FIX10_6(this->drawSpec.scale.x)
		)
	))) + 1;

	this->halfHeight = __FIX10_6_TO_I(__ABS(__FIX10_6_MULT(
		__FIX7_9_TO_FIX10_6(__COS(this->drawSpec.rotation.x)),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int)this->texture->textureSpec->rows << 2),
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

		z -= _cameraPosition->z;

		fix7_9 ratio = __FIX10_6_TO_FIX7_9(__I_TO_FIX10_6(1) - __FIX10_6_EXT_DIV(z, _optical->scalingFactor));

		ratio = 0 > ratio? 0 : ratio;
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
u16 BgmapSprite::doRender(u16 index, bool evenFrame __attribute__((unused)))
{
	NM_ASSERT(!isDeleted(this->texture), "BgmapSprite::doRender: null texture");

	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	// get coordinates
	s16 gx = this->position.x + this->displacement.x - this->halfWidth;
	s16 gy = this->position.y + this->displacement.y - this->halfHeight;
	s16 gp = this->position.parallax + this->displacement.parallax;

	s16 auxGp = __ABS(gp);

	// get sprite's size
	s16 width = this->halfWidth << 1;
	s16 height = this->halfHeight << 1;
	s16 w = width;
	s16 h = height;
	s16 myDisplacement = 0;

	// set the head
	int mx = this->drawSpec.textureSource.mx;
	int my = this->drawSpec.textureSource.my;
	int mp = this->drawSpec.textureSource.mp;

	// cap coordinates to camera space
	if(_cameraFrustum->x0 - auxGp > gx)
	{
		mx += (_cameraFrustum->x0 - auxGp - gx);
		w -= (_cameraFrustum->x0 - auxGp - gx);
		gx = _cameraFrustum->x0 - auxGp;
	}

	if(_cameraFrustum->y0 > gy)
	{
		my += (_cameraFrustum->y0 - gy);
		h -= (_cameraFrustum->y0 - gy);
		myDisplacement = (_cameraFrustum->y0 - gy);
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

	// set the world size according to the zoom
	if(this->param)
	{
		BgmapSprite::processAffineEffects(this, index, gx, width, myDisplacement);
		BgmapSprite::processHbiasEffects(this, index);
	}

	return index;
}

void BgmapSprite::processAffineEffects(u16 index, int gx, int width, int myDisplacement)
{
	if((__WORLD_AFFINE & this->head) && this->applyParamTableEffect)
	{
		WorldAttributes* worldPointer = &_worldAttributesBaseAddress[index];

		// provide a little bit of performance gain by only calculation transformation equations
		// for the visible rows, but causes that some sprites not be rendered completely when the
		// camera moves vertically
		// int lastRow = height + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: height;
		// this->paramTableRow = this->paramTableRow ? this->paramTableRow : myDisplacement;

		// un-cap x coordinate in affine mode
		if(_cameraFrustum->x0 > gx)
		{
			worldPointer->gx = gx;
			worldPointer->w = width;
		}

		ASSERT(0 <= (((signed)this->param + (signed)(myDisplacement << 4))) - 0x20000, "BgmapSprite::processAffineEffects: right shift on negative operand");

		worldPointer->param = (u16)((((this->param + (myDisplacement << 4))) - 0x20000) >> 1) & 0xFFF0;

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

void BgmapSprite::processHbiasEffects(u16 index)
{
	if((__WORLD_HBIAS & this->head) && this->applyParamTableEffect)
	{
		WorldAttributes* worldPointer = &_worldAttributesBaseAddress[index];

 		ASSERT(0 <= ((signed)this->param - 0x20000), "BgmapSprite::processAffineEffects: right shift on negative operand");

		worldPointer->param = (u16)(((this->param) - 0x20000) >> 1) & 0xFFF0;

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

// handles affine transformations accurately by translating using translations
// to clip the image to the camera space, but kill the CPU
/*
// render a world layer with the map's information
void BgmapSprite::displacement()
{
	ASSERT(this->texture, "BgmapSprite::render: null texture");

	if(!this->positioned)
	{
		return;
	}

	// if render flag is set
	if(this->initialized)
	{
		if(this->hidden)
		{
			WORLD_HEAD(index, 0x0000);
			return;
		}

		WorldAttributes* worldPointer = &_worldAttributesBaseAddress[index];

		// set the world camera position
		int gx = __FIX10_6_TO_I(this->position.x + this->displacement.x);
		int gy = __FIX10_6_TO_I(this->position.y + this->displacement.y);

		int w = Texture::getCols(this->texture)<< 3;
		int h = Texture::getRows(this->texture)<< 3;

		int mxDisplacement = 0 > gx ? -gx : 0;
		int myDisplacement = 0 > gy ? -gy : 0;

		worldPointer->gx = gx > _cameraFrustum->x1 ? _cameraFrustum->x1 : 0 > gx ? 0: gx;
		worldPointer->gy = gy > _cameraFrustum->y1 ? _cameraFrustum->y1 : 0 > gy ? 0: gy;
		worldPointer->gp = this->position.parallax + __FIX10_6_TO_I(this->displacement.z & 0xFFFFE000);

		worldPointer->mx = this->drawSpec.textureSource.mx + mxDisplacement;
		worldPointer->my = this->drawSpec.textureSource.my + myDisplacement;
		worldPointer->mp = this->drawSpec.textureSource.mp;

		// -1 because 0 means 1 pixel for width
		w = w - __WORLD_SIZE_DISPLACEMENT - mxDisplacement;
		h = h - __WORLD_SIZE_DISPLACEMENT - myDisplacement;

		worldPointer->w = 0;
		worldPointer->h = 0;

		if(w + worldPointer->gx >= _cameraFrustum->x1)
		{
			worldPointer->w = _cameraFrustum->x1 - worldPointer->gx;
		}
		else if (0 <= w)
		{
			worldPointer->w = w;
		}

		if(h + worldPointer->gy >= _cameraFrustum->y1)
		{
			worldPointer->h = _cameraFrustum->y1 - worldPointer->gy;
		}
		else if (0 <= h)
		{
			worldPointer->h = h;
		}

		// set the world size according to the zoom
		if(__WORLD_AFFINE & this->head)
		{
			if(__UPDATE_G)
			{
				if(0 > this->paramTableRow && (0 > gx || 0 > gy))
				{
					this->paramTableRow = 0;
				}
			}

			if(_cameraDisplacement->y && 0 > this->paramTableRow)
			{
				this->paramTableRow = 0;
			}

			worldPointer->w *= __FIX7_9_TO_F(__ABS(this->drawSpec.scale.x));
			worldPointer->h *= __FIX7_9_TO_F(__ABS(this->drawSpec.scale.y));

			if(0 <= this->paramTableRow)
			{
				h = Texture::getRows(this->texture)<< 3;
				int lastRow = h + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: h;

				BgmapSprite::doApplyAffineTransformations(this, lastRow);

				if(0 >= this->paramTableRow)
				{
					this->paramTableRow = -1;
				}
			}

			worldPointer->param = ((__PARAM_DISPLACEMENT(this->param) - 0x20000) >> 1) & 0xFFF0;
		}

		worldPointer->head = this->head | BgmapTexture::getSegment(this->texture);
	}
}
*/

/**
 * Add displacement to position
 *
 * @memberof				BgmapSprite
 * @public
 *
 * @param displacement		2D position displacement
 */
void BgmapSprite::addDisplacement(const PixelVector* displacement)
{
	this->position.x += displacement->x;
	this->position.y += displacement->y;
	this->position.z += displacement->z;
	this->position.parallax += displacement->parallax;
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
void BgmapSprite::setMode(u16 display, u16 mode)
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
 * Retrieve param table address
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @return			Param table address
 */
u32 BgmapSprite::getParam()
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
void BgmapSprite::setParam(u32 param)
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
s16 BgmapSprite::getParamTableRow()
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
static s16 BgmapSprite::doApplyAffineTransformations(BgmapSprite bgmapSprite)
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
