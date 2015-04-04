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

#include <Sprite.h>
#include <Game.h>
#include <SpriteManager.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <HardwareManager.h>
#include <Screen.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT	1


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Sprite
__CLASS_DEFINITION(Sprite, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;

static void Sprite_onTextureRewritten(Sprite this, Object eventFirer);
static void Sprite_doScale(Sprite this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Sprite, const SpriteDefinition* spriteDefinition)
__CLASS_NEW_END(Sprite, spriteDefinition);

// class's constructor
void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition)
{
	__CONSTRUCT_BASE();

	// create the texture
	this->texture = TextureManager_get(TextureManager_getInstance(), spriteDefinition->textureDefinition);

	if(this->texture)
	{
		Object_addEventListener(__UPCAST(Object, this->texture), __UPCAST(Object, this), (void (*)(Object, Object))Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);

		// set texture position
		this->drawSpec.textureSource.mx = Texture_getXOffset(this->texture) << 3;
		this->drawSpec.textureSource.my = Texture_getYOffset(this->texture) << 3;
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

	this->parallaxDisplacement = spriteDefinition->parallaxDisplacement;

	this->param = 0;
	this->paramTableRow = -1;

	//this->head = spriteDefinition->display | WRLD_BGMAP;
	//set world layer's head acording to map's render mode
	switch (spriteDefinition->bgmapMode)
	{
		case WRLD_BGMAP:

			//set map head
			this->head = spriteDefinition->display | WRLD_BGMAP;

			break;

		case WRLD_AFFINE:

			//set map head
			this->head = spriteDefinition->display | WRLD_AFFINE | WRLD_OVR;

			//allocate param table space
			ParamTableManager_allocate(ParamTableManager_getInstance(), this);

			break;

		case WRLD_HBIAS:

			//set map head
			this->head = spriteDefinition->display | WRLD_HBIAS | WRLD_OVR;

			break;
	}

	// set the default layer
	this->worldLayer = 0;

	// set the render flag
	this->renderFlag = 0;

	// register with sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), this);
}

// class's destructor
void Sprite_destructor(Sprite this)
{
	ASSERT(this, "Sprite::destructor: null this");
	ASSERT(__UPCAST(Sprite, this), "Sprite::destructor: null cast");

	Sprite_hide(this);

	//if affine or bgmap
	if (WRLD_AFFINE & this->head)
	{
		//free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}

	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), this);

	// free the texture
	if(this->texture)
	{
		Object_removeEventListener(__UPCAST(Object, this->texture), __UPCAST(Object, this), (void (*)(Object, Object))Sprite_onTextureRewritten, __EVENT_TEXTURE_REWRITTEN);
		TextureManager_free(TextureManager_getInstance(), this->texture);
		this->texture = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE;
}

Scale Sprite_getScale(Sprite this)
{
	ASSERT(this, "Sprite::getScale: null this");

	//  return the scale
	return this->drawSpec.scale;
}

// set the direction
void Sprite_setDirection(Sprite this, int axis, int direction)
{
	ASSERT(this, "Sprite::setDirection: null this");

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
	Sprite_invalidateParamTable(this);
}

// calculate zoom scaling factor
void Sprite_calculateScale(Sprite this, fix19_13 z)
{
	ASSERT(this, "Sprite::calculateScale: null this");

	z -= _screenPosition->z;
	
	Optical optical = Game_getOptical(Game_getInstance());
	fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - FIX19_13_DIV(z , optical.maximumViewDistance));

	ratio = ITOFIX7_9(__MAXIMUM_SCALE) < ratio? ITOFIX7_9(__MAXIMUM_SCALE): ratio;
	
	this->drawSpec.scale.x = ratio * (this->drawSpec.scale.x < 0 ? -1 : 1);
	this->drawSpec.scale.y = ratio;
	
	if(this->texture)
	{
		if (WRLD_AFFINE == Sprite_getMode(this))
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

	Sprite_invalidateParamTable(this);
}

void Sprite_roundDrawSpec(Sprite this)
{
	ASSERT(this, "Sprite::roundDrawSpec: null this");

	this->drawSpec.position.x &= 0xFFFFE000;
	this->drawSpec.position.y &= 0xFFFFE000;
	this->drawSpec.position.z &= 0xFFFFE000;
}

void Sprite_setPosition(Sprite this, VBVec3D position3D)
{
	ASSERT(this, "Sprite::setPosition: null this");

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	ASSERT(this->texture, "Sprite::setPosition: null texture");

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->drawSpec.position);

	this->renderFlag |= __UPDATE_G;
}

