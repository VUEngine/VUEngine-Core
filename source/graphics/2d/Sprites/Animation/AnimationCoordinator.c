/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <AnimationCoordinator.h>


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
void AnimationCoordinator::constructor(const CharSetSpec* charSetSpec, ListenerObject scope)
{
	Base::constructor();

	this->scope = scope;
	this->animationControllers = new VirtualList();
	this->charSetSpec = charSetSpec;
}

/**
 * Class destructor
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

const CharSetSpec* AnimationCoordinator::getCharSetSpec()
{
	return this->charSetSpec;
}


bool AnimationCoordinator::playAnimation(AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName)
{
	if(this->animationControllers->head)
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		if(animationController == firstAnimationController)
		{
			return true;
		}

		// only if not playing already
		if(!AnimationController::isPlaying(firstAnimationController) || strncmp(functionName, AnimationController::getPlayingAnimationFunction(firstAnimationController)->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
		{
			// first animate the frame
			AnimationController::play(firstAnimationController, animationDescription, functionName, this->scope);
		}

		return false;
	}

	return true;
}

/**
 * Add an AnimationController
 *
 * @param animationController		Animation controller to register
 */
void AnimationCoordinator::addAnimationController(AnimationController animationController)
{
	ASSERT(animationController, "AnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList::find(this->animationControllers, animationController), "AnimationCoordinator::addAnimationController: animationController already registered");

	if(!VirtualList::find(this->animationControllers, animationController))
	{
		VirtualList::pushBack(this->animationControllers, animationController);
	}
}

/**
 * Remove an AnimationController
 *
 * @param animationController		Animation controller to unregister
 */
void AnimationCoordinator::removeAnimationController(AnimationController animationController)
{
	ASSERT(this->animationControllers->head, "AnimationCoordinator::removeAnimationController: null this");

	bool mustChangeLeader = animationController == AnimationController::safeCast(VirtualList::front(this->animationControllers));
	VirtualList::removeElement(this->animationControllers, animationController);

	if(mustChangeLeader && !isDeleted(this->animationControllers->head))
	{
		AnimationController firstAnimationController = AnimationController::safeCast(VirtualList::front(this->animationControllers));

		if(firstAnimationController)
		{
			if(AnimationController::isPlaying(animationController))
			{
				AnimationController::playAnimationFunction(firstAnimationController, AnimationController::getPlayingAnimationFunction(animationController), this->scope);
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
