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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Texture.h>
#include <VisualComponent.h>
#include <VIPManager.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class AnimationController;


//=========================================================================================================
// CLASS'S DATA
//=========================================================================================================

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
	uint16 numberOfFrames;

	/// frames to play in animation
	uint16 frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

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


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class Sprite
///
/// Inherits from VisualComponent
///
/// Displays a Texture to the displays.
/// @ingroup graphics-2d-sprites
abstract class Sprite : VisualComponent
{
	// The unusual order of the attributes is to optimize data packing as much as possible
	// Flag to check if rendered even if outside the screen
	bool checkIfWithinScreenSpace;
	// 2D position
	PixelVector position;
	// Displacement modifier to achieve better control over display
	PixelVector displacement;
	// Rotation cache
	Rotation rotation;
	// World layer where to render the texture
	int16 index;
	// Scale cache
	PixelScale scale;
	// Head spec for world entry setup
	uint16 head;
	// Texture's half width
	int16 halfWidth;
	// Texture's half height
	int16 halfHeight;
	// Animation Controller
	AnimationController animationController;
	// Our texture
	Texture texture;
	// Update animation flag
	bool updateAnimationFrame;
	// The flag raises after the first render cycle
	bool transformed;

	/// @publicsection
	void constructor(SpatialObject owner, const SpriteSpec* spriteSpec);
	void createAnimationController();
	uint16 getHead();
	uint16 getMode();
	Texture getTexture();
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
	bool isPlayingAnimation(char* animationName);
	const char* getPlayingAnimationName();
	void nextFrame();
	void pause(bool pause);
	bool play(const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope);
	void stop();
	bool replay(const AnimationFunction* animationFunctions[]);
	void previousFrame();
	void setActualFrame(int16 actualFrame);
	void setFrameDurationDecrement(uint8 frameDurationDecrement);
	void setFrameDuration(uint8 frameDuration);
	void update();

	/// Add the color provided color data to a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR 
	void addChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a CHAR in the sprite's texture.
	/// @param texturePoint: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR 
	void putChar(const Point* texturePoint, const uint32* newChar);

	/// Replace a pixel in the sprite's texture.
	/// @param texturePixel: Coordinate in texture's space of the CHAR to replace
	/// @param newChar: Color data array for the CHAR 
	void putPixel(const Point* texturePixel, const Pixel* charSetPixel, BYTE newPixelColor);

	AnimationController getAnimationController();
	bool isVisible();
	bool isWithinScreenSpace();
	bool isDisposed();
	int16 render(int16 index, bool updateAnimation);
	void calculateParallax(fixed_t z);
	int16 getIndex();
	PixelVector getDisplacedPosition();
	void setPosition(const PixelVector* position);
	const PixelVector* getPosition();
	virtual void setRotation(const Rotation* rotation);
	virtual void setScale(const PixelScale* scale);
	virtual void registerWithManager() = 0;
	virtual void unregisterWithManager() = 0;
	virtual void hideForDebug();
	virtual void forceShow();
	virtual void processEffects();

	/// Set the current multiframe
	/// @param frame: Current animation frame 
	virtual void setMultiframe(uint16 frame);
	virtual int16 doRender(int16 index) = 0;
	virtual void updateAnimation();
	virtual void print(int32 x, int32 y);
	virtual int32 getTotalPixels() = 0;
	virtual void invalidateRendering();
}

#endif
