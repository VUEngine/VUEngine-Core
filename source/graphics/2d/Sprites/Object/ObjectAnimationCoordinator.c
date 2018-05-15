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

#include <ObjectAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	ObjectAnimationCoordinator
 * @extends AnimationCoordinator
 * @ingroup graphics-2d-sprites-object
 */

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof					ObjectAnimationCoordinator
 * @public
 *
 * @param this					Function scope
 * @param charSetDefinition		CharSetDefinition
 */
void ObjectAnimationCoordinator::constructor(ObjectAnimationCoordinator this, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "ObjectAnimationCoordinator::constructor: null this");

	Base::constructor(charSetDefinition);
}

/**
 * Class destructor
 *
 * @memberof					ObjectAnimationCoordinator
 * @public
 *
 * @param this					Function scope
 */
void ObjectAnimationCoordinator::destructor(ObjectAnimationCoordinator this)
{
	ASSERT(this, "ObjectAnimationCoordinator::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add an AnimationController
 *
 * @memberof					ObjectAnimationCoordinator
 * @public
 *
 * @param this					Function scope
 * @param animationController		Animation controller to register
 */
void ObjectAnimationCoordinator::addAnimationController(ObjectAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "ObjectAnimationCoordinator::addAnimationController: null this");
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
 * @memberof						ObjectAnimationCoordinator
 * @public
 *
 * @param this						Function scope
 * @param animationController		Animation controller to unregister
 */
void ObjectAnimationCoordinator::removeAnimationController(ObjectAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "ObjectAnimationCoordinator::removeAnimationController: null this");
	ASSERT(this->animationControllers->head, "ObjectAnimationCoordinator::removeAnimationController: null this");

	bool mustChangeLeader = animationController == __SAFE_CAST(AnimationController, VirtualList::front(this->animationControllers));
	VirtualList::removeElement(this->animationControllers, animationController);

	if(mustChangeLeader && this->animationControllers->head)
	{
		AnimationController firstAnimationController = __SAFE_CAST(AnimationController, VirtualList::front(this->animationControllers));

		if(firstAnimationController)
		{
			if(AnimationController::isPlaying(animationController))
			{
				AnimationController::playAnimationFunction(firstAnimationController, AnimationController::getPlayingAnimationFunction(animationController));
				s8 currentFrame = AnimationController::getActualFrame(firstAnimationController);
				s8 frameDuration = AnimationController::getFrameDuration(firstAnimationController);
				AnimationController::setActualFrame(firstAnimationController, currentFrame);
				AnimationController::setFrameDuration(firstAnimationController, frameDuration);
				AnimationController::stop(animationController);
			}
		}
	}
}

