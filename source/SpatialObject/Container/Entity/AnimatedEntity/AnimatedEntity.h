/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATED_ENTITY_H_
#define ANIMATED_ENTITY_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Entity.h>
#include <Sprite.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// An AnimatedEntity Spec
/// @memberof AnimatedEntity
typedef struct AnimatedEntitySpec
{
	// it has an Entity at the beginning
	EntitySpec entitySpec;

	// the animations
	const AnimationFunction** animationFunctions;

	// animation to play automatically
	char* initialAnimation;

} AnimatedEntitySpec;

/// A AnimatedEntity spec that is stored in ROM
/// @memberof AnimatedEntity
typedef const AnimatedEntitySpec AnimatedEntityROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class AnimatedEntity
///
/// Inherits from Entity
///
/// Implements an entity that proxies animation related methods to its sprites.
class AnimatedEntity : Entity
{
	/// Name of the currently playing animation
	const char* playingAnimationName;

	/// @publicsection

	/// @param animatedEntitySpec: Specification that determines how to configure the actor
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name);

	/// Make the animated entity ready to start operating once it has been completely intialized.
	/// @param recursive: If true, the ready call is propagated to its children, grand children, etc.
	override void ready(bool recursive);

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Default string handler for propagateString
	/// @param string: Propagated string
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override bool handlePropagatedString(const char* string);

	/// Play the animation with the provided name.
	/// @param animationName: Name of the animation to play
	void playAnimation(const char* animationName);

	/// Pause or unpause the currently playing animation if any.
	/// @param pause: Flag that signals if the animation must be paused or unpaused
	void pauseAnimation(bool pause);

	/// Stop any playing animation if any.
	void stopAnimation();

	/// Check if an animation is playing.
	/// @return True if an animation is playing; false otherwise
	bool isPlaying();

	/// Check if the animation whose name is provided is playing.
	/// @param animationName: Name of the animation to check
	/// @return True if an animation is playing; false otherwise
	bool isPlayingAnimation(char* animationName);

	/// Retrieve the animation function's name currently playing if any
	/// @return Animation function's name currently playing if any
	const char* getPlayingAnimationName();

	/// Skip the currently playing animation to the provided frame.
	/// @param frame: The frame of the playing animation to skip to
	/// @return True if the actual frame was changed; false otherwise
	void setActualFrame(int16 frame);

	/// Skip the currently playing animation to the next frame.
	void nextFrame();

	/// Rewind the currently playing animation to the previous frame.
	void previousFrame();

	/// Retrieve the actual frame of the playing animation if any.
	/// @return Actual frame of the playing animation if any
	int16 getActualFrame();

	/// Retrieve the number of frames in the currently playing animation if any.
	/// @return The numer of frames if an animation is playing; o otherwise
	int32 getNumberOfFrames();
}


#endif
