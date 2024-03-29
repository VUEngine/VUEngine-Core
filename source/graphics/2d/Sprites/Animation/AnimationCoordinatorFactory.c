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

#include <AnimationController.h>
#include <AnimationCoordinator.h>
#include <ListenerObject.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "AnimationCoordinatorFactory.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationCoordinatorFactory::getInstance()
 * @memberof	AnimationCoordinatorFactory
 * @public
 * @return		AnimationCoordinatorFactory instance
 */


/**
 * Class constructor
 *
 * @private
 */
void AnimationCoordinatorFactory::constructor()
{
	Base::constructor();

	this->animationCoordinators = new VirtualList();
}

/**
 * Class destructor
 */
void AnimationCoordinatorFactory::destructor()
{
	ASSERT(this->animationCoordinators, "AnimationCoordinatorFactory::destructor: null animationCoordinators");

	AnimationCoordinatorFactory::reset(this);
	delete this->animationCoordinators;
	this->animationCoordinators = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset
 *
 * @private
 */
void AnimationCoordinatorFactory::reset()
{
	VirtualList::deleteData(this->animationCoordinators);
}

/**
 * Get Coordinator
 *
 * @param animationController
 * @param sprite
 * @param charSetSpec
 * @return						AnimationCoordinator instance
 */
AnimationCoordinator AnimationCoordinatorFactory::getCoordinator(AnimationController animationController, ListenerObject scope, const CharSetSpec* charSetSpec)
{
	ASSERT(charSetSpec, "AnimationCoordinatorFactory::getCoordinator: null charSetSpec");

	if(charSetSpec->shared)
	{
		// try to find an already created coordinator
		for(VirtualNode node = this->animationCoordinators->head; NULL != node; node = node->next)
		{
			AnimationCoordinator animationCoordinator = AnimationCoordinator::safeCast(node->data);

			if(AnimationCoordinator::getCharSetSpec(animationCoordinator) == charSetSpec)
			{
				AnimationCoordinator::addAnimationController(animationCoordinator, animationController);
				return animationCoordinator;
			}
		}

		AnimationCoordinator animationCoordinator = new AnimationCoordinator(charSetSpec, scope);

		// create a new coordinator
		AnimationCoordinator::addAnimationController(animationCoordinator, animationController);

		VirtualList::pushBack(this->animationCoordinators, animationCoordinator);

		return animationCoordinator;
	}

	return NULL;
}
