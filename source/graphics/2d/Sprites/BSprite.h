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

#ifndef BSPRITE_H_
#define BSPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define BSprite_METHODS															\
	Sprite_METHODS																\

// declare the virtual methods which are redefined
#define BSprite_SET_VTABLE(ClassName)											\
	Sprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, BSprite, render);									\
	__VIRTUAL_SET(ClassName, BSprite, getPosition);								\
	__VIRTUAL_SET(ClassName, BSprite, setPosition);								\
	__VIRTUAL_SET(ClassName, BSprite, synchronizePosition);						\
	__VIRTUAL_SET(ClassName, BSprite, getScale);								\
	__VIRTUAL_SET(ClassName, BSprite, setDirection);							\
	__VIRTUAL_SET(ClassName, BSprite, scale);									\
	__VIRTUAL_SET(ClassName, BSprite, resize);									\
	__VIRTUAL_SET(ClassName, BSprite, calculateParallax);						\

#define BSprite_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\
																				\
	/* 3d world position */														\
	DrawSpec drawSpec;															\
																				\
	/* param table offset */													\
	u32 param;																	\
																				\
	/* param table offset */													\
	fix19_13 paramTableRow;														\
																				\
	/* h-bias max amplitude */													\
	/* int hbiasAmplitude; */													\


// declare a BSprite, which holds a texture and a drawing specification
__CLASS(BSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct BSpriteDefinition
{
	// the class type
	void* allocator;

	// texture to use with the sprite
	TextureDefinition* textureDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// flag to indicate in which display to show the bg texture
	u16 display;

	// parallax modifier to achieve better control over display
	s8 parallaxDisplacement;

} BSpriteDefinition;

typedef const BSpriteDefinition BSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BSprite, const BSpriteDefinition* bSpriteDefinition);

void BSprite_constructor(BSprite this, const BSpriteDefinition* bSpriteDefinition);
void BSprite_destructor(BSprite this);
Scale BSprite_getScale(BSprite this);
void BSprite_setDirection(BSprite this, int axis, int direction);
void BSprite_resize(BSprite this, fix19_13 z);
VBVec2D BSprite_getPosition(BSprite this);
void BSprite_setPosition(BSprite this, VBVec2D position);
void BSprite_synchronizePosition(BSprite this, VBVec3D position3D);
void BSprite_calculateParallax(BSprite this, fix19_13 z);
DrawSpec BSprite_getDrawSpec(BSprite this);
void BSprite_invalidateParamTable(BSprite this);
void BSprite_setDrawSpec(BSprite this, const DrawSpec* const drawSpec);
fix19_13 BSprite_getParamTableRow(BSprite this);
u32 BSprite_getParam(BSprite this);
void BSprite_setParam(BSprite this, u32 param);
void BSprite_render(BSprite this);


//---------------------------------------------------------------------------------------------------------
// 										BSprites FXs
//---------------------------------------------------------------------------------------------------------

// direct draw
void BSprite_putChar(BSprite this, Point* texturePixel, BYTE* newChar);
void BSprite_putPixel(BSprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);

// Affine FX
void BSprite_scale(BSprite this);
void BSprite_rotate(BSprite this, int angle);

// H-Bias FX
void BSprite_squeezeXHFX(BSprite this);
void BSprite_fireHFX(BSprite this);
void BSprite_waveHFX(BSprite this);


#endif