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

#ifndef BGMAP_SPRITE_H_
#define BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <AnimationController.h>
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
#define BgmapSprite_METHODS																				\
	Sprite_METHODS																						\

// declare the virtual methods which are redefined
#define BgmapSprite_SET_VTABLE(ClassName)																\
	Sprite_SET_VTABLE(ClassName)																		\
	__VIRTUAL_SET(ClassName, BgmapSprite, render);														\
	__VIRTUAL_SET(ClassName, BgmapSprite, getPosition);													\
	__VIRTUAL_SET(ClassName, BgmapSprite, setPosition);													\
	__VIRTUAL_SET(ClassName, BgmapSprite, position);													\
	__VIRTUAL_SET(ClassName, BgmapSprite, rotate);														\
	__VIRTUAL_SET(ClassName, BgmapSprite, getScale);													\
	__VIRTUAL_SET(ClassName, BgmapSprite, setDirection);												\
	__VIRTUAL_SET(ClassName, BgmapSprite, applyAffineTransformations);									\
	__VIRTUAL_SET(ClassName, BgmapSprite, applyHbiasTransformations);									\
	__VIRTUAL_SET(ClassName, BgmapSprite, resize);														\
	__VIRTUAL_SET(ClassName, BgmapSprite, calculateParallax);											\

#define BgmapSprite_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Sprite_ATTRIBUTES;																					\
																										\
	/* 3d world position */																				\
	DrawSpec drawSpec;																					\
																										\
	/* param table offset */																			\
	u32 param;																							\
																										\
	/* param table offset */																			\
	fix19_13 paramTableRow;																				\
																										\
	/* h-bias max amplitude */																			\
	/* int hbiasAmplitude; */																			\

// declare a BgmapSprite, which holds a texture and a drawing specification
__CLASS(BgmapSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct BgmapSpriteDefinition
{
	// the class type
	void* allocator;

	// texture to use with the sprite
	TextureDefinition* textureDefinition;

	// displacement modifier to achieve better control over display
	Displacement displacement;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// flag to indicate in which display to show the bg texture
	u16 display;

} BgmapSpriteDefinition;

typedef const BgmapSpriteDefinition BgmapSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapSprite, const BgmapSpriteDefinition* bSpriteDefinition, Object owner);

void BgmapSprite_constructor(BgmapSprite this, const BgmapSpriteDefinition* bSpriteDefinition, Object owner);
void BgmapSprite_destructor(BgmapSprite this);
Scale BgmapSprite_getScale(BgmapSprite this);
void BgmapSprite_setDirection(BgmapSprite this, int axis, int direction);
void BgmapSprite_resize(BgmapSprite this, Scale scale, fix19_13 z);
const VBVec2D* BgmapSprite_getPosition(BgmapSprite this);
void BgmapSprite_setPosition(BgmapSprite this, const VBVec2D* position);
void BgmapSprite_position(BgmapSprite this, const VBVec3D* position);
void BgmapSprite_rotate(BgmapSprite this, const Rotation* rotation);
void BgmapSprite_calculateParallax(BgmapSprite this, fix19_13 z);
DrawSpec BgmapSprite_getDrawSpec(BgmapSprite this);
void BgmapSprite_invalidateParamTable(BgmapSprite this);
void BgmapSprite_setDrawSpec(BgmapSprite this, const DrawSpec* const drawSpec);
fix19_13 BgmapSprite_getParamTableRow(BgmapSprite this);
u32 BgmapSprite_getParam(BgmapSprite this);
void BgmapSprite_setParam(BgmapSprite this, u32 param);
void BgmapSprite_render(BgmapSprite this);


//---------------------------------------------------------------------------------------------------------
// 										BgmapSprites FXs
//---------------------------------------------------------------------------------------------------------

// direct draw
void BgmapSprite_putChar(BgmapSprite this, Point* texturePixel, BYTE* newChar);
void BgmapSprite_putPixel(BgmapSprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);

// Affine FX
void BgmapSprite_applyAffineTransformations(BgmapSprite this);
void BgmapSprite_applyHbiasTransformations(BgmapSprite this);


#endif