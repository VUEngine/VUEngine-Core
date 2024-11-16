/**
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
/// @ingroup stage-entities
class AnimatedEntity : Entity
{
	/// Pointer to the animations available to this instance
	const AnimationFunction** animationFunctions;

	/// Name of the currently playing animation
	const char* playingAnimationName;

	/// @publicsection
	void constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name);
	bool playAnimation(const char* animationName);
	bool isPlaying();
	bool isPlayingAnimation(char* animationName);
	const char* getPlayingAnimationName();
	void pauseAnimation(bool pause);
	void setActualFrame(int16 frame);
	void nextFrame();
	void previousFrame();
	int16 getActualFrame();
	int32 getNumberOfFrames();

	/// Make the animated entity ready to starts operating once it has been completely intialized.
	/// @param recursive: If true, the ready call is propagated to its children, grand children, etc.
	override void ready(bool recursive);

	/// Prepare to resume this instance's logic.
	override void resume();

	/// Default string handler for propagateString
	/// @param string: Propagated string
	/// @return True if the propagation must stop; false if the propagation must reach other containers
	override bool handlePropagatedString(const char* string);
}


#endif
