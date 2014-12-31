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

#ifndef SPRITE_H_
#define SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
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
#define Sprite_METHODS															\
	Object_METHODS																\
	__VIRTUAL_DEC(render);														\
	__VIRTUAL_DEC(setPosition);													\

// declare the virtual methods which are redefined
#define Sprite_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Sprite, render);								\
		__VIRTUAL_SET(ClassName, Sprite, setPosition);							\

#define Sprite_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* this is our texture */													\
	Texture texture;															\
																				\
	/* 3d world position */														\
	DrawSpec drawSpec;															\
																				\
	/* location of texture in graphic memory */									\
	Point texturePosition;														\
																				\
	/* texture's half width */													\
	fix19_13 halfWidth;															\
																				\
	/* texture's half height */													\
	fix19_13 halfHeight;														\
																				\
	/* param table offset */													\
	u32 param;																	\
																				\
	/* head definition for world entry setup */									\
	u16 head;																	\
																				\
	/* world layer where to render the texture */								\
	u8 worldLayer;																\
																				\
	/* h-bias max amplitude */													\
	/* int hbiasAmplitude; */													\
																				\
	bool renderFlag;															\
																				\
	/* parallax modifier to achieve better control over display */				\
	s8 parallaxDisplacement;													\


// declare a Sprite, which holds a texture and a drawing specification
__CLASS(Sprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct SpriteDefinition
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

} SpriteDefinition;

typedef const SpriteDefinition SpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Sprite, __PARAMETERS(const SpriteDefinition* spriteDefinition));

void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition);
void Sprite_destructor(Sprite this);
Scale Sprite_getScale(Sprite this);
void Sprite_scale(Sprite this);
void Sprite_setDirection(Sprite this, int axis, int direction);
void Sprite_calculateScale(Sprite this, fix19_13 z);
void Sprite_setPosition(Sprite this, const VBVec3D* const position);
void Sprite_calculateParallax(Sprite this, fix19_13 z);
Texture Sprite_getTexture(Sprite this);
DrawSpec Sprite_getDrawSpec(Sprite this);
u16 Sprite_getMode(Sprite this);
void Sprite_invalidateParamTable(Sprite this);
void Sprite_resetMemoryState(Sprite this);
void Sprite_setDrawSpec(Sprite this, const DrawSpec* const drawSpec);
u32 Sprite_getParam(Sprite this);
void Sprite_setParam(Sprite this, u32 param);
void Sprite_setWorldLayer(Sprite this, u8 worldLayer);
u8 Sprite_getWorldLayer(Sprite this);
u16 Sprite_getHead(Sprite this);
void Sprite_setRenderFlag(Sprite this, bool renderFlag);
void Sprite_show(Sprite this);
void Sprite_hide(Sprite this);
void Sprite_update(Sprite this);
void Sprite_render(Sprite this);


//---------------------------------------------------------------------------------------------------------
// 										Sprites FXs
//---------------------------------------------------------------------------------------------------------

// direct draw
void Sprite_putChar(Sprite this, u8 x, u8 y, BYTE* newChar);

// Affine FX
void Sprite_scale(Sprite this);
void Sprite_rotate(Sprite this, int angle);

// H-Bias FX
void Sprite_squeezeXHFX(Sprite this);
void Sprite_fireHFX(Sprite this);
void Sprite_waveHFX(Sprite this);


#endif