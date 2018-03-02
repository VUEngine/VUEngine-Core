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

#ifndef SPRITE_H_
#define SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VIPManager.h>
#include <MiscStructs.h>
#include <Texture.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08

#define __TRANSPARENCY_NONE	0
#define __TRANSPARENCY_EVEN	1
#define __TRANSPARENCY_ODD	2


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Sprite_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, addDisplacement, const PixelVector* displacement);				\
		__VIRTUAL_DEC(ClassName, void, applyAffineTransformations);										\
		__VIRTUAL_DEC(ClassName, void, applyHbiasEffects);												\
		__VIRTUAL_DEC(ClassName, bool, areTexturesWritten);												\
		__VIRTUAL_DEC(ClassName, void, calculateParallax, fix10_6 z);									\
		__VIRTUAL_DEC(ClassName, PixelVector, getPosition);												\
		__VIRTUAL_DEC(ClassName, Scale, getScale);														\
		__VIRTUAL_DEC(ClassName, u8, getWorldLayer);													\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, void, position, const Vector3D* position);								\
		__VIRTUAL_DEC(ClassName, void, render);															\
		__VIRTUAL_DEC(ClassName, void, resize, Scale scale, fix10_6 z);									\
		__VIRTUAL_DEC(ClassName, void, rotate, const Rotation* rotation);								\
		__VIRTUAL_DEC(ClassName, void, setMode, u16 display, u16 mode);									\
		__VIRTUAL_DEC(ClassName, void, setPosition, const PixelVector* position);						\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, writeAnimation);													\
		__VIRTUAL_DEC(ClassName, bool, writeTextures);													\

// declare the virtual methods which are redefined
#define Sprite_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Sprite, applyAffineTransformations);									\
		__VIRTUAL_SET(ClassName, Sprite, applyHbiasEffects);											\
		__VIRTUAL_SET(ClassName, Sprite, areTexturesWritten);											\
		__VIRTUAL_SET(ClassName, Sprite, calculateParallax);											\
		__VIRTUAL_SET(ClassName, Sprite, getScale);														\
		__VIRTUAL_SET(ClassName, Sprite, getWorldLayer);												\
		__VIRTUAL_SET(ClassName, Sprite, hide);															\
		__VIRTUAL_SET(ClassName, Sprite, position);														\
		__VIRTUAL_SET(ClassName, Sprite, resize);														\
		__VIRTUAL_SET(ClassName, Sprite, rotate);														\
		__VIRTUAL_SET(ClassName, Sprite, setPosition);													\
		__VIRTUAL_SET(ClassName, Sprite, show);															\
		__VIRTUAL_SET(ClassName, Sprite, writeAnimation);												\
		__VIRTUAL_SET(ClassName, Sprite, writeTextures);												\

#define Sprite_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/*
		 * @var PixelVector 			displacement
		 * @brief					Displacement modifier to achieve better control over display
		 * @memberof				Sprite
		 */																								\
		PixelVector displacement;																		\
		/*
		 * @var AnimationController animationController
		 * @brief					AnimationController
		 * @memberof				Sprite
		 */																								\
		AnimationController animationController;														\
		/*
		 * @var Texture 			texture
		 * @brief					Our texture
		 * @memberof				Sprite
		 */																								\
		Texture texture;																				\
		/*
		 * @var s16 				halfWidth
		 * @brief					Texture's half width
		 * @memberof				Sprite
		 */																								\
		s16 halfWidth;																					\
		/*
		 * @var s16		 			halfHeight
		 * @brief					Texture's half height
		 * @memberof				Sprite
		 */																								\
		s16 halfHeight;																					\
		/*
		 * @var u16 				head
		 * @brief					Head definition for world entry setup
		 * @memberof				Sprite
		 */																								\
		u16 head;																						\
		/*
		 * @var bool 				hidden
		  * @brief
		  * @memberof				Sprite
		  */																							\
		bool hidden;																					\
		/*
		 * @var bool 				writeAnimationFrame
		 * @brief					Update animation
		 * @memberof				Sprite
		 */																								\
		bool writeAnimationFrame : 2;																	\
		/*
		 * @var bool 				visible
		 * @brief					Flag for transparency control
		 * @memberof				Sprite
		 */																								\
		bool visible : 2;																				\
		/*
		 * @var bool 				ready
		 * @brief					Flag to allow rendering
		 * @memberof				Sprite
		 */																								\
		bool ready : 2;																					\
		/*
		 * @var u8 					worldLayer
		 * @brief					World layer where to render the texture
		 * @memberof				Sprite
		 */																								\
		u8 worldLayer;																					\
		/*
		 * @var bool 				transparent
		 * @brief					Flag for making it transparent
		 * @memberof				Sprite
		 */																								\
		u8 transparent;																					\

