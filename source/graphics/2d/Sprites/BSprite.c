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

#include <BSprite.h>
#include <Game.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the BSprite
__CLASS_DEFINITION(BSprite, Sprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;
extern unsigned int volatile* _xpstts;

void Sprite_onTextureRewritten(Sprite this, Object eventFirer);

static void BSprite_doScale(BSprite this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BSprite, const BSpriteDefinition* bSpriteDefinition)
__CLASS_NEW_END(BSprite, bSpriteDefinition);

// class's constructor
void BSprite_constructor(BSprite this, const BSpriteDefinition* bSpriteDefinition)
{
	__CONSTRUCT_BASE();

	// create the texture
	this->texture = BTextureManager_get(BTextureManager_getInstance(), bSpriteDefinition->textureDefinition);

	if(this->texture)
	{
		Object_addEventListener(__UPCAST(Object, this->texture), __UPCAST(Object, this), (void (*)(Object, Object))Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);

		// set texture position
		this->drawSpec.textureSource.mx = BTexture_getXOffset(this->texture) << 3;
		this->drawSpec.textureSource.my = BTexture_getYOffset(this->texture) << 3;
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
			this->head = bSpriteDefinition->display | WRLD_AFFINE | WRLD_OVR;

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
void BSprite_destructor(BSprite this)
{
	ASSERT(this, "BSprite::destructor: null this");
	ASSERT(__UPCAST(BSprite, this), "BSprite::destructor: null cast");

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
		BTextureManager_free(BTextureManager_getInstance(), this->texture);
		this->texture = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE;
}

Scale BSprite_getScale(BSprite this)
{
	ASSERT(this, "BSprite::getScale: null this");

	//  return the scale
	return this->drawSpec.scale;
}

// set the direction
void BSprite_setDirection(BSprite this, int axis, int direction)
{
	ASSERT(this, "BSprite::setDirection: null this");

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
	BSprite_invalidateParamTable(this);
}

// calculate zoom scaling factor
void BSprite_resize(BSprite this, fix19_13 z)
{
	ASSERT(this, "BSprite::resize: null this");

	z -= _screenPosition->z;
	
	Optical optical = Game_getOptical(Game_getInstance());
	fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - FIX19_13_DIV(z , optical.maximumViewDistance));

	ratio = ITOFIX7_9(__MAXIMUM_SCALE) < ratio? ITOFIX7_9(__MAXIMUM_SCALE): ratio;
	
	this->drawSpec.scale.x = ratio * (this->drawSpec.scale.x < 0 ? -1 : 1);
	this->drawSpec.scale.y = ratio;
	
	if(this->texture)
	{
		if (WRLD_AFFINE == Sprite_getMode(__UPCAST(Sprite, this)))
		{
			this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
			this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
		}
		else
		{
			this->halfWidth = FIX19_13_DIV(ITOFIX19_13((int)Texture_getCols(this->texture) << 2), (FIX7_9TOFIX19_13(this->drawSpec.scale.x)));
			this->halfHeight = FIX19_13_DIV(ITOFIX19_13((int)Texture_getRows(this->texture) << 2), (FIX7_9TOFIX19_13(this->drawSpec.scale.y)));
		}
	}

	BSprite_invalidateParamTable(this);
}

VBVec2D BSprite_getPosition(BSprite this)
{
	ASSERT(this, "BSprite::getPosition: null this");

	return this->drawSpec.position;
}

void BSprite_setPosition(BSprite this, VBVec2D position)
{
	ASSERT(this, "BSprite::setPosition: null this");

	this->drawSpec.position.x = position.x;
	this->drawSpec.position.y = position.y;
	this->drawSpec.position.z = position.z;
}

void BSprite_synchronizePosition(BSprite this, VBVec3D position3D)
{
	ASSERT(this, "BSprite::synchronizePosition: null this");

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "BSprite::setPosition: null texture");

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->drawSpec.position);

	this->renderFlag |= __UPDATE_G;
}

// calculate the parallax
void BSprite_calculateParallax(BSprite this, fix19_13 z)
{
	ASSERT(this, "BSprite::calculateParallax: null this");

	this->drawSpec.position.z = z - _screenPosition->z;
	this->drawSpec.position.parallax = Optics_calculateParallax(this->drawSpec.position.x, z);
}

// retrieve drawspec
DrawSpec BSprite_getDrawSpec(BSprite this)
{
	ASSERT(this, "BSprite::getDrawSpec: null this");

	return this->drawSpec;
}

