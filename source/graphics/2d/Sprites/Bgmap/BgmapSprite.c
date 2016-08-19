/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
// 											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __WORLD_SIZE_DISPLACEMENT			1
#define __TOTAL_SIN_ENTRIES					(sizeof(SINLUT) / sizeof(u16))
#define __GX_LIMIT							511
#define __GY_LIMIT							223


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(BgmapSprite, Sprite);

__CLASS_FRIEND_DEFINITION(Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const VBVec3D* _screenDisplacement;
extern const Optical* _optical;

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);

static void BgmapSprite_doApplyAffineTransformations(BgmapSprite this);
static void BgmapSprite_doApplyHbiasTransformations(BgmapSprite this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapSprite, const BgmapSpriteDefinition* bSpriteDefinition, Object owner)
__CLASS_NEW_END(BgmapSprite, bSpriteDefinition, owner);

// class's constructor
void BgmapSprite_constructor(BgmapSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
{
	__CONSTRUCT_BASE(Sprite, (SpriteDefinition*)bgmapSpriteDefinition, owner);

	// create the texture
	if(bgmapSpriteDefinition->spriteDefinition.textureDefinition)
	{
		this->texture = __SAFE_CAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), bgmapSpriteDefinition->spriteDefinition.textureDefinition));
		ASSERT(this->texture, "BgmapSprite::constructor: null texture");
	}

	if(this->texture)
	{
		Object_addEventListener(__SAFE_CAST(Object, this->texture), __SAFE_CAST(Object, this), (EventListener)Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);

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

	this->drawSpec.scale.x = ITOFIX7_9(1);
	this->drawSpec.scale.y = ITOFIX7_9(1);

	this->drawSpec.rotation.x = 0;
	this->drawSpec.rotation.y = 0;
	this->drawSpec.rotation.z = 0;

	this->displacement = bgmapSpriteDefinition->spriteDefinition.displacement;

	this->param = 0;
	this->paramTableRow = -1;

	// set world layer's head acording to map's render mode
	switch(bgmapSpriteDefinition->bgmapMode)
	{
		case __WORLD_BGMAP:

			// set map head
			this->head = bgmapSpriteDefinition->display | __WORLD_BGMAP;

			break;

		case __WORLD_AFFINE:

			// set map head
			this->head = bgmapSpriteDefinition->display | __WORLD_AFFINE;

			// allocate param table space
			ParamTableManager_allocate(ParamTableManager_getInstance(), this);

			break;

		case __WORLD_HBIAS:

			// set map head
			this->head = bgmapSpriteDefinition->display | __WORLD_HBIAS | __WORLD_OVR;

			break;
	}
}

// class's destructor
void BgmapSprite_destructor(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::destructor: null this");
	ASSERT(__SAFE_CAST(BgmapSprite, this), "BgmapSprite::destructor: null cast");

	// make sure I'm hidden
	__VIRTUAL_CALL(Sprite, hide, this);

    if(this->worldLayer)
    {
        // remove from sprite manager before I become invalid
        // and the VPU triggers a new render cycle
        SpriteManager_relinquishWorldLayer(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}

	// if affine or bgmap
	if(__WORLD_AFFINE & this->head)
	{
		// free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}

	// free the texture
	if(this->texture)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->texture), __SAFE_CAST(Object, this), (EventListener)Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);
		BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, this->texture));
		this->texture = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

Scale BgmapSprite_getScale(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getScale: null this");

	//  return the scale
	return this->drawSpec.scale;
}

// set the direction
void BgmapSprite_setDirection(BgmapSprite this, int axis, int direction)
{
	ASSERT(this, "BgmapSprite::setDirection: null this");

	switch(axis)
	{
		case __XAXIS:

			this->drawSpec.scale.x = FIX7_9_MULT(abs(this->drawSpec.scale.x), ITOFIX7_9(direction));
			break;

		case __YAXIS:

			this->drawSpec.scale.y = FIX7_9_MULT(abs(this->drawSpec.scale.y), ITOFIX7_9(direction));
			break;
	}

	// scale the texture in the next render cycle
	BgmapSprite_invalidateParamTable(this);
}

