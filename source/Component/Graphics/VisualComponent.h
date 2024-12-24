/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VISUAL_COMPONENT_H_
#define VISUAL_COMPONENT_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Component.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class AnimationController;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

typedef ComponentSpec VisualComponentSpec;

/// VisualeComponent commands
enum VisualeComponentCommands
{
	cVisualComponentCommandShow = cComponentCommandLast + 1,
	cVisualComponentCommandHide,
	cVisualComponentCommandSetTransparency,
	cVisualComponentCommandLast
};

/// A function which defines the frames to play
/// @memberof VisualeComponent
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
/// Class VirtualNode
///
/// Inherits from Object
///
/// Implements an element of linked lists.
abstract class VisualComponent : Component 
{
	/// @protectedsection

	/// Animation controller
	AnimationController animationController;

	/// Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	uint8 transparency;

	/// Show state flag (__HIDE, __SHOW_NEXT_FRAME, __SHOW)
	uint8 show;

	/// Rendering status flag
	bool rendered;

	/// Flag to allow/prohibit the update of the animation
	bool updateAnimationFrame;
	
	/// @publicsection

	/// Propagate a command to the components.
	/// @param command: Command to propagate to all the components
	/// @param owner: Owner of the components to command (all if NULL)
	/// @param ...: Variable arguments list depending on the command
	static void propagateCommand(int32 command, SpatialObject owner, ...);

	/// Compute the rightbox for the owner in base of its visual components.
	/// @param owner: SpatialObject that the components attaches to
	/// @param rightBox: Rightbox to configure
	/// @return True if the owner has visual components; false otherwise
	static bool calculateRightBox(SpatialObject owner, RightBox* rightBox);

	/// Check if at least of the visual components that attach to the provided owner is visible.
	/// @param owner: Object to which the visual components attach to
	/// @return True if at least of the visual components that attach to the provided owner is visible
	static bool isAnyVisible(SpatialObject owner);

	/// Class' constructor
	/// @param owner: SpatialObject that this component attaches to
	/// @param visualComponentSpec: Pointer to the spec that defines how to initialize the visual component
	void constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec);

	/// Handle a command.
	/// @param command: Command to handle
	/// @param args: Variable arguments list depending on the command to handle
	override void handleCommand(int32 command, va_list args);

	/// Make the visual component visible.
	void show();

	/// Make the visual component invisible.
	void hide();

	/// Retrieve the transparency mode
	/// @return Transparecy effect
	uint8 getTransparent();

	/// Set the transparency mode
	/// @param transparency: Transparecy effect (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
	void setTransparency(uint8 transparency);

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

	/// Retrieve the sprite's animation controller.
	/// @return sprite's animation controller
	AnimationController getAnimationController();

	/// Create an animation controller for this sprite.
	virtual void createAnimationController();

	/// Force the change of frame according to each child class' implementation.
	/// @param actualFrame: The frame of the playing animation to skip to
	virtual void forceChangeOfFrame(int16 actualFrame);

	/// Retrieve the mesh's bounding box.
	/// @return Bounding box of the mesh
	virtual RightBox getRightBox();
}


#endif
