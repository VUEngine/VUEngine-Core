/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_COORDINATOR_FACTORY_H_
#define ANIMATION_COORDINATOR_FACTORY_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <CharSet.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class AnimationController;
class AnimationCoordinator;
class ListenerObject;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class AnimationCoordinator
///
/// Inherits from ListenerObject
///
/// Creates instances of animation coordinators.
singleton class AnimationCoordinatorFactory : Object
{
	/// @protectedsection

	/// List of instances of animation coordinators
	VirtualList animationCoordinators;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return AnimationCoordinatorFactory singleton
	static AnimationCoordinatorFactory getInstance();

	/// Reset the animation coordinator factory's state.
	void reset();

	/// Get an animation coordinator.
	/// @param animationController: Animation controller to potentially coordinate
	/// @param scope: Object that might need to listen for playback related events
	/// @param charSetSpec: CharSetSpec shared by the animation controllers to coordinate   
	/// @return ClockManager singleton
	AnimationCoordinator getCoordinator(AnimationController animationController, ListenerObject scope, const CharSetSpec* charSetSpec);
}


#endif