// calculate zoom scaling factor
void BgmapSprite_resize(BgmapSprite this, Scale scale, fix19_13 z)
{
	ASSERT(this, "BgmapSprite::resize: null this");

	z -= _screenPosition->z;

	fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - (z >> _optical->maximumViewDistancePower));

	ratio = ITOFIX7_9(__MAXIMUM_SCALE) < ratio? ITOFIX7_9(__MAXIMUM_SCALE): ratio;

	this->drawSpec.scale.x = FIX7_9_MULT(scale.x, ratio * (this->drawSpec.scale.x < 0 ? -1 : 1));
	this->drawSpec.scale.y = FIX7_9_MULT(scale.y, ratio * (this->drawSpec.scale.y < 0 ? -1 : 1));

	ASSERT(this->drawSpec.scale.x, "BgmapSprite::resize: null scale x");
	ASSERT(this->drawSpec.scale.y, "BgmapSprite::resize: null scale y");

	if(this->texture)
	{
        this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
        this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
	}

	if(this->param)
	{
		this->paramTableRow = -1 == this->paramTableRow? 0: this->paramTableRow;
	}
}

VBVec2D BgmapSprite_getPosition(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getPosition: null this");

	return this->drawSpec.position;
}

void BgmapSprite_setPosition(BgmapSprite this, const VBVec2D* position)
{
	ASSERT(this, "BgmapSprite::setPosition: null this");

	this->drawSpec.position = *position;

	if(!this->worldLayer)
	{
    	Sprite_setWorldLayer(__SAFE_CAST(Sprite, this), SpriteManager_getWorldLayer(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this)));
    }
}

void BgmapSprite_position(BgmapSprite this, const VBVec3D* position)
{
	ASSERT(this, "BgmapSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "BgmapSprite::setPosition: null texture");

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->drawSpec.position);

	this->drawSpec.position.x -= this->halfWidth;
	this->drawSpec.position.y -= this->halfHeight;

	this->renderFlag |= __UPDATE_G;

	if(!this->worldLayer)
	{
		// register with sprite manager
    	Sprite_setWorldLayer(__SAFE_CAST(Sprite, this), SpriteManager_getWorldLayer(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this)));
    }
}

void BgmapSprite_rotate(BgmapSprite this, const Rotation* rotation)
{
	ASSERT(this, "BgmapSprite::rotate: null this");

	this->drawSpec.rotation.x = rotation->x % __TOTAL_SIN_ENTRIES;
	this->drawSpec.rotation.y = rotation->y % __TOTAL_SIN_ENTRIES;
	this->drawSpec.rotation.z = rotation->z % __TOTAL_SIN_ENTRIES;
}

// calculate the parallax
void BgmapSprite_calculateParallax(BgmapSprite this, fix19_13 z)
{
	ASSERT(this, "BgmapSprite::calculateParallax: null this");

	this->drawSpec.position.z = z;
	this->drawSpec.position.parallax = Optics_calculateParallax(this->drawSpec.position.x, z);
}

// retrieve drawspec
DrawSpec BgmapSprite_getDrawSpec(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getDrawSpec: null this");

	return this->drawSpec;
}

