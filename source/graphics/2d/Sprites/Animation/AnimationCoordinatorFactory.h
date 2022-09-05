/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_COORDINATOR_FACTORY_H_
#define ANIMATION_COORDINATOR_FACTORY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <ListenerObject.h>
#include <AnimationController.h>
#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-animation
singleton class AnimationCoordinatorFactory : Object
{
	// entities that use bgmap sprites
	VirtualList animationCoordinators;

	/// @publicsection
	static AnimationCoordinatorFactory getInstance();
	AnimationCoordinator getCoordinator(AnimationController animationController, ListenerObject scope, const CharSetSpec* charSetSpec);
	void reset();
}


#endif