// calculate the parallax
void Sprite_calculateParallax(Sprite this, fix19_13 z)
{
	ASSERT(this, "Sprite::calculateParallax: null this");

	this->drawSpec.position.z = z - _screenPosition->z;
	this->drawSpec.position.parallax = Optics_calculateParallax(this->drawSpec.position.x, z);
}

// retrieve the texture
Texture Sprite_getTexture(Sprite this)
{
	ASSERT(this, "Sprite::getTexture: null this");

	return this->texture;
}

// retrieve drawspec
DrawSpec Sprite_getDrawSpec(Sprite this)
{
	ASSERT(this, "Sprite::getDrawSpec: null this");

	return this->drawSpec;
}

// set to true to allow render
void Sprite_setRenderFlag(Sprite this, bool renderFlag)
{
	ASSERT(this, "Sprite::setRenderFlag: null this");

	// do not override the whole world entry, or will be updated in the
	// next render
	if (__UPDATE_HEAD != this->renderFlag || !renderFlag)
	{
		this->renderFlag = !renderFlag ? 0 : this->renderFlag | renderFlag;
	}
}

// show
void Sprite_show(Sprite this)
{
	this->renderFlag = __UPDATE_HEAD;
}

// hide
void Sprite_hide(Sprite this)
{
	ASSERT(this, "Sprite::hide: null this");
	ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "Sprite::hide: freeLayer >= this->worldLayer");

	WORLD_HEAD(this->worldLayer, 0x0000);
}

// preset the WORLD values before showing it by settings its head attribute
void Sprite_preRender(Sprite this)
{
	ASSERT(this, "Sprite::preRender: null this");
	ASSERT(this->texture, "Sprite::preRender: null texture");

	WORLD* worldPointer = &WA[this->worldLayer];
	ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "Sprite::preRender: freeLayer >= this->worldLayer");

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
	this->renderFlag = false;
}

