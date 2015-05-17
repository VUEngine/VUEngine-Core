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
#include <Clock.h>


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
	__VIRTUAL_DEC(getPosition);													\
	__VIRTUAL_DEC(setPosition);													\
	__VIRTUAL_DEC(positione);													\
	__VIRTUAL_DEC(resize);														\
	__VIRTUAL_DEC(rotate);														\
	__VIRTUAL_DEC(getScale);													\
	__VIRTUAL_DEC(setDirection);												\
	__VIRTUAL_DEC(applyAffineTransformations);									\
	__VIRTUAL_DEC(applyHbiasTransformations);									\
	__VIRTUAL_DEC(calculateParallax);											\
	__VIRTUAL_DEC(writeAnimation);												\
	__VIRTUAL_DEC(show);														\
	__VIRTUAL_DEC(hide);														\
	__VIRTUAL_DEC(getWorldLayer);												\

// declare the virtual methods which are redefined
#define Sprite_SET_VTABLE(ClassName)											\
	Object_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, Sprite, getScale);									\
	__VIRTUAL_SET(ClassName, Sprite, applyAffineTransformations);				\
	__VIRTUAL_SET(ClassName, Sprite, applyHbiasTransformations);				\
	__VIRTUAL_SET(ClassName, Sprite, resize);									\
	__VIRTUAL_SET(ClassName, Sprite, writeAnimation);							\
	__VIRTUAL_SET(ClassName, Sprite, show);										\
	__VIRTUAL_SET(ClassName, Sprite, hide);										\
	__VIRTUAL_SET(ClassName, Sprite, getWorldLayer);							\
	__VIRTUAL_SET(ClassName, Sprite, rotate);									\

#define Sprite_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* super's attributes */													\
	AnimationController animationController;									\
																				\
	/* this is our texture */													\
	Texture texture;															\
																				\
	/* texture's half width */													\
	fix19_13 halfWidth;															\
																				\
	/* texture's half height */													\
	fix19_13 halfHeight;														\
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

} SpriteDefinition;

typedef const SpriteDefinition SpriteROMDef;


// a function which defines the frames to play
typedef struct AnimationFunction
{
	// number of frames of this animation function
	int numberOfFrames;

	// frames to play in animation
	int frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	// number of cicles a frame of animation is displayed
	int delay;

	// whether to play it in loop or not
	int loop;

	// method to call function completion
	void* onAnimationComplete;

	// function's name
	char name[__MAX_ANIMATION_FUNCTION_NAME_LENGTH];

} AnimationFunction;

typedef const AnimationFunction AnimationFunctionROMDef;

// an animation definition
typedef struct AnimationDescription
{
	// animation functions
	AnimationFunction* animationFunctions[__MAX_ANIMATION_FUNCTIONS];

} AnimationDescription;

typedef const AnimationDescription AnimationDescriptionROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition, Object owner);
void Sprite_destructor(Sprite this);
Scale Sprite_getScale(Sprite this);
void Sprite_resize(Sprite this, Scale scale, fix19_13 z);
Texture Sprite_getTexture(Sprite this);
u16 Sprite_getMode(Sprite this);
void Sprite_rewrite(Sprite this);
u32 Sprite_getRenderFlag(Sprite this);
void Sprite_setWorldLayer(Sprite this, u8 worldLayer);
u8 Sprite_getWorldLayer(Sprite this);
u16 Sprite_getHead(Sprite this);
void Sprite_setRenderFlag(Sprite this, bool renderFlag);
void Sprite_show(Sprite this);
void Sprite_hide(Sprite this);

//---------------------------------------------------------------------------------------------------------
// 										Animation
//---------------------------------------------------------------------------------------------------------

void Sprite_update(Sprite this, Clock clock);
void Sprite_pause(Sprite this, bool pause);
void Sprite_play(Sprite thisa, AnimationDescription* animationDescription, char* functionName);
bool Sprite_isPlaying(Sprite this);
bool Sprite_isPlayingFunction(Sprite this, AnimationDescription* animationDescription, char* functionName);
void Sprite_setFrameDelayDelta(Sprite this, u8 frameDelayDelta);
s8 Sprite_getActualFrame(Sprite this);
void Sprite_setActualFrame(Sprite this, s8 actualFrame);
s8 Sprite_getFrameDelay(Sprite this);
void Sprite_setFrameDelay(Sprite this, u8 frameDelay);
void Sprite_writeAnimation(Sprite this);
u8 Sprite_getParallaxDisplacement(Sprite this);
void Sprite_rotate(Sprite this, const Rotation* rotation);


//---------------------------------------------------------------------------------------------------------
// 										Sprites FXs
//---------------------------------------------------------------------------------------------------------

// direct draw
void Sprite_putChar(Sprite this, Point* texturePixel, BYTE* newChar);
void Sprite_putPixel(Sprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);

// Affine FX
void Sprite_applyAffineTransformations(Sprite this);
void Sprite_applyHbiasTransformations(Sprite this);

#endif