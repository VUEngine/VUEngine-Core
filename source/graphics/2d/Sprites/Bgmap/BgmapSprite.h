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

#ifndef BGMAP_SPRITE_H_
#define BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <AnimationController.h>
#include <MiscStructs.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08

#define __G_DISPLACEMENT_BECAUSE_WH_0_EQUALS_1		1
#define __WORLD_SIZE_DISPLACEMENT					1
#define __GX_LIMIT									511
#define __GY_LIMIT									223


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define BgmapSprite_METHODS(ClassName)																	\
		Sprite_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define BgmapSprite_SET_VTABLE(ClassName)																\
		Sprite_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, BgmapSprite, render);													\
		__VIRTUAL_SET(ClassName, BgmapSprite, getPosition);												\
		__VIRTUAL_SET(ClassName, BgmapSprite, setPosition);												\
		__VIRTUAL_SET(ClassName, BgmapSprite, position);												\
		__VIRTUAL_SET(ClassName, BgmapSprite, rotate);													\
		__VIRTUAL_SET(ClassName, BgmapSprite, getScale);												\
		__VIRTUAL_SET(ClassName, BgmapSprite, applyAffineTransformations);								\
		__VIRTUAL_SET(ClassName, BgmapSprite, applyHbiasEffects);										\
		__VIRTUAL_SET(ClassName, BgmapSprite, resize);													\
		__VIRTUAL_SET(ClassName, BgmapSprite, calculateParallax);										\
		__VIRTUAL_SET(ClassName, BgmapSprite, addDisplacement);											\
		__VIRTUAL_SET(ClassName, BgmapSprite, setMode);													\


#define BgmapSprite_ATTRIBUTES																			\
		Sprite_ATTRIBUTES																				\
		/**
		 * @var DrawSpec 				drawSpec
		 * @brief						3d world position
		 * @memberof					BgmapSprite
		 */																								\
		DrawSpec drawSpec;																				\
		/**
		 * @var u32 					param
		 * @brief						param table offset
		 * @memberof					BgmapSprite
		 */																								\
		u32 param;																						\
		/**
		 * @var s16 					paramTableRow
		 * @brief						param table offset
		 * @memberof					BgmapSprite
		 */																								\
		s16 paramTableRow;																				\
		/**
		 * @var void(*)(BgmapSprite) 	paramTableEffect
		 * @brief						pointer to function that implements the param table based effects
		 * @memberof					BgmapSprite
		 */																								\
		s16 (*applyParamTableEffect)(BgmapSprite);

// declare a BgmapSprite, which holds a texture and a drawing specification
__CLASS(BgmapSprite);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct BgmapSpriteDefinition
{
	// it has a Sprite definition at the beginning
	SpriteDefinition spriteDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// pointer to affine/hbias manipulation function
	s16 (*applyParamTableEffect)(BgmapSprite);

	// flag to indicate in which display to show the bg texture
	u16 display;

} BgmapSpriteDefinition;

typedef const BgmapSpriteDefinition BgmapSpriteROMDef;

typedef s16 (*ParamTableEffectMethod)(BgmapSprite);

//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapSprite, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner);

void BgmapSprite_constructor(BgmapSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner);
void BgmapSprite_destructor(BgmapSprite this);

// general
Scale BgmapSprite_getScale(BgmapSprite this);
void BgmapSprite_resize(BgmapSprite this, Scale scale, fix10_6 z);
PixelVector BgmapSprite_getPosition(BgmapSprite this);
void BgmapSprite_setPosition(BgmapSprite this, const PixelVector* position);
void BgmapSprite_position(BgmapSprite this, const Vector3D* position);
void BgmapSprite_rotate(BgmapSprite this, const Rotation* rotation);
void BgmapSprite_calculateParallax(BgmapSprite this, fix10_6 z);
DrawSpec BgmapSprite_getDrawSpec(BgmapSprite this);
void BgmapSprite_invalidateParamTable(BgmapSprite this);
void BgmapSprite_setDrawSpec(BgmapSprite this, const DrawSpec* const drawSpec);
s16 BgmapSprite_getParamTableRow(BgmapSprite this);
u32 BgmapSprite_getParam(BgmapSprite this);
void BgmapSprite_setParam(BgmapSprite this, u32 param);
void BgmapSprite_render(BgmapSprite this);
void BgmapSprite_addDisplacement(BgmapSprite this, const PixelVector* displacement);
void BgmapSprite_setMode(BgmapSprite this, u16 display, u16 mode);

// direct draw
void BgmapSprite_putChar(BgmapSprite this, Point* texturePixel, BYTE* newChar);
void BgmapSprite_putPixel(BgmapSprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);

// affine & hbias fx
void BgmapSprite_applyAffineTransformations(BgmapSprite this);
void BgmapSprite_applyHbiasEffects(BgmapSprite this);
void BgmapSprite_processAffineEffects(BgmapSprite this, int gx, int width, int myDisplacement);
void BgmapSprite_processHbiasEffects(BgmapSprite this);


#endif
