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

/**
 * @class	BgmapSprite
 * @extends Sprite
 * @ingroup graphics-2d-sprites-bgmap
 * @brief	Sprite which holds a texture and a drawing specification.
 */
__CLASS_DEFINITION(BgmapSprite, Sprite);
__CLASS_FRIEND_DEFINITION(Texture);
__CLASS_FRIEND_DEFINITION(BgmapTexture);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);
static s16 BgmapSprite_doApplyAffineTransformations(BgmapSprite this);
static void BgmapSprite_computeDimensions(BgmapSprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapSprite, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
__CLASS_NEW_END(BgmapSprite, bgmapSpriteDefinition, owner);

/**
 * Class constructor
 *
 * @memberof						BgmapSprite
 * @public
 *
 * @param this						Function scope
 * @param bgmapSpriteDefinition		Sprite definition
 * @param owner						Owner
 */
void BgmapSprite_constructor(BgmapSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
{
	ASSERT(this, "BgmapSprite::constructor: null this");

	__CONSTRUCT_BASE(Sprite, (SpriteDefinition*)&bgmapSpriteDefinition->spriteDefinition, owner);

	// create the texture
	if(bgmapSpriteDefinition->spriteDefinition.textureDefinition)
	{
		this->texture = __SAFE_CAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), bgmapSpriteDefinition->spriteDefinition.textureDefinition));
		ASSERT(this->texture, "BgmapSprite::constructor: null texture");
	}

	if(this->texture)
	{
		Object_addEventListener(__SAFE_CAST(Object, this->texture), __SAFE_CAST(Object, this), (EventListener)Sprite_onTextureRewritten, kEventTextureRewritten);

		// set texture position
		this->drawSpec.textureSource.mx = BgmapTexture_getXOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
		this->drawSpec.textureSource.my = BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
		this->drawSpec.textureSource.mp = 0;

		this->halfWidth = Texture_getCols(this->texture) << 2;
		this->halfHeight = Texture_getRows(this->texture) << 2;
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

	this->displacement = bgmapSpriteDefinition->spriteDefinition.displacement;

	this->param = 0;
	this->paramTableRow = 0;

	// set WORLD layer's head according to map's render mode
	this->applyParamTableEffect = bgmapSpriteDefinition->applyParamTableEffect;
	BgmapSprite_setMode(this, bgmapSpriteDefinition->display, bgmapSpriteDefinition->bgmapMode);
}

/**
 * Class denstructor
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 */
void BgmapSprite_destructor(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::destructor: null cast");

	// if affine or bgmap
	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}

	// free the texture
	if(__IS_OBJECT_ALIVE(this->texture))
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->texture), __SAFE_CAST(Object, this), (EventListener)Sprite_onTextureRewritten, kEventTextureRewritten);
		BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, this->texture));
	}

	this->texture = NULL;

	// force stop rendering
	this->head = __WORLD_OFF;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Retrieve scale
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			Scale
 */
Scale BgmapSprite_getScale(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getScale: null this");

	// return the scale
	return this->drawSpec.scale;
}

/**
 * Calculate with and height
 *
 * @memberof			BgmapSprite
 * @private
 *
 * @param this			Function scope
 */
