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

class AnimationController : Object
{
	/**
	* @var Object 						owner
	* @brief							who owns the animated sprite
	* @memberof						AnimationController
	*/
	Object owner;
	/**
	* @var AnimationCoordinator 		animationCoordinator
	* @brief							who owns the animated sprite
	* @memberof						AnimationController
	*/
	AnimationCoordinator animationCoordinator;
	/**
	* @var s8 							actualFrame
	* @brief							actual animation's frame to show
	* @memberof						AnimationController
	*/
	s8 actualFrame;
	/**
	* @var s8 							previousFrame
	* @brief							previous animation's frame shown
	* @memberof						AnimationController
	*/
	s8 previousFrame;
	/**
	* @var s8 							frameDuration
	* @brief							the number of game cycles that an animation frame is show
	* @memberof						AnimationController
	*/
	s8 frameDuration;
	/**
	* @var s8 							frameCycleDecrement
	* @brief							frame delay decrement update cycle
	* @memberof						AnimationController
	*/
	s8 frameCycleDecrement;
	/**
	* @var const AnimationFunction*	animationFunction
	* @brief							a pointer to the animation function being played
	* @memberof						AnimationController
	*/
	const AnimationFunction* animationFunction;
	/**
	* @var u8 							playing
	* @brief							flag to know if playing an animation
	* @memberof						AnimationController
	*/
	u8 playing;
	/**
	* @var u8 							animationFrameChanged
	* @brief							frame changed flag
	* @memberof						AnimationController
	*/
	u8 animationFrameChanged;

	void constructor(Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition);
	s8 getActualFrame();
	s8 getActualFrameIndex();
	u8 getCols();
	u8 getFrameCycleDecrement();
	s8 getFrameDuration();
	int getMapType();
	int getNumberOfFrames();
	const AnimationFunction* getPlayingAnimationFunction();
	s8 getPreviousFrame();
	u8 getRows();
	bool isPlaying();
	bool isPlayingFunction(const char* functionName);
	void nextFrame();
	void pause(bool pause);
	bool play(const AnimationDescription* animationDescription, const char* functionName);
	void playAnimationFunction(const AnimationFunction* animationFunction);
	void previousFrame();
	void setActualFrame(s8 actualFrame);
	void setFrameCycleDecrement(u8 frameCycleDecrement);
	void setFrameDuration(u8 frameDuration);
	void stop();
	bool update(Clock clock);
	bool updateAnimation();
	void write();
	void writeAnimation();
}


#endif
