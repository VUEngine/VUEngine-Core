/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

/// @ingroup graphics-2d-sprites-animation
class AnimationController : Object
{
	// who owns the animated sprite
	Object owner;
	// who owns the animated sprite
	AnimationCoordinator animationCoordinator;
	// a pointer to the animation function being played
	const AnimationFunction* animationFunction;
	// actual animation's frame to show
	s16 actualFrame;
	// previous animation's frame shown
	s16 previousFrame;
	// the number of game cycles that an animation frame is shown
	s8 frameDuration;
	// frame delay decrement update cycle
	s8 frameCycleDecrement;
	// flag to know if playing an animation
	u8 playing;
	// frame changed flag
	u8 animationFrameChanged;

	/// @publicsection
	void constructor(Object owner, Sprite sprite, const CharSetSpec* charSetSpec);
	s16 getActualFrame();
	s16 getActualFrameIndex();
	u8 getCols();
	u8 getFrameCycleDecrement();
	s8 getFrameDuration();
	int getMapType();
	int getNumberOfFrames();
	const AnimationFunction* getPlayingAnimationFunction();
	s16 getPreviousFrame();
	u8 getRows();
	bool isPlaying();
	bool isPlayingFunction(const char* functionName);
	void nextFrame();
	void pause(bool pause);
	bool play(const AnimationDescription* animationDescription, const char* functionName);
	void playAnimationFunction(const AnimationFunction* animationFunction);
	void previousFrame();
	bool setActualFrame(s16 actualFrame);
	void setFrameCycleDecrement(u8 frameCycleDecrement);
	void setFrameDuration(u8 frameDuration);
	void stop();
	bool update(Clock clock);
	bool updateAnimation();
	void write();
	void writeAnimation();
}


#endif
