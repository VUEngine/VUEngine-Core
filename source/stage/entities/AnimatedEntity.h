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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __ANIMATION_COMMAND_PLAY								"play:"


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef struct AnimatedEntitySpec
{
	// it has an Entity at the beginning
	EntitySpec entitySpec;

	// the animations
	const AnimationFunction** animationFunctions;

	// animation to play automatically
	char* initialAnimation;

} AnimatedEntitySpec;

typedef const AnimatedEntitySpec AnimatedEntityROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

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

	override void ready(bool recursive);
	override void resume();
	override bool handlePropagatedString(const char* string);
}


#endif
