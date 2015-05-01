/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>
#include <Game.h>
#include <Optics.h>
#include <SpriteManager.h>
#include <ParamTableManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1
#define __TOTAL_SIN_ENTRIES				(sizeof(SINLUT) / sizeof(u16))


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the BgmapSprite
__CLASS_DEFINITION(BgmapSprite, Sprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;
extern unsigned int volatile* _xpstts;

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);

static void BgmapSprite_doApplyAffineTransformations(BgmapSprite this);
static void BgmapSprite_doApplyHbiasTransformations(BgmapSprite this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapSprite, const BgmapSpriteDefinition* bSpriteDefinition)
__CLASS_NEW_END(BgmapSprite, bSpriteDefinition);

// class's constructor
void BgmapSprite_constructor(BgmapSprite this, const BgmapSpriteDefinition* bSpriteDefinition)
{
	__CONSTRUCT_BASE();

	// register with sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

	// create the texture
	if(bSpriteDefinition->textureDefinition)
	{
		this->texture = __UPCAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), bSpriteDefinition->textureDefinition));
	}
	
	if(this->texture)
	{
		Object_addEventListener(__UPCAST(Object, this->texture), __UPCAST(Object, this), (void (*)(Object, Object))Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);

		// set texture position
		this->drawSpec.textureSource.mx = BgmapTexture_getXOffset(__UPCAST(BgmapTexture, this->texture)) << 3;
		this->drawSpec.textureSource.my = BgmapTexture_getYOffset(__UPCAST(BgmapTexture, this->texture)) << 3;
		this->drawSpec.textureSource.mp = 0;
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

	this->parallaxDisplacement = bSpriteDefinition->parallaxDisplacement;

	this->param = 0;
	this->paramTableRow = -1;

	//this->head = bSpriteDefinition->display | WRLD_BGMAP;
	//set world layer's head acording to map's render mode
	switch (bSpriteDefinition->bgmapMode)
	{
		case WRLD_BGMAP:

			//set map head
			this->head = bSpriteDefinition->display | WRLD_BGMAP;

			break;

		case WRLD_AFFINE:

			//set map head
			this->head = bSpriteDefinition->display | WRLD_AFFINE;

			//allocate param table space
			ParamTableManager_allocate(ParamTableManager_getInstance(), this);

			break;

		case WRLD_HBIAS:

			//set map head
			this->head = bSpriteDefinition->display | WRLD_HBIAS | WRLD_OVR;

			break;
	}
}

// class's destructor
void BgmapSprite_destructor(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::destructor: null this");
	ASSERT(__UPCAST(BgmapSprite, this), "BgmapSprite::destructor: null cast");

	//if affine or bgmap
	if (WRLD_AFFINE & this->head)
	{
		//free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}

	// free the texture
	if(this->texture)
	{
		Object_removeEventListener(__UPCAST(Object, this->texture), __UPCAST(Object, this), (void (*)(Object, Object))Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);
		BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __UPCAST(BgmapTexture, this->texture));
		this->texture = NULL;
	}
	
	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), __UPCAST(Sprite, this));

	// destroy the super object
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

	switch (axis)
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
	
	fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - FIX19_13_DIV(z , _optical->maximumViewDistance));

	ratio = ITOFIX7_9(__MAXIMUM_SCALE) < ratio? ITOFIX7_9(__MAXIMUM_SCALE): ratio;
	
	this->drawSpec.scale.x = FIX7_9_MULT(scale.x, ratio * (this->drawSpec.scale.x < 0 ? -1 : 1));
	this->drawSpec.scale.y = FIX7_9_MULT(scale.y, ratio * (this->drawSpec.scale.y < 0 ? -1 : 1));

	ASSERT(this->drawSpec.scale.x, "BgmapSprite::resize: null scale x");
	ASSERT(this->drawSpec.scale.y, "BgmapSprite::resize: null scale y");
	
	if(this->texture)
	{
		if (WRLD_AFFINE == Sprite_getMode(__UPCAST(Sprite, this)))
		{
			this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
			this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
		}
		else
		{
			this->halfWidth = FIX19_13_DIV(ITOFIX19_13((int)Texture_getCols(this->texture) << 2), abs((FIX7_9TOFIX19_13(this->drawSpec.scale.x))));
			this->halfHeight = FIX19_13_DIV(ITOFIX19_13((int)Texture_getRows(this->texture) << 2), (FIX7_9TOFIX19_13(this->drawSpec.scale.y)));
		}
	}

	BgmapSprite_invalidateParamTable(this);
}

VBVec2D BgmapSprite_getPosition(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getPosition: null this");

	return this->drawSpec.position;
}

void BgmapSprite_setPosition(BgmapSprite this, VBVec2D position)
{
	ASSERT(this, "BgmapSprite::setPosition: null this");

	this->drawSpec.position.x = position.x;
	this->drawSpec.position.y = position.y;
	this->drawSpec.position.z = position.z;
}

void BgmapSprite_synchronizePosition(BgmapSprite this, VBVec3D position3D)
{
	ASSERT(this, "BgmapSprite::synchronizePosition: null this");

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "BgmapSprite::setPosition: null texture");

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->drawSpec.position);

	this->renderFlag |= __UPDATE_G;
}

void BgmapSprite_synchronizeRotation(BgmapSprite this, Rotation rotation)
{
	ASSERT(this, "BgmapSprite::synchronizeRotation: null this");

	this->drawSpec.rotation.x = rotation.x % __TOTAL_SIN_ENTRIES; 
	this->drawSpec.rotation.y = rotation.y % __TOTAL_SIN_ENTRIES; 
	this->drawSpec.rotation.z = rotation.z % __TOTAL_SIN_ENTRIES; 
}

