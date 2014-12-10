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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <MiscStructs.h>
#include <Texture.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define Sprite_METHODS															\
	Object_METHODS																\
	__VIRTUAL_DEC(update);														\

// declare the virtual methods which are redefined
#define Sprite_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Sprite, update);								\

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
	/* head definition for world entry setup */									\
	u16 head;																	\
																				\
	/* param table offset */													\
	u32 param;																	\
																				\
	/* world layer where to render the texture */								\
	u8 worldLayer;																\
																				\
	/* raise to update the param table */										\
	u8 updateParamTable;														\
																				\
	/* h-bias max amplitude */													\
	/* int hbiasAmplitude; */													\
	u8 renderFlag;																\
																				\
	/* parallax modifier to achieve better */									\
	/* control over display */													\
	s8 parallaxDisplacement;													\
																				\
	/* location of texture in graphic memory */									\
	Point texturePosition;														\
																				\
	/* texture's half width */													\
	fix19_13 halfWidth;															\
																				\
	/* texture's half height */													\
	fix19_13 halfHeight;															\
								

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(Sprite);
							
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

typedef struct SpriteDefinition{

	// texture to use with the sprite
	TextureDefinition* textureDefinition;
	
	// the display mode ( BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;
	
	// flag to indicate in which display to show the bgtexture
	u16 display;
	
	// parallax modifier to achieve better */			
	s8 parallaxDisplacement;
	
}SpriteDefinition;

typedef const SpriteDefinition SpriteROMDef;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Sprite, __PARAMETERS(const SpriteDefinition* spriteDefinition));

// class's constructor
void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition);

//class's destructor
void Sprite_destructor(Sprite this);

// retrieve sprite's scale
Scale Sprite_getScale(Sprite this);

// set the scale
void Sprite_scale(Sprite this);

// set the direction
void Sprite_setDirection(Sprite this, int axis, int direction);

// calculate zoom scaling factor
void Sprite_calculateScale(Sprite this, fix19_13 z);

// set sprite's position
void Sprite_setPosition(Sprite this, const VBVec3D* const position);

// calculate the parallax
void Sprite_calculateParallax(Sprite this, fix19_13 z);

// retrieve the texture
Texture Sprite_getTexture(Sprite this);

// retrieve drawspec
DrawSpec Sprite_getDrawSpec(Sprite this);

//get map's render mode
u16 Sprite_getMode(Sprite this);

// retrieve param table flag
u8 Sprite_updateParamTable(Sprite this);

// force refresh param table in the next render
void Sprite_invalidateParamTable(Sprite this);

// this reallocate a write the bgmap definition in graphical memory
void Sprite_resetMemoryState(Sprite this);

// set drawspec
void Sprite_setDrawSpec(Sprite this, const DrawSpec* const drawSpec);

// get map's param table address
u32 Sprite_getParam(Sprite this);

// set map's param table address
void Sprite_setParam(Sprite this, u32 param);

// set map's world layer
void Sprite_setWorldLayer(Sprite this, u8 worldLayer);

// get map's world layer
u8 Sprite_getWorldLayer(Sprite this);

// get sprite's render head
u16 Sprite_getHead(Sprite this);

// set to true to allow render
void Sprite_setRenderFlag(Sprite this, u8 renderFlag);

// show
void Sprite_show(Sprite this);

// hide
void Sprite_hide(Sprite this);

// update sprite
void Sprite_update(Sprite this);

// render a world layer with the map's information
void Sprite_render(Sprite this);

// render a world layer with the map's information
void Sprite_renderAll(Sprite this);

// turn off render of the map's layer
void Sprite_renderOut(Sprite this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										MAP FXs
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/*
 * Affine FX  
 */


void Sprite_scale(Sprite this);

void Sprite_rotate(Sprite this, int angle);


/*
 * H-Bias FX
 */

void Sprite_squezeXHFX(Sprite this);

void Sprite_fireHFX(Sprite this);

void Sprite_waveHFX(Sprite this);


#endif /* SPRITE_H_ */
