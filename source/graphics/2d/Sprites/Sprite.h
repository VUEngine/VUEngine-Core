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

class AnimationController;

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

abstract class Sprite : Object
{
	/*
	* @var PixelVector 		Position
	* @brief					Projected position based on optics configuration
	* @memberof				Sprite
	*/
	PixelVector position;
	/*
	* @var PixelVector 		Displacement
	* @brief					Displacement modifier to achieve better control over display
	* @memberof				Sprite
	*/
	PixelVector displacement;
	/*
	* @var AnimationController animationController
	* @brief					AnimationController
	* @memberof				Sprite
	*/
	AnimationController animationController;
	/*
	* @var Texture 			texture
	* @brief					Our texture
	* @memberof				Sprite
	*/
	Texture texture;
	/*
	* @var s16 				halfWidth
	* @brief					Texture's half width
	* @memberof				Sprite
	*/
	s16 halfWidth;
	/*
	* @var s16		 			halfHeight
	* @brief					Texture's half height
	* @memberof				Sprite
	*/
	s16 halfHeight;
	/*
	* @var u16 				head
	* @brief					Head definition for world entry setup
	* @memberof				Sprite
	*/
	u16 head;
	/*
	* @var bool 				hidden
	* @brief
	* @memberof				Sprite
	*/
	bool hidden;
	/*
	* @var bool 				writeAnimationFrame
	* @brief					Update animation
	* @memberof				Sprite
	*/
	bool writeAnimationFrame : 2;
	/*
	* @var bool 				visible
	* @brief					Flag for transparency control
	* @memberof				Sprite
	*/
	bool visible : 2;
	/*
	* @var bool 				positioned
	* @brief					Flag to allow rendering
	* @memberof				Sprite
	*/
	bool positioned : 2;
	/*
	* @var u8 					worldLayer
	* @brief					World layer where to render the texture
	* @memberof				Sprite
	*/
	u8 worldLayer;
	/*
	* @var bool 				transparent
	* @brief					Flag for making it transparent
	* @memberof				Sprite
	*/
	u8 transparent;

	void constructor(const SpriteDefinition* spriteDefinition, Object owner);
	PixelVector getPosition();
	PixelVector getDisplacedPosition();
	u16 getHead();
	u16 getMode();
	Texture getTexture();
	u8 getTransparent();
	u32 getWorldHead();
	u16 getWorldHeight();
	u16 getWorldWidth();
	s16 getWorldGP();
	s16 getWorldGX();
	s16 getWorldGY();
	s16 getWorldMP();
	s16 getWorldMX();
	s16 getWorldMY();
	bool isHidden();
	void rewrite();
	void setTransparent(u8 value);
	void setWorldLayer(u8 worldLayer);
	s8 getActualFrame();
	PixelVector getDisplacement();
	void setDisplacement(PixelVector displacement);
	s8 getFrameDuration();
	int getHalfHeight();
	int getHalfWidth();
	bool isAffine();
	bool isHBias();
	bool isObject();
	bool isPlaying();
	bool isPlayingFunction(char* functionName);
	void nextFrame();
	void pause(bool pause);
	void play(AnimationDescription* animationDescription, char* functionName);
	void previousFrame();
	void print(int x, int y);
	void setActualFrame(s8 actualFrame);
	void setFrameCycleDecrement(u8 frameDelayDelta);
	void setFrameDuration(u8 frameDuration);
	void update();
	void updateAnimation();
	void putChar(Point* texturePixel, BYTE* newChar);
	void putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	void onTextureRewritten(Object eventFirer);
	AnimationController getAnimationController();
	virtual void addDisplacement(const PixelVector* displacement) = 0;
	virtual void applyAffineTransformations();
	virtual void applyHbiasEffects();
	virtual bool areTexturesWritten();
	virtual void calculateParallax(fix10_6 z);
	virtual Scale getScale();
	virtual u8 getWorldLayer();
	virtual void hide();
	virtual void position(const Vector3D* position);
	virtual void render(bool evenFrame);
	virtual void resize(Scale scale, fix10_6 z);
	virtual void rotate(const Rotation* rotation);
	virtual void setMode(u16 display, u16 mode) = 0;
	virtual void setPosition(const PixelVector* position);
	virtual void show();
	virtual void writeAnimation();
	virtual bool writeTextures();
}


#endif