// calculate the parallax
void BgmapSprite_calculateParallax(BgmapSprite this, fix19_13 z)
{
	ASSERT(this, "BgmapSprite::calculateParallax: null this");

	this->drawSpec.position.z = z - _screenPosition->z;
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

	//if render flag is set
	if (this->renderFlag)
	{
		static WORLD* worldPointer = NULL;
		worldPointer = &WA[this->worldLayer];

//		ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "BgmapSprite::render: freeLayer >= this->worldLayer");

		if (__UPDATE_HEAD == this->renderFlag)
		{
			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;
			worldPointer->gx = FIX19_13TOI(this->drawSpec.position.x);
			worldPointer->gp = this->drawSpec.position.parallax + this->parallaxDisplacement;
			worldPointer->gy = FIX19_13TOI(this->drawSpec.position.y);

			//set the world size according to the zoom
			if (WRLD_AFFINE & this->head)
			{
				worldPointer->param = ((__PARAM_DISPLACEMENT(this->param) - 0x20000) >> 1) & 0xFFF0;
				worldPointer->w = ((int)Texture_getCols(this->texture)<< 3) * FIX7_9TOF(abs(this->drawSpec.scale.x)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = ((int)Texture_getRows(this->texture)<< 3) * FIX7_9TOF(abs(this->drawSpec.scale.y)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3);
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3);
			}
			
			// make sure to not render again
			worldPointer->head = this->head | BgmapTexture_getBgmapSegment(__UPCAST(BgmapTexture, this->texture));
			this->renderFlag = 0 < this->paramTableRow? __UPDATE_SIZE: false;
			return;
		}
		
		//set the world screen position
		if (this->renderFlag & __UPDATE_M)
		{
			worldPointer->mx = this->drawSpec.textureSource.mx;
			worldPointer->mp = this->drawSpec.textureSource.mp;
			worldPointer->my = this->drawSpec.textureSource.my;
		}
		
		//set the world screen position
		if (this->renderFlag & __UPDATE_G)
		{
			worldPointer->gx = FIX19_13TOI(this->drawSpec.position.x);
			worldPointer->gp = this->drawSpec.position.parallax + this->parallaxDisplacement;
			worldPointer->gy = FIX19_13TOI(this->drawSpec.position.y);
		}

		if (this->renderFlag & __UPDATE_SIZE)
		{
			//set the world size according to the zoom
			if (WRLD_AFFINE & this->head)
			{
				if(0 < this->paramTableRow)
				{
					BgmapSprite_doApplyAffineTransformations(this);
					
					if(0 < this->paramTableRow)
					{
						this->renderFlag = __UPDATE_SIZE;
						return;
					}
				}

				worldPointer->param = ((__PARAM_DISPLACEMENT(this->param) - 0x20000) >> 1) & 0xFFF0;
				worldPointer->w = ((int)Texture_getCols(this->texture)<< 3) * FIX7_9TOF(abs(this->drawSpec.scale.x)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
				worldPointer->h = ((int)Texture_getRows(this->texture)<< 3) * FIX7_9TOF(abs(this->drawSpec.scale.y)) - __ACCOUNT_FOR_BGMAP_PLACEMENT;
			}
			else
			{
				worldPointer->w = (((int)Texture_getCols(this->texture))<< 3);
				worldPointer->h = (((int)Texture_getRows(this->texture))<< 3);
			}
		}

		// make sure to not render again
		this->renderFlag = false;
	}
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

	this->drawSpec.position.x = drawSpec->position.x;
	this->drawSpec.position.y = drawSpec->position.y;
	this->drawSpec.position.z = drawSpec->position.z;

	this->drawSpec.textureSource.mx = drawSpec->textureSource.mx;
	this->drawSpec.textureSource.my = drawSpec->textureSource.my;
	this->drawSpec.textureSource.mp = drawSpec->textureSource.mp;

	this->drawSpec.scale.x = drawSpec->scale.x;
	this->drawSpec.scale.y = drawSpec->scale.y;
}

// retrieve param table current row
fix19_13 BgmapSprite_getParamTableRow(BgmapSprite this)
{
	return this->paramTableRow;
}

//---------------------------------------------------------------------------------------------------------
// 										MAP FXs
//---------------------------------------------------------------------------------------------------------

/*
 * Affine FX
 */

void BgmapSprite_noAFX(BgmapSprite this, int direction)
{
	ASSERT(this, "BgmapSprite::noAFX: null this");
}


static void BgmapSprite_doApplyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::doApplyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::doApplyAffineTransformations: null texture");

	if (this->param)
	{
		int halfWidth = (int)Texture_getCols(this->texture) << 2;
		int halfHeight = ((int)Texture_getRows(this->texture) + __PARAM_TABLE_PADDING) << 2;

		this->paramTableRow = Affine_applyAll(
				this->param, 
				this->paramTableRow, 
				&this->drawSpec.scale, 
				&this->drawSpec.rotation, 
				&this->drawSpec.textureSource,
				halfWidth,
				halfHeight
		);
	}
}

static void BgmapSprite_doApplyHbiasTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::doApplyHbiasTransformations: null this");
}

void BgmapSprite_applyAffineTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	if (this->param)
	{
		this->paramTableRow = 0;
		
		BgmapSprite_doApplyAffineTransformations(this);
	}
}

void BgmapSprite_applyHbiasTransformations(BgmapSprite this)
{
	ASSERT(this, "BgmapSprite::applyAffineTransformations: null this");
	ASSERT(this->texture, "BgmapSprite::applyAffineTransformations: null texture");

	if (this->param)
	{
		this->paramTableRow = 0;
		
		BgmapSprite_doApplyHbiasTransformations(this);
	}
}
