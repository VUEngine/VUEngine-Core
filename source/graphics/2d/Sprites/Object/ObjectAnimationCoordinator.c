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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param charSetSpec		CharSetSpec
 */
void ObjectAnimationCoordinator::constructor(const CharSetSpec* charSetSpec)
{
	Base::constructor(charSetSpec);
}

/**
 * Class destructor
 */
void ObjectAnimationCoordinator::destructor()
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
void ObjectAnimationCoordinator::addAnimationController(AnimationController animationController)
{
	ASSERT(animationController, "ObjectAnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList::find(this->animationControllers, animationController), "ObjectAnimationCoordinator::addAnimationController: animationController already registered");

	if(this->animationControllers->head)
	{
		AnimationController::stop(animationController);
	}

	VirtualList::pushBack(this->animationControllers, animationController);
}

/**
 * Remove an AnimationController
 *
 * @param animationController		Animation controller to unregister
 */
void ObjectAnimationCoordinator::removeAnimationController(AnimationController animationController)
{
	ASSERT(this->animationControllers->head, "ObjectAnimationCoordinator::removeAnimationController: null this");

	bool mustChangeLeader = animationController == AnimationController::safeCast(VirtualList::front(this->animationControllers));
	VirtualList::removeElement(this->animationControllers, animationController);

	if(mustChangeLeader && this->animationControllers->head)
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		if(firstAnimationController)
		{
			if(AnimationController::isPlaying(animationController))
			{
				AnimationController::playAnimationFunction(firstAnimationController, AnimationController::getPlayingAnimationFunction(animationController));
				s16 currentFrame = AnimationController::getActualFrame(firstAnimationController);
				u8 frameDuration = AnimationController::getFrameDuration(firstAnimationController);
				AnimationController::setActualFrame(firstAnimationController, currentFrame);
				AnimationController::setFrameDuration(firstAnimationController, frameDuration);
				AnimationController::stop(animationController);
			}
		}
	}
}