// render a world layer with the map's information
void BSprite_render(BSprite this)
{
	ASSERT(this, "BSprite::render: null this");
	ASSERT(this->texture, "BSprite::render: null texture");

	//if render flag is set
	if (this->renderFlag)
	{
		static WORLD* worldPointer = NULL;
		worldPointer = &WA[this->worldLayer];

//		ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "BSprite::render: freeLayer >= this->worldLayer");

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
			worldPointer->head = this->head | BTexture_getBgmapSegment(this->texture);
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
					BSprite_doScale(this);
					
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
u32 BSprite_getParam(BSprite this)
{
	ASSERT(this, "BSprite::getParam: null this");

	return this->param;
}

// set map's param table address
void BSprite_setParam(BSprite this, u32 param)
{
	ASSERT(this, "BSprite::setParam: null this");

	this->param = param;

	// set flag to rewrite texture's param table
	BSprite_invalidateParamTable(this);
}

// force refresh param table in the next render
void BSprite_invalidateParamTable(BSprite this)
{
	ASSERT(this, "BSprite::invalidateParamTable: null this");

	this->renderFlag |= __UPDATE_SIZE;
	
	BSprite_scale(this);
}

// set drawspec
void BSprite_setDrawSpec(BSprite this, const DrawSpec* const drawSpec)
{
	ASSERT(this, "BSprite::setDrawSpec: null this");

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
fix19_13 BSprite_getParamTableRow(BSprite this)
{
	return this->paramTableRow;
}

//---------------------------------------------------------------------------------------------------------
// 										MAP FXs
//---------------------------------------------------------------------------------------------------------

/*
 * Affine FX
 */

void BSprite_noAFX(BSprite this, int direction)
{
	ASSERT(this, "BSprite::noAFX: null this");
}

static void BSprite_doScale(BSprite this)
{
	ASSERT(this, "BSprite::scale: null this");
	ASSERT(this->texture, "BSprite::scale: null texture");

	if (this->param)
	{
		int cols = (int)Texture_getCols(this->texture) << 2;
		int rows = ((int)Texture_getRows(this->texture) + __PARAM_TABLE_PADDING) << 2;

		this->paramTableRow = Affine_scale(this->param, this->paramTableRow, this->drawSpec.scale.x, this->drawSpec.scale.y,
				   this->drawSpec.textureSource.mx + cols,
				   this->drawSpec.textureSource.my + rows,
				   cols, rows);
	}
}

// scale sprite
void BSprite_scale(BSprite this)
{
	ASSERT(this, "BSprite::scale: null this");
	ASSERT(this->texture, "BSprite::scale: null texture");

	if (this->param)
	{
		this->paramTableRow = 0;
		
		BSprite_doScale(this);
	}
}

void BSprite_rotate(BSprite this, int angle)
{
	ASSERT(this, "BSprite::rotate: null this");
	ASSERT(this->texture, "BSprite::rotate: null texture");

	// TODO
	if (this->param)
	{
		int cols = Texture_getCols(this->texture) << 2;
		int rows = Texture_getRows(this->texture) << 2;

		Affine_rotateZ(this->param, this->drawSpec.scale.x, this->drawSpec.scale.y,
				   this->drawSpec.textureSource.mx + cols,
				   this->drawSpec.textureSource.my + rows,
				   cols, rows, angle);
	}
}

	// TODO
	/*
	static int alpha=0;
	if (this->updateParamTable==true){
		affineRotateY(this->param,alpha,this->scale.x,this->scale.y,
		(this->xOffset<<3)+(this->cols<<2), (this->yOffset<<3)+(this->rows<<2),
		(this->cols<<2),(this->rows<<2));
		if (alpha++ >125){
			alpha=125;

		}
		// put down the flag
		BSprite_setUpdateParamTableFlag(this, false);

	}
	*/
	//delay(5);

/*
 * H-Bias FX
 */

void BSprite_squeezeXHFX(BSprite this)
{
	ASSERT(this, "BSprite::squezeXHFX: null this");

	// TODO
}

void BSprite_fireHFX(BSprite this)
{
	ASSERT(this, "BSprite::fireHFX: null this");

	// TODO
}

void BSprite_waveHFX(BSprite this){
	ASSERT(this, "BSprite::waveHFX: null this");

	// TODO
}