static void BgmapSprite_computeDimensions(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::rotate: null this");

	this->halfWidth = __FIX10_6_TO_I(__ABS(__FIX10_6_MULT(
		__FIX7_9_TO_FIX10_6(__COS(this->drawSpec.rotation.y)),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int)this->texture->textureDefinition->cols << 2),
			__FIX7_9_TO_FIX10_6(this->drawSpec.scale.x)
		)
	))) + 1;

	this->halfHeight = __FIX10_6_TO_I(__ABS(__FIX10_6_MULT(
		__FIX7_9_TO_FIX10_6(__COS(this->drawSpec.rotation.x)),
		__FIX10_6_MULT(
			__I_TO_FIX10_6((int)this->texture->textureDefinition->rows << 2),
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
 * @param this			Function scope
 * @param rotation		Rotation
 */
void BgmapSprite_rotate(BgmapSprite this, const Rotation* rotation)
{
	ASSERT(this, "BgmapSprite::rotate: null this");

	if(this->param)
	{
		this->drawSpec.rotation = *rotation;

		if(this->texture)
		{
			BgmapSprite_computeDimensions(this);
		}

		this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;

		// scale the texture in the next render cycle
		BgmapSprite_invalidateParamTable(this);
	}
}

/**
 * Resize
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void BgmapSprite_resize(BgmapSprite this, Scale scale, fix10_6 z)
{
	ASSERT(this, "BgmapSprite::resize: null this");

	if(__WORLD_AFFINE & this->head)
	{
		z -= _cameraPosition->z;

		fix7_9 ratio = __FIX10_6_TO_FIX7_9(__I_TO_FIX10_6(1) - __FIX10_6_EXT_DIV(z, _optical->scalingFactor));

		ratio = 0 > ratio? 0 : ratio;
		ratio = __I_TO_FIX7_9(__MAXIMUM_SCALE) < ratio? __I_TO_FIX7_9(__MAXIMUM_SCALE) : ratio;

		this->drawSpec.scale.x = __FIX7_9_MULT(scale.x, ratio);
		this->drawSpec.scale.y = __FIX7_9_MULT(scale.y, ratio);

		ASSERT(0 <= this->drawSpec.scale.x, "BgmapSprite::resize: null scale x");
		ASSERT(0 <= this->drawSpec.scale.y, "BgmapSprite::resize: null scale y");

		if(this->texture)
		{
			// apply add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
			BgmapSprite_computeDimensions(this);
		}

		if(this->param)
    	{
			this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;
		}
	}
}

/**
 * Calculate parallax
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param z				Z coordinate to base on the calculation
 */
void BgmapSprite_calculateParallax(BgmapSprite this, fix10_6 z)
{
	ASSERT(this, "BgmapSprite::calculateParallax: null this");

	this->position.z = __METERS_TO_PIXELS(z - _cameraPosition->z);
	this->position.parallax = Optics_calculateParallax(__PIXELS_TO_METERS(this->position.x), z);
}

/**
 * Retrieve the drawspec
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			DrawSpec
 */
DrawSpec BgmapSprite_getDrawSpec(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getDrawSpec: null this");

	return this->drawSpec;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 * @param evenFrame
 */
void BgmapSprite_render(BgmapSprite this, bool evenFrame)
{
	ASSERT(this, "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

	__CALL_BASE_METHOD(Sprite, render, this, evenFrame);

	if(!this->positioned)
	{
		return;
	}

	if(!this->worldLayer)
	{
		return;
	}

	static WorldAttributes* worldPointer = NULL;
	worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

	// TODO: check if required, causes that the sprite is turned off when changing the texture definition
/*
	if(!this->texture->written)
	{
		worldPointer->head = 0x0000;
		return;
	}
*/

	// set the head
	worldPointer->head = this->head | (__SAFE_CAST(BgmapTexture, this->texture))->segment;

	// get coordinates
	int gx = this->position.x + this->displacement.x - this->halfWidth;
	int gy = this->position.y + this->displacement.y - this->halfHeight;
	worldPointer->gx = gx;
	worldPointer->gy = gy;
	worldPointer->gp = this->position.parallax + this->displacement.parallax;

	// get sprite's size
	int width = this->halfWidth << 1;
	int height = this->halfHeight << 1;
	int w = width;
	int h = height;
	int myDisplacement = 0;

	worldPointer->mx = this->drawSpec.textureSource.mx;
	worldPointer->my = this->drawSpec.textureSource.my;
	worldPointer->mp = this->drawSpec.textureSource.mp;

	// cap coordinates to camera space
	if(_cameraFrustum->x0 - worldPointer->gp > gx)
	{
		worldPointer->gx = _cameraFrustum->x0 - worldPointer->gp;
		worldPointer->mx += (_cameraFrustum->x0 - worldPointer->gp - gx);
		w -= (_cameraFrustum->x0 - worldPointer->gp - gx);
	}

	if(_cameraFrustum->y0 > gy)
	{
		worldPointer->gy = _cameraFrustum->y0;
		worldPointer->my += (_cameraFrustum->y0 - gy);
		h -= (_cameraFrustum->y0 - gy);
		myDisplacement = (_cameraFrustum->y0 - gy);
	}

	if(w + worldPointer->gx >= _cameraFrustum->x1 + worldPointer->gp)
	{
		w = _cameraFrustum->x1 - worldPointer->gx + worldPointer->gp;
	}

	if (0 >= w)
	{
		worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
		worldPointer->w = 0;
		worldPointer->h = 0;
#endif
		return;
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
	if(h + worldPointer->gy >= _cameraFrustum->y1)
	{
		h = _cameraFrustum->y1 - worldPointer->gy;
	}

	if (0 >= h)
	{
		worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
		worldPointer->w = 0;
		worldPointer->h = 0;
#endif
		return;
	}

	worldPointer->w = w - __WORLD_SIZE_DISPLACEMENT;
	worldPointer->h = h - __WORLD_SIZE_DISPLACEMENT;

	// set the world size according to the zoom
	BgmapSprite_processAffineEffects(this, gx, width, myDisplacement);
	BgmapSprite_processHbiasEffects(this);
}

void BgmapSprite_processAffineEffects(BgmapSprite this, int gx, int width, int myDisplacement)
{
	ASSERT(this, "BgmapSprite::processAffineEffects: null this");

	if((__WORLD_AFFINE & this->head) && this->applyParamTableEffect)
	{
		if(!this->param)
		{
			// allocate param table space
			this->param = ParamTableManager_allocate(ParamTableManager_getInstance(), this);
    	}

		static WorldAttributes* worldPointer = NULL;
    	worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

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

void BgmapSprite_processHbiasEffects(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::processHbiasEffects: null this");

	if((__WORLD_HBIAS & this->head) && this->applyParamTableEffect)
	{
		if(!this->param)
		{
			// allocate param table space
			this->param = ParamTableManager_allocate(ParamTableManager_getInstance(), this);
    	}

		static WorldAttributes* worldPointer = NULL;
    	worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

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
void BgmapSprite_render(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::render: null this");
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
			WORLD_HEAD(this->worldLayer, 0x0000);
			return;
		}

		static WorldAttributes* worldPointer = NULL;
		worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		// set the world camera position
		int gx = __FIX10_6_TO_I(this->position.x + this->displacement.x);
		int gy = __FIX10_6_TO_I(this->position.y + this->displacement.y);

		int w = Texture_getCols(this->texture)<< 3;
		int h = Texture_getRows(this->texture)<< 3;

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
				h = Texture_getRows(this->texture)<< 3;
				int lastRow = h + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: h;

				BgmapSprite_doApplyAffineTransformations(this, lastRow);

				if(0 >= this->paramTableRow)
				{
					this->paramTableRow = -1;
				}
			}

			worldPointer->param = ((__PARAM_DISPLACEMENT(this->param) - 0x20000) >> 1) & 0xFFF0;
		}

		worldPointer->head = this->head | BgmapTexture_getSegment(__SAFE_CAST(BgmapTexture, this->texture));
	}
}
*/

/**
 * Add displacement to position
 *
 * @memberof				BgmapSprite
 * @public
 *
 * @param this				Function scope
 * @param displacement		2D position displacement
 */
void BgmapSprite_addDisplacement(BgmapSprite this, const PixelVector* displacement)
{
	ASSERT(this, "BgmapSprite::addDisplacement: null this");

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
 * @param this		Function scope
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void BgmapSprite_setMode(BgmapSprite this, u16 display, u16 mode)
{
	ASSERT(this, "BgmapSprite::setMode: null this");

	this->head &= ~(__WORLD_BGMAP | __WORLD_AFFINE | __WORLD_HBIAS);

	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);

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

			this->applyParamTableEffect = this->applyParamTableEffect ? this->applyParamTableEffect : BgmapSprite_doApplyAffineTransformations;
			break;

		case __WORLD_HBIAS:

			// set map head
			this->head = display | __WORLD_HBIAS;
			break;
	}
}

/**
 * Retrieve param table address
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			Param table address
 */
u32 BgmapSprite_getParam(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getParam: null this");

	return this->param;
}

/**
 * Set param table address
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 * @param param		Net param table address
 */
void BgmapSprite_setParam(BgmapSprite this, u32 param)
{
	ASSERT(this, "BgmapSprite::setParam: null this");

	this->param = param;

	// set flag to rewrite texture's param table
	BgmapSprite_invalidateParamTable(this);
}

/**
 * Force to refresh param table in the next render cycle
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 */
void BgmapSprite_invalidateParamTable(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::invalidateParamTable: null this");

	if(__WORLD_AFFINE & this->head)
	{
		BgmapSprite_applyAffineTransformations(this);
	}
	else if(__WORLD_HBIAS & this->head)
	{
		BgmapSprite_applyHbiasEffects(this);
	}
}

/**
 * Set drawspec
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param drawSpec		New drawSpec
 */
void BgmapSprite_setDrawSpec(BgmapSprite this, const DrawSpec* const drawSpec)
{
	ASSERT(this, "BgmapSprite::setDrawSpec: null this");

	this->drawSpec = *drawSpec;
}

/**
 * Retrieve the next param table row to calculate
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 *
 * @return				Next param table row to calculate
 */
s16 BgmapSprite_getParamTableRow(BgmapSprite this)
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
 *
 * @param this			Function scope
 * @return 				last computed row
 */
static s16 BgmapSprite_doApplyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::doApplyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if(this->param)
	{
		return Affine_applyAll(
			this->param,
			this->paramTableRow,
			// geometrically accurate, but kills the CPU
			// (0 > this->position.x? this->position.x : 0) + this->halfWidth,
			// (0 > this->position.y? this->position.y : 0) + this->halfHeight,
			// don't do translations
			__I_TO_FIX10_6(this->halfWidth),
			__I_TO_FIX10_6(this->halfHeight),
			__I_TO_FIX13_3(this->drawSpec.textureSource.mx),
			__I_TO_FIX13_3(this->drawSpec.textureSource.my),
			__I_TO_FIX10_6(this->texture->textureDefinition->cols << 2),
			__I_TO_FIX10_6(this->texture->textureDefinition->rows << 2),
			&this->drawSpec.scale,
			&this->drawSpec.rotation
		);
	}

	return this->paramTableRow;
}

/**
 * Trigger affine calculations
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 */
void BgmapSprite_applyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	this->paramTableRow = 0;
}

/**
 * Trigger h-bias calculations
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 */
void BgmapSprite_applyHbiasEffects(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyHbiasEffects: null this");
	ASSERT(this->texture, "BgmapSprite::applyHbiasEffects: null texture");

	this->paramTableRow = 0;
}
