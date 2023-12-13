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
#include <Texture.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------



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
	uint8 numberOfFrames;

	/// frames to play in animation
	uint8 frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	/// number of cycles a frame of animation is displayed
	uint8 delay;

	/// whether to play it in loop or not
	bool loop;

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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites
abstract class Sprite : ListenerObject
{
	// Flag to signal if the sprite has been already registered
	bool registered;
	// show flag
	bool show;
	// Flag to allow rendering
	bool positioned;
	// Projected position based on optics configuration
	PixelVector position;
	// Displacement modifier to achieve better control over display
	PixelVector displacement;
	// Animation Controller
	AnimationController animationController;
	// Our texture
	Texture texture;
	// Head spec for world entry setup
	uint16 head;
	// Texture's half width
	int16 halfWidth;
	// Texture's half height
	int16 halfHeight;
	// World layer where to render the texture
	int16 index;
	// Flag for making it transparent
	uint8 transparent;
	// Update animation
	bool writeAnimationFrame;
	// Flag to check if rendered even if outside the screen
	bool checkIfWithinScreenSpace;
	// Flag to avoid rewriting DRAM's cache if not needed (helps a lot in menus)
 	bool renderFlag;

	/// @publicsection
	void constructor(const SpriteSpec* spriteSpec, ListenerObject owner);
	void createAnimationController(CharSetSpec* charSetSpec, ListenerObject owner);
	const PixelVector* getPosition();
	uint16 getHead();
	uint16 getMode();
	Texture getTexture();
	uint8 getTransparent();
	uint32 getEffectiveHead();
	uint16 getEffectiveHeight();
	uint16 getEffectiveWidth();
	int16 getEffectiveP();
	int16 getEffectiveX();
	int16 getEffectiveY();
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
	bool isBgmap();
	bool isAffine();
	bool isHBias();
	bool isObject();
	bool isPlaying();
	bool isPlayingFunction(char* functionName);
	const char* getPlayingAnimationName();
	void nextFrame();
	void pause(bool pause);
	bool play(const AnimationFunction* animationFunctions[], const char* functionName, ListenerObject scope);
	void stop();
	bool replay(const AnimationFunction* animationFunctions[]);
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
	void setPosition(const PixelVector* position);
	virtual void rewrite();
	virtual void hideForDebug();
	virtual void forceShow();
	virtual Scale getScale();
	virtual void processEffects();
	virtual int16 doRender(int16 index, bool evenFrame) = 0;
	virtual void resize(Scale scale, fixed_t z);
	virtual void rotate(const Rotation* rotation);
	virtual void writeAnimation();
	virtual bool writeTextures(int16 maximumTextureRowsToWrite);
	virtual void print(int32 x, int32 y);
	virtual int32 getTotalPixels() = 0;
	virtual void registerWithManager() = 0;
	virtual void invalidateRenderFlag();
}

#endif
