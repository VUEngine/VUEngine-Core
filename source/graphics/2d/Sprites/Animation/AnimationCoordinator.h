/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_COORDINATOR_H_
#define ANIMATION_COORDINATOR_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>
#include <CharSet.h>
#include <Sprite.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class AnimationController;


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class AnimationCoordinator
///
/// Inherits from ListenerObject
///
/// Coordinates animation controllers whose sprites share the same CharSet.
/// @ingroup graphics-2d-sprites-animation
class AnimationCoordinator : ListenerObject
{
	/// @protectedsection

	/// Object that might need to listen for playback related events
	ListenerObject scope;

	/// List of animation controllers to sync
	VirtualList animationControllers;

	// Spec shared by the animation controllers to coordinate
	const CharSetSpec* charSetSpec;

	/// @publicsection

	/// Class' constructor
	/// @param charSetSpec: Spec shared by the animation controllers to coordinate
	/// @param scope: Object that might need to listen for playback related events
	void constructor(const CharSetSpec* charSetSpec, ListenerObject scope);

	/// Play an animation on the specified animation controller.
	/// @param animationController: Animation controller on which to play the animation
	/// @param animationFunctions: Array of animation functions in which to look for the animation to play
	/// @param animationName: Name of the animation to play
	/// @return True if the animation was found and started to play; false otherwise
	bool playAnimation(AnimationController animationController, const AnimationFunction** animationFunctions, const char* animationName);

	/// Add an animation controller to coordinate.
	/// @param animationController: Animation controller to coordinate
	void addAnimationController(AnimationController animationController);

	/// Removed a coordinated animation controller.
	/// @param animationController: Coordinated animation controller to remove
	void removeAnimationController(AnimationController animationController);

	/// Retrieve the spec shared by the animation controllers to coordinate.
	/// @return Pointer to spec shared by the animation controllers to coordinate
	const CharSetSpec* getCharSetSpec();
}


#endif
