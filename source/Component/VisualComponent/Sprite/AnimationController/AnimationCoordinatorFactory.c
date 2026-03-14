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
#include <Singleton.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationCoordinatorFactory.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

AnimationCoordinator AnimationCoordinatorFactory::getCoordinator(AnimationController animationController, ListenerObject scope, const TileSetSpec* tileSetSpec)
{
	NM_ASSERT(NULL != tileSetSpec, "AnimationCoordinatorFactory::getCoordinator: null tileSetSpec");

	if(NULL != tileSetSpec && tileSetSpec->shared)
	{
		// Try to find an already created coordinator
		for(VirtualNode node = this->animationCoordinators->head; NULL != node; node = node->next)
		{
			AnimationCoordinator animationCoordinator = AnimationCoordinator::safeCast(node->data);

			if(AnimationCoordinator::getTileSetSpec(animationCoordinator) == tileSetSpec)
			{
				AnimationCoordinator::addAnimationController(animationCoordinator, animationController);
				return animationCoordinator;
			}
		}

		AnimationCoordinator animationCoordinator = new AnimationCoordinator(tileSetSpec, scope);

		// Create a new coordinator
		AnimationCoordinator::addAnimationController(animationCoordinator, animationController);

		VirtualList::pushBack(this->animationCoordinators, animationCoordinator);

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

	if(NULL != this->animationCoordinators)
	{
		VirtualList::deleteData(this->animationCoordinators);
		delete this->animationCoordinators;
		this->animationCoordinators = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
