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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	AnimationCoordinator
 * @extends Object
 * @ingroup graphics-2d-sprites-animation
 */

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof					AnimationCoordinator
 * @public
 *
 * @param this					Function scope
 * @param charSetDefinition		CharSetDefinition
 */
void AnimationCoordinator::constructor(const CharSetDefinition* charSetDefinition)
{
	Base::constructor();

	this->animationControllers = new VirtualList();
	this->charSetDefinition = charSetDefinition;
}

/**
 * Class destructor
 *
 * @memberof			AnimationCoordinator
 * @public
 *
 * @param this			Function scope
 */
void AnimationCoordinator::destructor()
{
	ASSERT(this->animationControllers, "AnimationCoordinator::destructor: null animationControllers");

	delete this->animationControllers;
	this->animationControllers = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Class constructor
 *
 * @memberof			AnimationCoordinator
 * @public
 *
 * @param this			Function scope
 *
 * @return 				CharSetDefinition
 */
const CharSetDefinition* AnimationCoordinator::getCharSetDefinition()
{
	return this->charSetDefinition;
}

/**
 * Class constructor
 *
 * @memberof						AnimationCoordinator
 * @public
 *
 * @param this						Function scope
 * @param animationController		Animation controller
 * @param animationDescription		Animation description holding the animation function
 * @param functionName				Name of the animation function's to play
 *
 * @return 							True if the animation started playing
 */
bool AnimationCoordinator::playAnimation(AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName)
{
	if(this->animationControllers->head)
	{
		AnimationController firstAnimationController = __SAFE_CAST(AnimationController, VirtualList::front(this->animationControllers));

		if(animationController == firstAnimationController)
		{
			return true;
		}

		// only if not playing already
		if(!AnimationController::isPlaying(firstAnimationController) || strncmp(functionName, AnimationController::getPlayingAnimationFunction(firstAnimationController)->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
		{
			// first animate the frame
			AnimationController::play(firstAnimationController, animationDescription, functionName);
		}

		return false;
	}

	return true;
}
