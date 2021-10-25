/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#define __UPDATE_HEAD				0x0F
#define __UPDATE_G					0x01
#define __UPDATE_PARAM				0x02
#define __UPDATE_SIZE				0x04
#define __UPDATE_M					0x08

#define __TRANSPARENCY_NONE			0
#define __TRANSPARENCY_ODD			1
#define __TRANSPARENCY_EVEN			2

#define __NO_RENDER_INDEX			-1


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class AnimationController;

/**
 * A SpriteSpec
 *
 * @memberof	Sprite
 */
typedef struct SpriteSpec
{
	/// class allocator
	AllocatorPointer allocator;

	/// texture to use with the sprite
	TextureSpec* textureSpec;

	/// transparency mode
	uint8 transparent;

	/// displacement modifier to achieve better control over display
	PixelVector displacement;

} SpriteSpec;

/**
 * A SpriteSpec that is stored in ROM
 *
 * @memberof	Sprite
 */
typedef const SpriteSpec SpriteROMSpec;

/**
 * A function which defines the frames to play
 *
 * @memberof	Sprite
 */
typedef struct AnimationFunction
{
	/// number of frames of this animation function
	int32 numberOfFrames;

	/// frames to play in animation
	uint8 frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	/// number of cycles a frame of animation is displayed
	int32 delay;

	/// whether to play it in loop or not
	int32 loop;

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
typedef const AnimationFunction AnimationFunctionROMSpec;

/**
 * An animation spec
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
typedef const AnimationDescription AnimationDescriptionROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites
abstract class Sprite : Object
{
	// Projected position based on optics configuration
	PixelVector position;
	// Displacement modifier to achieve better control over display
	PixelVector displacement;
	// AnimationController
	AnimationController animationController;
	// Our texture
	Texture texture;
	// Owner
	Object owner;
	// Head spec for world entry setup
	uint16 head;
	// Texture's half width
	int16 halfWidth;
	// Texture's half height
	int16 halfHeight;
	// World layer where to render the texture
	int16 index;
	// Hidden flag
	bool hidden;
	// Update animation
	bool writeAnimationFrame;
	// Flag for transparency control
	bool visible;
	// Flag to allow rendering
	bool positioned;
	// Flato to allow registering
	bool registered;
	// Flag for making it transparent
	uint8 transparent;

	/// @publicsection
	void constructor(const SpriteSpec* spriteSpec, Object owner);
	const PixelVector* getPosition();
	PixelVector getDisplacedPosition();
	uint16 getHead();
	uint16 getMode();
	Texture getTexture();
	uint8 getTransparent();
	uint32 getWorldHead();
	uint16 getWorldHeight();
	uint16 getWorldWidth();
	int16 getWorldGP();
	int16 getWorldGX();
	int16 getWorldGY();
	int16 getWorldMP();
	int16 getWorldMX();
	int16 getWorldMY();
	bool isHidden();
	void rewrite();
	void setTransparent(uint8 value);
	int16 getActualFrame();
	const PixelVector* getDisplacement();
	void setDisplacement(const PixelVector* displacement);
	uint8 getFrameDuration();
	int32 getHalfHeight();
	int32 getHalfWidth();
	bool isAffine();
	bool isHBias();
	bool isObject();
	bool isPlaying();
	bool isPlayingFunction(char* functionName);
	void nextFrame();
	void pause(bool pause);
	bool play(const AnimationDescription* animationDescription, char* functionName, Object scope);
	void stop();
	bool replay(const AnimationDescription* animationDescription);
	void previousFrame();
	void setActualFrame(int16 actualFrame);
	void setFrameCycleDecrement(uint8 frameDelayDelta);
	void setFrameDuration(uint8 frameDuration);
	void update();
	bool updateAnimation();
	void putChar(Point* texturePixel, BYTE* newChar);
	void putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	AnimationController getAnimationController();
	bool isVisible();
	bool isWithinScreenSpace();
	bool isDisposed();
	int16 render(int16 index, bool evenFrame);
	void calculateParallax(fix10_6 z);
	void hide();
	void show();
	uint8 getIndex();
	virtual void hideForDebug();
	virtual void showForDebug();
	virtual void addDisplacement(const PixelVector* displacement);
	virtual Scale getScale();
	virtual void position(const Vector3D* position);
	virtual void processEffects();
	virtual int16 doRender(int16 index, bool evenFrame) = 0;
	virtual void resize(Scale scale, fix10_6 z);
	virtual void rotate(const Rotation* rotation);
	virtual void setMode(uint16 display, uint16 mode) = 0;
	virtual void setPosition(const PixelVector* position);
	virtual void writeAnimation();
	virtual bool writeTextures();
	virtual void print(int32 x, int32 y);
	virtual int32 getTotalPixels();
	virtual void registerWithManager();
}


#endif
