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

#include <BgmapAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param charSetDefinition		CharSetDefinition used by the sprites to be coordinated
 */
void BgmapAnimationCoordinator::constructor(const CharSetDefinition* charSetDefinition)
{
	Base::constructor(charSetDefinition);
}

/**
 * Class destructor
 */
void BgmapAnimationCoordinator::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add an AnimationController
 *
 * @param animationController		Animation controller to register
 */
void BgmapAnimationCoordinator::addAnimationController(AnimationController animationController)
{
	ASSERT(animationController, "BgmapAnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList::find(this->animationControllers, animationController), "BgmapAnimationCoordinator::addAnimationController: animationController already registered");

	if(VirtualList::front(this->animationControllers))
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		ASSERT(firstAnimationController, "BgmapAnimationCoordinator::addAnimationController: null firstAnimationController");

		if(AnimationController::isPlaying(firstAnimationController))
		{
			AnimationController::playAnimationFunction(animationController, AnimationController::getPlayingAnimationFunction(firstAnimationController));
			s16 currentFrame = AnimationController::getActualFrame(firstAnimationController);
			s8 frameDuration = AnimationController::getFrameDuration(firstAnimationController);
			AnimationController::setActualFrame(animationController, currentFrame);
			AnimationController::setFrameDuration(animationController, frameDuration);
		}
	}

	VirtualList::pushBack(this->animationControllers, animationController);
}

/**
 * Remove an AnimationController
 *
 * @param animationController		Animation controller to unregister
 */
void BgmapAnimationCoordinator::removeAnimationController(AnimationController animationController)
{
	VirtualList::removeElement(this->animationControllers, animationController);
}

