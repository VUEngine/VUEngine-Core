/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	BgmapAnimationCoordinator
 * @extends AnimationCoordinator
 * @ingroup graphics-2d-sprites-bgmap
 */
__CLASS_DEFINITION(BgmapAnimationCoordinator, AnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapAnimationCoordinator, const CharSetDefinition* charSetDefinition)
__CLASS_NEW_END(BgmapAnimationCoordinator, charSetDefinition);

/**
 * Class constructor
 *
 * @memberof		BgmapAnimationCoordinator
 * @public
 *
 * @param this		Function scope
 */
void BgmapAnimationCoordinator_constructor(BgmapAnimationCoordinator this, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "BgmapAnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE(AnimationCoordinator, charSetDefinition);
}

/**
 * Class destructor
 *
 * @memberof		BgmapAnimationCoordinator
 * @public
 *
 * @param this		Function scope
 */
void BgmapAnimationCoordinator_destructor(BgmapAnimationCoordinator this)
{
	ASSERT(this, "BgmapAnimationCoordinator::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Add an AnimationController
 *
 * @memberof						BgmapAnimationCoordinator
 * @public
 *
 * @param this						Function scope
 * @param animationController		Animation controller to register
 */
void BgmapAnimationCoordinator_addAnimationController(BgmapAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "BgmapAnimationCoordinator::addAnimationController: null this");
	ASSERT(animationController, "BgmapAnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList_find(this->animationControllers, animationController), "BgmapAnimationCoordinator::addAnimationController: animationController already registered");

	if(VirtualList_front(this->animationControllers))
	{
		AnimationController firstAnimationController = __SAFE_CAST(AnimationController, VirtualList_front(this->animationControllers));

		ASSERT(firstAnimationController, "BgmapAnimationCoordinator::addAnimationController: null firstAnimationController");

		if(AnimationController_isPlaying(firstAnimationController))
		{
			AnimationController_playAnimationFunction(animationController, AnimationController_getPlayingAnimationFunction(firstAnimationController));
			s8 currentFrame = AnimationController_getActualFrame(firstAnimationController);
			s8 frameDuration = AnimationController_getFrameDuration(firstAnimationController);
			AnimationController_setActualFrame(animationController, currentFrame);
			AnimationController_setFrameDuration(animationController, frameDuration);
		}
	}

	VirtualList_pushBack(this->animationControllers, animationController);
}

/**
 * Remove an AnimationController
 *
 * @memberof						BgmapAnimationCoordinator
 * @public
 *
 * @param this						Function scope
 * @param animationController		Animation controller to unregister
 */
void BgmapAnimationCoordinator_removeAnimationController(BgmapAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "BgmapAnimationCoordinator::removeAnimationController: null this");

	VirtualList_removeElement(this->animationControllers, animationController);
}

