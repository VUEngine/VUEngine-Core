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

#include <Object.h>
#include <CharSet.h>
#include <Sprite.h>
#include <AnimationController.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-animation
class AnimationCoordinator : Object
{
	// who owns the coordinator
	Object scope;
	// Controllers to sync
	VirtualList animationControllers;
	// Charset spec shared among entities
	const CharSetSpec* charSetSpec;

	/// @publicsection
	void constructor(const CharSetSpec* charSetSpec, Object scope);
	const CharSetSpec* getCharSetSpec();
	bool playAnimation(AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName);
	void addAnimationController(AnimationController animationController);
	void removeAnimationController(AnimationController animationController);
}


#endif
