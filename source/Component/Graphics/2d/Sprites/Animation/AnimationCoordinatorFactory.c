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

#include <AnimationController.h>
#include <AnimationCoordinator.h>
#include <ListenerObject.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationCoordinatorFactory.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void AnimationCoordinatorFactory::reset()
{
	AnimationCoordinatorFactory animationCoordinatorFactory = AnimationCoordinatorFactory::getInstance();

	VirtualList::deleteData(animationCoordinatorFactory->animationCoordinators);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static AnimationCoordinator AnimationCoordinatorFactory::getCoordinator(AnimationController animationController, ListenerObject scope, const CharSetSpec* charSetSpec)
{
	AnimationCoordinatorFactory animationCoordinatorFactory = AnimationCoordinatorFactory::getInstance();

	NM_ASSERT(NULL != charSetSpec, "AnimationCoordinatorFactory::getCoordinator: null charSetSpec");

	if(NULL != charSetSpec && charSetSpec->shared)
	{
		// Try to find an already created coordinator
		for(VirtualNode node = animationCoordinatorFactory->animationCoordinators->head; NULL != node; node = node->next)
		{
			AnimationCoordinator animationCoordinator = AnimationCoordinator::safeCast(node->data);

			if(AnimationCoordinator::getCharSetSpec(animationCoordinator) == charSetSpec)
			{
				AnimationCoordinator::addAnimationController(animationCoordinator, animationController);
				return animationCoordinator;
			}
		}

		AnimationCoordinator animationCoordinator = new AnimationCoordinator(charSetSpec, scope);

		// Create a new coordinator
		AnimationCoordinator::addAnimationController(animationCoordinator, animationController);

		VirtualList::pushBack(animationCoordinatorFactory->animationCoordinators, animationCoordinator);

		return animationCoordinator;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinatorFactory::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->animationCoordinators = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationCoordinatorFactory::destructor()
{
	ASSERT(this->animationCoordinators, "AnimationCoordinatorFactory::destructor: null animationCoordinators");

	AnimationCoordinatorFactory::reset(this);
	delete this->animationCoordinators;
	this->animationCoordinators = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
