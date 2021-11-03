/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Clock.h>
#include <CharSet.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Forward declaration
class AnimationCoordinator;

/// @ingroup graphics-2d-sprites-animation
class AnimationController : Object
{
	// who owns the animated sprite
	AnimationCoordinator animationCoordinator;
	// a pointer to the animation function being played
	const AnimationFunction* animationFunction;
	// actual animation's frame to show
	int16 actualFrame;
	// previous animation's frame shown
	uint8 previousFrameValue;
	// the number of game cycles that an animation frame is shown
	uint8 frameDuration;
	// frame delay decrement update cycle
	uint8 frameCycleDecrement;
	// flag to know if playing an animation
	uint8 playing;
	// frame changed flag
	uint8 animationFrameChanged;

	/// @publicsection
	void constructor();
	AnimationCoordinator getAnimationCoordinator();
	void setAnimationCoordinator(AnimationCoordinator animationCoordinator);
	int16 getActualFrame();
	int16 getActualFrameIndex();
	uint8 getCols();
	uint8 getFrameCycleDecrement();
	uint8 getFrameDuration();
	int32 getMapType();
	int32 getNumberOfFrames();
	const AnimationFunction* getPlayingAnimationFunction();
	uint8 getRows();
	bool isPlaying();
	bool isPlayingFunction(const char* functionName);
	void pause(bool pause);
	bool play(const AnimationDescription* animationDescription, const char* functionName, Object scope);
	bool replay(const AnimationDescription* animationDescription);
	void playAnimationFunction(const AnimationFunction* animationFunction, Object scope);
	void nextFrame();
	void previousFrame();
	bool setActualFrame(int16 actualFrame);
	void setFrameCycleDecrement(uint8 frameCycleDecrement);
	void setFrameDuration(uint8 frameDuration);
	void stop();
	bool update(Clock clock);
	bool updateAnimation();
	void write();
	void writeAnimation();
}


#endif
