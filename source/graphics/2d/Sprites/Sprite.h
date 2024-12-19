/*
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
// CLASS' DATA
//=========================================================================================================

/// A Sprite Spec
/// @memberof Sprite
typedef struct SpriteSpec
{
	VisualComponentSpec visualComponentSpec;

	/// Spec for the texture to display
	TextureSpec* textureSpec;

	/// Transparency mode
	/// (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	uint8 transparency;

	/// Displacement added to the sprite's position
	PixelVector displacement;

} SpriteSpec;

/// A Sprite spec that is stored in ROM
/// @memberof Sprite
typedef const SpriteSpec SpriteROMSpec;

/// A function which defines the frames to play
/// @memberof Sprite
typedef struct AnimationFunction
{
	/// Number of frames of this animation function
	uint16 numberOfFrames;

	/// Frames to play in animation
	uint16 frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	/// Number of cycles a frame of animation is displayed
	uint8 delay;

	/// Whether to play it in loop or not
	bool loop;

	/// Callback on function completion
	EventListener onAnimationComplete;

	/// Animation's name
	char name[__MAX_ANIMATION_FUNCTION_NAME_LENGTH];

} AnimationFunction;

/// An AnimationFunction that is stored in ROM
/// @memberof	Sprite
typedef const AnimationFunction AnimationFunctionROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Sprite
///
/// Inherits from VisualComponent
///
/// Displays a Texture on the screen.
abstract class Sprite : VisualComponent
{
	/// @protectedsection

	/// Flag to check if rendered even if outside the screen
	bool checkIfWithinScreenSpace;

	/// Position cache
	PixelVector position;

	/// Displacement added to the sprite's position
	PixelVector displacement;

	/// Rotation cache
	Rotation rotation;
	
	/// Index of the block in DRAM that the sprite configures to
	/// display its texture
	int16 index;
	
	/// Scale cache
	PixelScale scale;
	
	/// Head flags for DRAM entries
	uint16 head;
	
	/// Cache of the texture's half width
	int16 halfWidth;
	
	/// Cache of the texture's half height
	int16 halfHeight;
	
	/// Animation controller
	AnimationController animationController;
	
	/// Texture to display
	Texture texture;
	
	/// Flag to allow/prohibit the update of the animation
	bool updateAnimationFrame;
	
	/// Flag to invalidate the spatial properties caches (position, rotation, scale)
	bool transformed;

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the sprite attaches to
	/// @param spriteSpec: Specification that determines how to configure the sprite
	void constructor(SpatialObject owner, const SpriteSpec* spriteSpec);

	/// Retrieve the sprite's bounding box.
	/// @return Bounding box of the mesh
	override RightBox getRightBox();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @param updateAnimation: If false, animations are not updated
	/// @return The index that determines the region of DRAM that this sprite manages
	int16 render(int16 index, bool updateAnimation);

	/// Retrieve the sprite's texture.
	/// @return Texture displayed by the sprite
	Texture getTexture();

	/// Retrieve the index that determines the region of DRAM that this sprite configured
	/// @return The index that determines the region of DRAM that this sprite manages
	int16 getIndex();

	/// Retrieve the head flags for DRAM entries.
	/// @return Head flags for DRAM entries
	uint16 getHead();

	/// Retrieve the sprite's texture's half weight.
	/// @return Sprite's texture's half weight
	int32 getHalfWidth();

	/// Retrieve the sprite's texture's half height.
	/// @return Sprite's texture's half height
	int32 getHalfHeight();

	/// Retrieve the head flags written in the DRAM entries determined by index.
	/// @return Head flags written to DRAM entries
	uint32 getEffectiveHead();

	/// Retrieve the weight written in the DRAM entries determined by index.
	/// @return Weight written to DRAM entries
	uint16 getEffectiveWidth();

	/// Retrieve the height written in the DRAM entries determined by index.
	/// @return Height written to DRAM entries
	uint16 getEffectiveHeight();

	/// Retrieve the X coordinate written in the DRAM entries determined by index.
	/// @return X coordinate written to DRAM entries
	int16 getEffectiveX();

	/// Retrieve the Y coordinate written in the DRAM entries determined by index.
	/// @return Y coordinate written to DRAM entries
	int16 getEffectiveY();

	/// Retrieve the P value written in the DRAM entries determined by index.
	/// @return P value written to DRAM entries
	int16 getEffectiveP();

	/// Retrieve the MX coordinate written in the DRAM entries determined by index.
	/// @return MX coordinate written to DRAM entries
	int16 getEffectiveMX();

	/// Retrieve the MY coordinate written in the DRAM entries determined by index.
	/// @return MY coordinate written to DRAM entries
	int16 getEffectiveMY();

	/// Retrieve the MP value written in the DRAM entries determined by index.
	/// @return MP value written to DRAM entries
	int16 getEffectiveMP();

	/// Check if the sprite is visible.
	/// @return True if the sprite is visible; false otherwise
	bool isVisible();

	/// Check if the sprite is hidden.
	/// @return True if the sprite is hidden; false otherwise
	bool isHidden();

	/// Check if the sprite displays a texture in BGMAP mode.
	/// @return True if the sprite displays a texture in BGMAP mode; false otherwise
	bool isBgmap();

	/// Check if the sprite displays a texture in OBJECT mode.
	/// @return True if the sprite displays a texture in OBJECT mode; false otherwise
	bool isObject();

	/// Check if the sprite displays a texture in AFFINE mode.
	/// @return True if the sprite displays a texture in AFFINE mode; false otherwise
	bool isAffine();

	/// Check if the sprite displays a texture in HBIAS mode.
	/// @return True if the sprite displays a texture in HBIAS mode; false otherwise
	bool isHBias();
	
	/// Create an animation controller for this sprite.
	void createAnimationController();

	/// Retrieve the sprite's animation controller.
	/// @return sprite's animation controller
	AnimationController getAnimationController();

	/// Play the animation with the provided name from the provided array of animation functions.
	/// @param animationFunctions: Array of animation functions to look for the animation function to replay
	/// @param animationName: Name of the animation to play
	/// @param scope: Object that will be notified of playback events
	/// @return True if the animation started playing; false otherwise
	bool play(const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope);

	/// Replay the last playing animation, if any, from the provided array of animation functions.
	/// @param animationFunctions: Array of animation functions to look for the animation function to replay
	/// @return True if the animation started playing again; false otherwise
	bool replay(const AnimationFunction* animationFunctions[]);

	/// Pause or unpause the currently playing animation if any.
	/// @param pause: Flag that signals if the animation must be paused or unpaused
	void pause(bool pause);

	/// Stop any playing animation if any.
	void stop();

	/// Check if an animation is playing.
	/// @return True if an animation is playing; false otherwise
	bool isPlaying();

	/// Check if the animation whose name is provided is playing.
	/// @param animationName: Name of the animation to check
	/// @return True if an animation is playing; false otherwise
	bool isPlayingAnimation(char* animationName);

	/// Skip the currently playing animation to the next frame.
	void nextFrame();

	/// Rewind the currently playing animation to the previous frame.
	void previousFrame();

	/// Skip the currently playing animation to the provided frame.
	/// @param actualFrame: The frame of the playing animation to skip to
	/// @return True if the actual frame was changed; false otherwise
	void setActualFrame(int16 actualFrame);

	/// Retrieve the actual frame of the playing animation if any.
	/// @return Actual frame of the playing animation if any
	int16 getActualFrame();

	/// Set the duration in game cycles for each frame of animation.
	/// @param frameDuration: Duration in game cycles for each frame of animation
	void setFrameDuration(uint8 frameDuration);

	/// Retrieve the duration in game cycles for each frame of animation.
	uint8 getFrameDuration();

	/// Set the decrement to frameDuration in each game cycle for each frame of animation.
	/// @param frameDurationDecrement: Decrement to frameDuration in each game cycle for each frame of animation
	void setFrameDurationDecrement(uint8 frameDurationDecrement);

	/// Retrieve the animation function's name currently playing if any
	/// @return Animation function's name currently playing if any
	const char* getPlayingAnimationName();

	/// Set the position cache.
	/// @param position: Position cache to save 
	void setPosition(const PixelVector* position);

	/// Retrieve the position cache.
	const PixelVector* getPosition();

	/// Set the position displacement.
	/// @param displacement: Displacement added to the sprite's position
	void setDisplacement(const PixelVector* displacement);
	
	/// Retrieve the position displacement.
	/// @return Displacement added to the sprite's position
	const PixelVector* getDisplacement();
	
	/// Retrieve the cached position plus the position displacement.
	/// @return Cached position plus the position displacement
	PixelVector getDisplacedPosition();

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
	/// @param charSetPixel: Coordinate in CHAR space of the CHAR to replace
	/// @param newPixelColor: Color data array for the CHAR 
	void putPixel(const Point* texturePixel, const Pixel* charSetPixel, BYTE newPixelColor);

	/// Invalidate the flags that determine if the sprite requires rendering.
	void invalidateRendering();

	/// Register this sprite with the appropriate sprites manager.
	virtual void registerWithManager() = 0;

	/// Unegister this sprite with the appropriate sprites manager.	
	virtual void unregisterWithManager() = 0;

	/// Check if the sprite has special effects.
	/// @return True if the sprite has special effects
	virtual bool hasSpecialEffects();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	virtual int16 doRender(int16 index) = 0;

	/// Update the animation.
	virtual void updateAnimation();

	/// Process special effects.
	virtual void processEffects();

	/// Set the current multiframe.
	/// @param frame: Current animation frame 
	virtual void setMultiframe(uint16 frame);

	/// Forcefully show the sprite
	virtual void forceShow();

	/// Forcefully hide the sprite
	virtual void forceHide();

	/// Set the rotation cache.
	/// @param rotation: Rotation cache to save 
	virtual void setRotation(const Rotation* rotation);

	/// Set the scale cache.
	/// @param scale: Scale cache to save 
	virtual void setScale(const PixelScale* scale);

	/// Retrieve the sprite's total number of pixels actually displayed.
	/// @return Sprite's total number of pixels actually displayed
	virtual int32 getTotalPixels() = 0;

	/// Print the sprite's properties.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	virtual void print(int32 x, int32 y);
}

#endif
