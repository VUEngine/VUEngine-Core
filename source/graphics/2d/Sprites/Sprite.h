/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SPRITE_H_
#define SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
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
abstract class Sprite : ListenerObject
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
	ListenerObject owner;
	// Head spec for world entry setup
	uint16 head;
	// Texture's half width
	int16 halfWidth;
	// Texture's half height
	int16 halfHeight;
	// World layer where to render the texture
	int16 index;
	// show flag
	bool show;
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
	// Flag to check if rendered even if outside the screen
	bool checkIfWithinScreenSpace;
	// Flag to avoid rewriting DRAM's cache if not needed
	uint8 renderFlag;

	/// @publicsection
	void constructor(const SpriteSpec* spriteSpec, ListenerObject owner);
	const PixelVector* getPosition();
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
	bool play(const AnimationDescription* animationDescription, const char* functionName, ListenerObject scope);
	void stop();
	bool replay(const AnimationDescription* animationDescription);
	void previousFrame();
	void setActualFrame(int16 actualFrame);
	void setFrameCycleDecrement(uint8 frameDelayDelta);
	void setFrameDuration(uint8 frameDuration);
	void update();
	bool updateAnimation();
	void putChar(Point* texturePixel, uint32* newChar);
	void putPixel(Point* texturePixel, Pixel* charSetPixel, BYTE newPixelColor);
	AnimationController getAnimationController();
	bool isVisible();
	bool isWithinScreenSpace();
	bool isDisposed();
	int16 render(int16 index, bool evenFrame);
	void calculateParallax(fixed_t z);
	void hide();
	void show();
	int16 getIndex();
	PixelVector getDisplacedPosition();
	void position(const Vector3D* position);
	virtual void setPosition(const PixelVector* position);
	virtual void rewrite();
	virtual void hideForDebug();
	virtual void forceShow();
	virtual Scale getScale();
	virtual void processEffects();
	virtual int16 doRender(int16 index, bool evenFrame) = 0;
	virtual void resize(Scale scale, fixed_t z);
	virtual void rotate(const Rotation* rotation);
	virtual void setMode(uint16 display, uint16 mode) = 0;
	virtual void writeAnimation();
	virtual bool writeTextures();
	virtual bool prepareTexture();
	virtual void print(int32 x, int32 y);
	virtual int32 getTotalPixels();
	virtual void registerWithManager() = 0;
}


#endif
