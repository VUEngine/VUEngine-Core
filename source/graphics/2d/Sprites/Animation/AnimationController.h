/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
