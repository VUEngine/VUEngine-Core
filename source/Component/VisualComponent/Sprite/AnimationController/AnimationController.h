/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>
#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class AnimationCoordinator;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class AnimationController
///
/// Inherits from ListenerObject
///
/// Controls animations.
class AnimationController : ListenerObject
{
	/// @protectedsection

	/// Animation coordinator that syncs the animations with other animation controllers
	AnimationCoordinator animationCoordinator;

	/// Pointer to the current animation function being played
	const AnimationFunction* animationFunction;

	/// The current frame of the playing animation function
	int16 actualFrame;

	/// The current frame index of the playing animation function
	int16 actualFrameIndex;	

	/// Number of game cycles that an animation frame persists
	uint8 frameDuration;

	/// Decrement to frameDuration in each game cycle
	uint8 frameDurationDecrement;

	/// Flag that signals if playing an animation
	uint8 playing;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Play the animation with the provided name from the provided array of animation functions.
	/// @param animationFunctions: Array of functions where to look for the animation to play
	/// @param animationName: Name of the animation to play
	/// @param scope: Object that will be notified of playback events
	/// @return True if the animation started playing; false otherwise
	bool play(const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope);

	/// Play the animation defined by the the provided animation function.
	/// @param animationFunction: Animation function to play
	/// @param scope: Object that will be notified of playback events
	void playAnimationFunction(const AnimationFunction* animationFunction, ListenerObject scope);

	/// Replay the last playing animation, if any, from the provided array of animation functions.
	/// @param animationFunctions: Array of animation functions to look for the animation function to replay
	/// @return True if the animation started playing again; false otherwise
	bool replay(const AnimationFunction* animationFunctions[]);

	/// Pause or unpause the currently playing animation if any.
	/// @param pause: Flag that signals if the animation must be paused or unpaused
	void pause(bool pause);

	/// Stop any playing animation if any.
	void stop();

	/// Update the current playing animation if any.
	bool updateAnimation();

	/// Check if an animation is playing.
	/// @return True if an animation is playing; false otherwise
	bool isPlaying();

	/// Check if the animation whose name is provided is playing.
	/// @param animationName: Name of the animation to check
	/// @return True if an animation is playing; false otherwise
	bool isPlayingFunction(const char* animationName);

	/// Skip the currently playing animation to the next frame.
	void nextFrame();

	/// Rewind the currently playing animation to the previous frame.
	void previousFrame();

	/// Skip the currently playing animation to the provided frame.
	/// @param actualFrame: The frame of the playing animation to skip to
	/// @return True if the actual frame was changed; false otherwise
	bool setActualFrame(int16 actualFrame);

	/// Retrieve the actual frame of the playing animation if any.
	/// @return Actual frame of the playing animation if any
	int16 getActualFrame();

	/// Retrieve the actual frame index of the playing animation if any.
	/// @return Actual frame index of the playing animation if any
	int16 getActualFrameIndex();

	/// Set the duration in game cycles for each frame of animation.
	/// @param frameDuration: Duration in game cycles for each frame of animation
	void setFrameDuration(uint8 frameDuration);

	/// Retrieve the duration in game cycles for each frame of animation.
	uint8 getFrameDuration();

	/// Set the decrement to frameDuration in each game cycle for each frame of animation.
	/// @param frameCycleDecrement: Decrement to frameDuration in each game cycle for each frame of animation
	void setFrameDurationDecrement(uint8 frameCycleDecrement);

	/// Retrieve the decrement to frameDuration in each game cycle for each frame of animation.
	/// @return Decrement to frameDuration in each game cycle for each frame of animation
	uint8 getFrameDurationDecrement();

	/// Set the animation coordinator for this animation controller.
	/// @param animationCoordinator: Animation coordinator for this animation controller
	void setAnimationCoordinator(AnimationCoordinator animationCoordinator);

	/// Retrieve the animation coordinator for this animation controller.
	/// @return Animation coordinator for this animation controller
	AnimationCoordinator getAnimationCoordinator();

	/// Retrieve the animation function currently playing if any
	/// @return Animation function currently playing if any
	const AnimationFunction* getPlayingAnimationFunction();

	/// Retrieve the animation function's name currently playing if any
	/// @return Animation function's name currently playing if any
	const char* getPlayingAnimationName();

	/// Retrieve the number of frames in the currently playing if any
	/// @return Number of frames in the currently playing if any
	int32 getNumberOfFrames();

	/// Check if the currently playing animation is looped or not.
	/// @return True or false
	bool isAnimationLooped();
}

#endif
