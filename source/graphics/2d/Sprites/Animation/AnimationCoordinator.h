/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_COORDINATOR_H_
#define ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <CharSet.h>
#include <Sprite.h>
#include <AnimationController.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-animation
class AnimationCoordinator : ListenerObject
{
	// who owns the coordinator
	ListenerObject scope;
	// Controllers to sync
	VirtualList animationControllers;
	// Charset spec shared among entities
	const CharSetSpec* charSetSpec;

	/// @publicsection
	void constructor(const CharSetSpec* charSetSpec, ListenerObject scope);
	const CharSetSpec* getCharSetSpec();
	bool playAnimation(AnimationController animationController, const AnimationFunction** animationFunctions, const char* functionName);
	void addAnimationController(AnimationController animationController);
	void removeAnimationController(AnimationController animationController);
}


#endif
