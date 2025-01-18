/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <AnimationController.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationCoordinator.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinator::constructor(const CharSetSpec* charSetSpec, ListenerObject scope)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->scope = scope;
	this->animationControllers = new VirtualList();
	this->charSetSpec = charSetSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinator::destructor()
{
	ASSERT(this->animationControllers, "AnimationCoordinator::destructor: null animationControllers");

	delete this->animationControllers;
	this->animationControllers = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimationCoordinator::playAnimation(AnimationController animationController, const AnimationFunction** animationFunctions, const char* animationName)
{
	if(!isDeleted(this->animationControllers->head))
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		if(animationController == firstAnimationController)
		{
			return true;
		}

		// Only if not playing already
		if
		(
			!AnimationController::isPlaying(firstAnimationController) 
			|| 
			strncmp
			(
				animationName, 
				AnimationController::getPlayingAnimationFunction(firstAnimationController)->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH
			)
		)
		{
			// First animate the frame
			AnimationController::play(firstAnimationController, animationFunctions, animationName, this->scope, NULL);
		}

		return false;
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinator::addAnimationController(AnimationController animationController)
{
	ASSERT(animationController, "AnimationCoordinator::addAnimationController: null animationController");
	ASSERT
	(
		!VirtualList::find(this->animationControllers, animationController), 
		"AnimationCoordinator::addAnimationController: animationController already registered"
	);

	if(!VirtualList::find(this->animationControllers, animationController))
	{
		VirtualList::pushBack(this->animationControllers, animationController);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinator::removeAnimationController(AnimationController animationController)
{
	NM_ASSERT(this->animationControllers->head, "AnimationCoordinator::removeAnimationController: null this");

	bool mustChangeLeader = animationController == AnimationController::safeCast(VirtualList::front(this->animationControllers));
	VirtualList::removeData(this->animationControllers, animationController);

	if(mustChangeLeader && !isDeleted(this->animationControllers->head))
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		if(firstAnimationController)
		{
			if(AnimationController::isPlaying(animationController))
			{
				AnimationController::playAnimationFunction
				(
					firstAnimationController, AnimationController::getPlayingAnimationFunction(animationController), this->scope
				);

				int16 currentFrame = AnimationController::getActualFrame(animationController);
				uint8 frameDuration = AnimationController::getFrameDuration(animationController);
				AnimationController::setActualFrame(firstAnimationController, currentFrame);
				AnimationController::setFrameDuration(firstAnimationController, frameDuration);
				AnimationController::stop(animationController);
			 	AnimationController::fireEvent(firstAnimationController, kEventAnimationStarted);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const CharSetSpec* AnimationCoordinator::getCharSetSpec()
{
	return this->charSetSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