// render a world layer with the map's information
void BgmapSprite_render(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

	// if render flag is set
	if(this->renderFlag && this->worldLayer)
	{
		static WorldAttributes* worldPointer = NULL;
		worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		if(this->hidden)
		{
			worldPointer->head = 0x0000;
			this->renderFlag = 0;
			return;
		}

		// get coordinates
        int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
        int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);

        // get sprite's size
        int width = Texture_getCols(this->texture)<< 3;
        int height = Texture_getRows(this->texture)<< 3;
        int w = width;
        int h = height;

        // cap coordinates to screen space
        worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : 0 > gx? 0: gx;
        worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : 0 > gy? 0: gy;
        worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);

        // clip texture's source to screen space
        int mxDisplacement = 0 > gx? -gx : 0;
        int myDisplacement = 0 > gy? -gy : 0;

        worldPointer->mx = this->drawSpec.textureSource.mx + mxDisplacement;
        worldPointer->my = this->drawSpec.textureSource.my + myDisplacement;
        worldPointer->mp = this->drawSpec.textureSource.mp;

        // clip sprite's size to screen space
        // reduce by 1 since 0 means 1 in the VB
        w = w - __WORLD_SIZE_DISPLACEMENT - mxDisplacement;
        h = h - __WORLD_SIZE_DISPLACEMENT - myDisplacement;

        w = w + worldPointer->gx >= __SCREEN_WIDTH? __SCREEN_WIDTH - worldPointer->gx : w;
        h = h + worldPointer->gy >= __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy : h;

        if (0 > w)
        {
            w = 0;
            worldPointer->gx = -__G_DISPLACEMENT_BECAUSE_WH_0_EQUALS_1;
            worldPointer->gp = 0;
        }

        if (0 > h)
        {
            h = 0;
            worldPointer->gy = -__G_DISPLACEMENT_BECAUSE_WH_0_EQUALS_1;
        }

        worldPointer->w = w;
        worldPointer->h = h;

        // affine transformations may be deferred
        bool clearRenderFlagValue = false;

        // set the world size according to the zoom
        if(__WORLD_AFFINE & this->head)
        {
            // set param table's souce
            worldPointer->param = (u16)((((this->param + (myDisplacement << 4))) - 0x20000) >> 1) & 0xFFF0;

            // un-cap x coordinate in affine mode
            if(0 > gx)
            {
                worldPointer->gx = gx;
                worldPointer->w = width;
            }

            // apply scaling and add 1 pixel to the width and 7 to the height to avoid cutting off the graphics
            worldPointer->w = FIX19_13TOI(FIX19_13_MULT(ITOFIX19_13(worldPointer->w), FIX7_9TOFIX19_13(abs(this->drawSpec.scale.x)))) + 1;
            worldPointer->h = FIX19_13TOI(FIX19_13_MULT(ITOFIX19_13(worldPointer->h), FIX7_9TOFIX19_13(abs(this->drawSpec.scale.y)))) + 1;

             if(0 <= this->paramTableRow)
            {

                // provide a little bit of performance gain by only calculation transform equations
                // for the visible rows, but causes that some sprites not be rendered completely when the
                // screen moves vertically
                // int lastRow = height + worldPointer->gy >= __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy + myDisplacement: height;
                // this->paramTableRow = this->paramTableRow? this->paramTableRow : myDisplacement;

                // apply affine transformation
                BgmapSprite_doApplyAffineTransformations(this);

                if(0 < this->paramTableRow)
                {
                    // keep rendering in the next cycle if affine transformation
                    // is deferred
                    clearRenderFlagValue = __UPDATE_SIZE;
                }
                else
                {
                    this->paramTableRow = -1;
                }
            }
        }
		else if(__WORLD_HBIAS & this->head)
		{
			BgmapSprite_doApplyHbiasTransformations(this);
		}

        // set the head
        worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__SAFE_CAST(BgmapTexture, this->texture));

		// make sure to not render again
		this->renderFlag = clearRenderFlagValue;
	}
}