// render a world layer with the map's information
void Sprite_render(Sprite this)
{
	ASSERT(this, "Sprite::render: null this");
	ASSERT(this->texture, "Sprite::render: null texture");

	//if render flag is set
	if (this->renderFlag)
	{
		extern unsigned int volatile* _xpstts;
		while (*_xpstts & XPBSYR);

		WORLD* worldPointer = &WA[this->worldLayer];

		ASSERT(SpriteManager_getFreeLayer(SpriteManager_getInstance()) < this->worldLayer, "Sprite::render: freeLayer >= this->worldLayer");

		if (__UPDATE_HEAD == this->renderFlag)
		{
			Sprite_preRender(this);
			worldPointer->head = this->head | Texture_getBgmapSegment(this->texture);
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
					Sprite_doScale(this);
					
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

// get render flag
u32 Sprite_getRenderFlag(Sprite this)
{
	ASSERT(this, "Sprite::getRenderFlag: null this");

	return this->renderFlag;
}

// get map's param table address
u32 Sprite_getParam(Sprite this)
{
	ASSERT(this, "Sprite::getParam: null this");

	return this->param;
}

// set map's param table address
void Sprite_setParam(Sprite this, u32 param)
{
	ASSERT(this, "Sprite::setParam: null this");

	this->param = param;

	// set flag to rewrite texture's param table
	Sprite_invalidateParamTable(this);
}

// set map's world layer
void Sprite_setWorldLayer(Sprite this, u8 worldLayer)
{
	ASSERT(this, "Sprite::setWorldLayer: null this");

	if (this->worldLayer != worldLayer)
	{
		this->worldLayer = worldLayer;
	
		// make sure everything is setup in the next render cycle
		this->renderFlag = __UPDATE_HEAD;
	}
}

//get map's world layer
u8 Sprite_getWorldLayer(Sprite this)
{
	ASSERT(this, "Sprite::getWorldLayer: null this");

	return this->worldLayer;
}

// get sprite's render head
u16 Sprite_getHead(Sprite this)
{
	ASSERT(this, "Sprite::getHead: null this");

	return this->head;
}

// get map's render mode
u16 Sprite_getMode(Sprite this)
{
	ASSERT(this, "Sprite::getMode: null this");

	return this->head & 0x3000;
}

// force refresh param table in the next render
void Sprite_invalidateParamTable(Sprite this)
{
	ASSERT(this, "Sprite::invalidateParamTable: null this");

	this->renderFlag |= __UPDATE_SIZE;
	
	Sprite_scale(this);
}

// reload the sprite in bgmap memory
void Sprite_rewrite(Sprite this)
{
	ASSERT(this, "Sprite::reload: null this");

	if(this->texture)
	{
		// write it in graphical memory
		Texture_rewrite(this->texture);
	}
	
	// raise flag to render again
	Sprite_show(this);
}

// process event
static void Sprite_onTextureRewritten(Sprite this, Object eventFirer)
{
	// scale again
	Sprite_scale(this);
	
	// raise flag to render again
	Sprite_show(this);
}

// set drawspec
void Sprite_setDrawSpec(Sprite this, const DrawSpec* const drawSpec)
{
	ASSERT(this, "Sprite::setDrawSpec: null this");

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
fix19_13 Sprite_getParamTableRow(Sprite this)
{
	return this->paramTableRow;
}

// get render flag
u8 Sprite_getParallaxDisplacement(Sprite this)
{
	ASSERT(this, "Sprite::getRenderFlag: null this");

	return this->parallaxDisplacement;
}

//---------------------------------------------------------------------------------------------------------
// 										MAP FXs
//---------------------------------------------------------------------------------------------------------

// write directly to texture
void Sprite_putChar(Sprite this, Point* texturePixel, BYTE* newChar)
{
	ASSERT(this, "Sprite::putChar: null this");

	if(this->texture && newChar && texturePixel)
	{
		Texture_putChar(this->texture, texturePixel, newChar);
	}
}

// write directly to texture
void Sprite_putPixel(Sprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "Sprite::putPixel: null this");

	if(this->texture)
	{
		Texture_putPixel(this->texture, texturePixel, charSetPixel, newPixelColor);
	}
}

/*
 * Affine FX
 */

void Sprite_noAFX(Sprite this, int direction)
{
	ASSERT(this, "Sprite::noAFX: null this");
}

static void Sprite_doScale(Sprite this)
{
	ASSERT(this, "Sprite::scale: null this");
	ASSERT(this->texture, "Sprite::scale: null texture");

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
void Sprite_scale(Sprite this)
{
	ASSERT(this, "Sprite::scale: null this");
	ASSERT(this->texture, "Sprite::scale: null texture");

	if (this->param)
	{
		this->paramTableRow = 0;
		
		Sprite_doScale(this);
	}
}

void Sprite_rotate(Sprite this, int angle)
{
	ASSERT(this, "Sprite::rotate: null this");
	ASSERT(this->texture, "Sprite::rotate: null texture");

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
		Sprite_setUpdateParamTableFlag(this, false);

	}
	*/
	//delay(5);

/*
 * H-Bias FX
 */

void Sprite_squeezeXHFX(Sprite this)
{
	ASSERT(this, "Sprite::squezeXHFX: null this");

	// TODO
}

void Sprite_fireHFX(Sprite this)
{
	ASSERT(this, "Sprite::fireHFX: null this");

	// TODO
}

void Sprite_waveHFX(Sprite this){
	ASSERT(this, "Sprite::waveHFX: null this");

	// TODO
}