/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Screen.h>
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::constructor: null this");

	__CONSTRUCT_BASE(Sprite, (SpriteDefinition*)bgmapSpriteDefinition, owner);

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

		this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
		this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
	}
	else
	{
		this->drawSpec.textureSource.mx = 0;
		this->drawSpec.textureSource.my = 0;
		this->drawSpec.textureSource.mp = 0;
	}

	// clear position
	this->drawSpec.position.x = 0;
	this->drawSpec.position.y = 0;
	this->drawSpec.position.z = 0;
	this->drawSpec.position.parallax = 0;

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::destructor: null cast");

	if(this->worldLayer)
	{
		// remove from sprite manager before I become invalid
		// and the VPU triggers a new render cycle
		SpriteManager_unregisterSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}

	// if affine or bgmap
	if(((__WORLD_AFFINE | __WORLD_HBIAS) & this->head) && this->param)
	{
		// free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}

	// free the texture
	if(this->texture)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->texture), __SAFE_CAST(Object, this), (EventListener)Sprite_onTextureRewritten, kEventTextureRewritten);
		BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, this->texture));
		this->texture = NULL;
	}

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::getScale: null this");

	// return the scale
	return this->drawSpec.scale;
}

/**
 * Set direction
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param axis			Axis to modify
 * @param direction		Direction value
 */
void BgmapSprite_setDirection(BgmapSprite this, int axis, int direction)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::setDirection: null this");

	switch(axis)
	{
		case __X_AXIS:

			this->drawSpec.scale.x = __ABS(this->drawSpec.scale.x) * direction;
			break;

		case __Y_AXIS:

			this->drawSpec.scale.y = __ABS(this->drawSpec.scale.y) * direction;
			break;
	}

	// scale the texture in the next render cycle
	BgmapSprite_invalidateParamTable(this);
}

/**
 * Retrieve 2D position
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			2D position
 */
VBVec2D BgmapSprite_getPosition(BgmapSprite this)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::getPosition: null this");

	return this->drawSpec.position;
}

/**
 * Set 2D position
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		New 2D position
 */
void BgmapSprite_setPosition(BgmapSprite this, const VBVec2D* position)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::setPosition: null this");

	this->drawSpec.position = *position;

	if(!this->worldLayer)
	{
		SpriteManager_registerSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}
}

/**
 * Calculate 2D position
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		3D position
 */