// handles affine transformations accurately by translating using translations
// to clip the image to the screen space, but kill the CPU
/*
// render a world layer with the map's information
void BgmapSprite_render(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::render: null this");
	ASSERT(this->texture, "BgmapSprite::render: null texture");

	// if render flag is set
	if(this->renderFlag && this->initialized)
	{
		if(this->hidden)
		{
			WORLD_HEAD(this->worldLayer, 0x0000);
			this->renderFlag = 0;
			return;
		}

		static WorldAttributes* worldPointer = NULL;
		worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		// set the world screen position
        int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x);
        int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y);

        bool clearRenderFlagValue = false;

        int w = Texture_getCols(this->texture)<< 3;
        int h = Texture_getRows(this->texture)<< 3;

        int mxDisplacement = 0 > gx? -gx : 0;
        int myDisplacement = 0 > gy? -gy : 0;

        worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : 0 > gx? 0: gx;
        worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : 0 > gy? 0: gy;
        worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI(this->displacement.z & 0xFFFFE000);

        worldPointer->mx = this->drawSpec.textureSource.mx + mxDisplacement;
        worldPointer->my = this->drawSpec.textureSource.my + myDisplacement;
        worldPointer->mp = this->drawSpec.textureSource.mp;

        // -1 because 0 means 1 pixel for width
        w = w - __WORLD_SIZE_DISPLACEMENT - mxDisplacement;
        h = h - __WORLD_SIZE_DISPLACEMENT - myDisplacement;

        worldPointer->w = 0;
        worldPointer->h = 0;

        if(w + worldPointer->gx >= __SCREEN_WIDTH)
        {
            worldPointer->w = __SCREEN_WIDTH - worldPointer->gx;
        }
        else if (0 <= w)
        {
            worldPointer->w = w;
        }

        if(h + worldPointer->gy >= __SCREEN_HEIGHT)
        {
            worldPointer->h = __SCREEN_HEIGHT - worldPointer->gy;
        }
        else if (0 <= h)
        {
            worldPointer->h = h;
        }

        // set the world size according to the zoom
        if(__WORLD_AFFINE & this->head)
        {
            if(this->renderFlag & __UPDATE_G)
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

            worldPointer->w *= FIX7_9TOF(abs(this->drawSpec.scale.x));
            worldPointer->h *= FIX7_9TOF(abs(this->drawSpec.scale.y));

             if(0 <= this->paramTableRow)
            {
                h = Texture_getRows(this->texture)<< 3;
                int lastRow = h + worldPointer->gy >= __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy + myDisplacement: h;

                BgmapSprite_doApplyAffineTransformations(this, lastRow);

                if(0 < this->paramTableRow)
                {
                    clearRenderFlagValue = __UPDATE_SIZE;
                }
                else
                {
                    this->paramTableRow = -1;
                }
            }

            worldPointer->param = ((__PARAM_DISPLACEMENT(this->param) - 0x20000) >> 1) & 0xFFF0;
        }

        worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__SAFE_CAST(BgmapTexture, this->texture));

		// make sure to not render again
		this->renderFlag = clearRenderFlagValue;
	}
}
*/

void BgmapSprite_addDisplacement(BgmapSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "BgmapSprite::addDisplacement: null this");

	this->drawSpec.position.x += displacement->x;
	this->drawSpec.position.y += displacement->y;
	this->drawSpec.position.z += displacement->z;
	this->drawSpec.position.parallax += displacement->parallax;

	Sprite_setRenderFlag(__SAFE_CAST(Sprite, this), __UPDATE_G);
}


// get map's param table address
u32 BgmapSprite_getParam(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getParam: null this");

	return this->param;
}

// set map's param table address
void BgmapSprite_setParam(BgmapSprite this, u32 param)
{
	ASSERT(this, "BgmapSprite::setParam: null this");

	this->param = param;

	// set flag to rewrite texture's param table
	BgmapSprite_invalidateParamTable(this);
}

// force refresh param table in the next render
void BgmapSprite_invalidateParamTable(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::invalidateParamTable: null this");

	this->renderFlag |= __UPDATE_SIZE;

	BgmapSprite_applyAffineTransformations(this);
}

// set drawspec
void BgmapSprite_setDrawSpec(BgmapSprite this, const DrawSpec* const drawSpec)
{
	ASSERT(this, "BgmapSprite::setDrawSpec: null this");

	this->drawSpec = *drawSpec;
}

// retrieve param table current row
fix19_13 BgmapSprite_getParamTableRow(BgmapSprite this)
{
	return this->paramTableRow;
}


//---------------------------------------------------------------------------------------------------------
// 										MAP FXs
//---------------------------------------------------------------------------------------------------------

static void BgmapSprite_doApplyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::doApplyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if(this->param)
	{
		this->paramTableRow = Affine_applyAll(
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
}

static void BgmapSprite_doApplyHbiasTransformations(BgmapSprite this __attribute__ ((unused)))
{
	ASSERT(this, "BgmapSprite::doApplyHbiasTransformations: null this");
}

void BgmapSprite_applyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	this->paramTableRow = 0;
}

void BgmapSprite_applyHbiasTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	this->paramTableRow = 0;
}
