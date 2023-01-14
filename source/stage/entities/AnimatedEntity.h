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
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


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
	// Pointer to the animation description
	const AnimationFunction** animationFunctions;
	// need to save for pausing
	const char* currentAnimationName;

	/// @publicsection
	void constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name);
	const AnimationFunction** getAnimationFunctions();
	int16 getActualFrame();
	int32 getNumberOfFrames();
	bool isAnimationLoaded(char* functionName);
	bool isPlayingAnimation();
	void nextFrame();
	void pauseAnimation(bool pause);
	bool playAnimation(const char* animationName);
	void previousFrame();
	void setActualFrame(int16 frame);
	void setAnimationFunction(const AnimationFunction** animationFunctions);
	void onAnimationCompleteHide(ListenerObject eventFirer);
	void animate();
	override void ready(bool recursive);
	override void update();
	override void resume();
	override bool handlePropagatedString(const char* string);
}


#endif