void BgmapSprite_position(BgmapSprite this, const VBVec3D* position)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "BgmapSprite::setPosition: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->drawSpec.position);

	this->drawSpec.position.x -= this->halfWidth;
	this->drawSpec.position.y -= this->halfHeight;

	if(!this->worldLayer)
	{
		// register with sprite manager
		SpriteManager_registerSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::rotate: null this");

	if(this->param)
	{
		this->drawSpec.rotation = *rotation;
		this->paramTableRow = -1 == this->paramTableRow ? 0 : this->paramTableRow;
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
void BgmapSprite_resize(BgmapSprite this, Scale scale, fix19_13 z)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::resize: null this");

	if(__WORLD_AFFINE & this->head)
	{
		z -= _screenPosition->z;

		fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - (z / (1 << _optical->maximumViewDistancePower)));

		ratio = ITOFIX7_9(__MAXIMUM_SCALE) < ratio? ITOFIX7_9(__MAXIMUM_SCALE) : ratio;

		this->drawSpec.scale.x = FIX7_9_MULT(scale.x, ratio * (this->drawSpec.scale.x < 0 ? -1 : 1));
		this->drawSpec.scale.y = FIX7_9_MULT(scale.y, ratio * (this->drawSpec.scale.y < 0 ? -1 : 1));

		ASSERT(this->drawSpec.scale.x, "BgmapSprite::resize: null scale x");
		ASSERT(this->drawSpec.scale.y, "BgmapSprite::resize: null scale y");

		if(this->texture)
		{
			this->halfWidth = FIX19_13_MULT(ITOFIX19_13((int)this->texture->textureDefinition->cols << 2), FIX7_9TOFIX19_13(__ABS(this->drawSpec.scale.x)))  + __0_5F_FIX19_13;
			this->halfHeight = FIX19_13_MULT(ITOFIX19_13((int)this->texture->textureDefinition->rows << 2), FIX7_9TOFIX19_13(__ABS(this->drawSpec.scale.y))) + __0_5F_FIX19_13;
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
void BgmapSprite_calculateParallax(BgmapSprite this, fix19_13 z)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::calculateParallax: null this");

	this->drawSpec.position.z = z - _screenPosition->z;
	this->drawSpec.position.parallax = Optics_calculateParallax(this->drawSpec.position.x, z);
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::getDrawSpec: null this");

	return this->drawSpec;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		BgmapSprite
 * @public
 *
 * @param this		Function scope
 */
void BgmapSprite_render(BgmapSprite this)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

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
	int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
	int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);
	worldPointer->gx = gx;
	worldPointer->gy = gy;

	// get sprite's size
	int width = FIX19_13TOI(this->halfWidth) << 1;
	int height = FIX19_13TOI(this->halfHeight) << 1;
	int w = width;
	int h = height;

	worldPointer->mx = this->drawSpec.textureSource.mx;
	worldPointer->my = this->drawSpec.textureSource.my;
	worldPointer->mp = this->drawSpec.textureSource.mp;

	// cap coordinates to screen space
//	int mxDisplacement = 0;
	if(_cameraFrustum->x0 > gx)
	{
		worldPointer->gx = _cameraFrustum->x0;
		worldPointer->mx += (_cameraFrustum->x0 - gx);
		w -= (_cameraFrustum->x0 - gx);
//		mxDisplacement = (_cameraFrustum->x0 - gx);
	}

	int myDisplacement = 0;

	if(_cameraFrustum->y0 > gy)
	{
		worldPointer->gy = _cameraFrustum->y0;
		worldPointer->my += (_cameraFrustum->y0 - gy);
		h -= (_cameraFrustum->y0 - gy);
		myDisplacement = (_cameraFrustum->y0 - gy);
	}

	worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI((this->displacement.z + this->displacement.p) & 0xFFFFE000);
//		worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z + this->displacement.p + __0_5F_FIX19_13);

	if(w + worldPointer->gx >= _cameraFrustum->x1)
	{
		w = _cameraFrustum->x1 - worldPointer->gx;
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::processAffineEffects: null this");

	if((__WORLD_AFFINE & this->head) && this->applyParamTableEffect)
	{
		if(!this->param)
		{
			// allocate param table space
			this->param = ParamTableManager_allocate(ParamTableManager_getInstance(), this);
    	}

		static WorldAttributes* worldPointer = NULL;
    	worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		// provide a little bit of performance gain by only calculation transform equations
		// for the visible rows, but causes that some sprites not be rendered completely when the
		// screen moves vertically
		// int lastRow = height + worldPointer->gy >= _cameraFrustum->y1 ? _cameraFrustum->y1 - worldPointer->gy + myDisplacement: height;
		// this->paramTableRow = this->paramTableRow ? this->paramTableRow : myDisplacement;

		// un-cap x coordinate in affine mode
		if(_cameraFrustum->x0 > gx)
		{
			worldPointer->gx = gx;
			worldPointer->w = width;
		}

		// apply scaling and add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
		worldPointer->w += 1;
		worldPointer->h += 1;

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::processHbiasEffects: null this");

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
// to clip the image to the screen space, but kill the CPU
/*
// render a world layer with the map's information
void BgmapSprite_render(BgmapSprite this)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

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

		// set the world screen position
		int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
		int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);

		int w = Texture_getCols(this->texture)<< 3;
		int h = Texture_getRows(this->texture)<< 3;

		int mxDisplacement = 0 > gx ? -gx : 0;
		int myDisplacement = 0 > gy ? -gy : 0;

		worldPointer->gx = gx > _cameraFrustum->x1 ? _cameraFrustum->x1 : 0 > gx ? 0: gx;
		worldPointer->gy = gy > _cameraFrustum->y1 ? _cameraFrustum->y1 : 0 > gy ? 0: gy;
		worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);

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

			if(_screenDisplacement->y && 0 > this->paramTableRow)
			{
				this->paramTableRow = 0;
			}

			worldPointer->w *= FIX7_9TOF(__ABS(this->drawSpec.scale.x));
			worldPointer->h *= FIX7_9TOF(__ABS(this->drawSpec.scale.y));

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
void BgmapSprite_addDisplacement(BgmapSprite this, const VBVec2D* displacement)
{
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::addDisplacement: null this");

	this->drawSpec.position.x += displacement->x;
	this->drawSpec.position.y += displacement->y;
	this->drawSpec.position.z += displacement->z;
	this->drawSpec.position.parallax += displacement->parallax;
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::setMode: null this");

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::getParam: null this");

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::setParam: null this");

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::invalidateParamTable: null this");

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::setDrawSpec: null this");

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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::doApplyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if(this->param)
	{
		return Affine_applyAll(
			this->param,
			this->paramTableRow,
			// geometrically accurate, but kills the CPU
			// (0 > this->drawSpec.position.x? this->drawSpec.position.x : 0) + this->halfWidth,
			// (0 > this->drawSpec.position.y? this->drawSpec.position.y : 0) + this->halfHeight,
			// don't do translations
			this->halfWidth,
			this->halfHeight,
			ITOFIX13_3(this->drawSpec.textureSource.mx),
			ITOFIX13_3(this->drawSpec.textureSource.my),
			this->halfWidth,
			this->halfHeight,
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::applyAffineTransformations: null this");
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
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::applyHbiasEffects: null this");
	ASSERT(this->texture, "BgmapSprite::applyHbiasEffects: null texture");

	this->paramTableRow = 0;
}