__CLASS(Sprite);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * A SpriteDefinition
 *
 * @memberof	Sprite
 */
typedef struct SpriteDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	/// texture to use with the sprite
	TextureDefinition* textureDefinition;

	/// transparency mode
	u8 transparent;

	/// displacement modifier to achieve better control over display
	PixelVector displacement;

} SpriteDefinition;

/**
 * A SpriteDefinition that is stored in ROM
 *
 * @memberof	Sprite
 */
typedef const SpriteDefinition SpriteROMDef;


/**
 * A function which defines the frames to play
 *
 * @memberof	Sprite
 */
typedef struct AnimationFunction
{
	/// number of frames of this animation function
	int numberOfFrames;

	/// frames to play in animation
	u32 frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	/// number of cycles a frame of animation is displayed
	int delay;

	/// whether to play it in loop or not
	int loop;

	/// method to call on function completion
	EventListener onAnimationComplete;

	/// function's name
	char name[__MAX_ANIMATION_FUNCTION_NAME_LENGTH];

} AnimationFunction;

/**
 * An AnimationFunction that is stored in ROM
 *
 * @memberof	Sprite
 */
typedef const AnimationFunction AnimationFunctionROMDef;

/**
 * An animation definition
 *
 * @memberof	Sprite
 */
typedef struct AnimationDescription
{
	/// animation functions
	AnimationFunction* animationFunctions[__MAX_ANIMATION_FUNCTIONS];

} AnimationDescription;

/**
 * An AnimationDescription that is stored in ROM
 *
 * @memberof	Sprite
 */
typedef const AnimationDescription AnimationDescriptionROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition, Object owner);
void Sprite_destructor(Sprite this);

// general
bool Sprite_areTexturesWritten(Sprite this);
void Sprite_calculateParallax(Sprite this, fix10_6 z);
u16 Sprite_getHead(Sprite this);
u16 Sprite_getMode(Sprite this);
Scale Sprite_getScale(Sprite this);
Texture Sprite_getTexture(Sprite this);
u8 Sprite_getTransparent(Sprite this);
u32 Sprite_getWorldHead(Sprite this);
u16 Sprite_getWorldHeight(Sprite this);
u8 Sprite_getWorldLayer(Sprite this);
u16 Sprite_getWorldWidth(Sprite this);
s16 Sprite_getWorldGP(Sprite this);
s16 Sprite_getWorldGX(Sprite this);
s16 Sprite_getWorldGY(Sprite this);
s16 Sprite_getWorldMP(Sprite this);
s16 Sprite_getWorldMX(Sprite this);
s16 Sprite_getWorldMY(Sprite this);
void Sprite_hide(Sprite this);
bool Sprite_isHidden(Sprite this);
void Sprite_position(Sprite this, const Vector3D* position);
void Sprite_resize(Sprite this, Scale scale, fix10_6 z);
void Sprite_rewrite(Sprite this);
void Sprite_setPosition(Sprite this, const PixelVector* position);
void Sprite_setTransparent(Sprite this, u8 value);
void Sprite_setWorldLayer(Sprite this, u8 worldLayer);
void Sprite_show(Sprite this);
bool Sprite_writeTextures(Sprite this);

// animation
s8 Sprite_getActualFrame(Sprite this);
PixelVector Sprite_getDisplacement(Sprite this);
s8 Sprite_getFrameDuration(Sprite this);
int Sprite_getHalfHeight(Sprite this);
int Sprite_getHalfWidth(Sprite this);
bool Sprite_isAffine(Sprite this);
bool Sprite_isHBias(Sprite this);
bool Sprite_isObject(Sprite this);
bool Sprite_isPlaying(Sprite this);
bool Sprite_isPlayingFunction(Sprite this, char* functionName);
void Sprite_nextFrame(Sprite this);
void Sprite_pause(Sprite this, bool pause);
void Sprite_play(Sprite this, AnimationDescription* animationDescription, char* functionName);
void Sprite_previousFrame(Sprite this);
void Sprite_print(Sprite this, int x, int y);
void Sprite_rotate(Sprite this, const Rotation* rotation);
void Sprite_setActualFrame(Sprite this, s8 actualFrame);
void Sprite_setFrameCycleDecrement(Sprite this, u8 frameDelayDelta);
void Sprite_setFrameDuration(Sprite this, u8 frameDuration);
void Sprite_update(Sprite this, bool evenFrame);
void Sprite_updateAnimation(Sprite this);
void Sprite_writeAnimation(Sprite this);

// direct draw
void Sprite_putChar(Sprite this, Point* texturePixel, BYTE* newChar);
void Sprite_putPixel(Sprite this, Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);

// affine & hbias fx
void Sprite_applyAffineTransformations(Sprite this);
void Sprite_applyHbiasEffects(Sprite this);


#endif
