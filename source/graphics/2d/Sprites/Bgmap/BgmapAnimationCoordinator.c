/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


__CLASS_DEFINITION(BgmapAnimationCoordinator, AnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapAnimationCoordinator, const CharSet charSet)
__CLASS_NEW_END(BgmapAnimationCoordinator, charSet);

//class constructor
void BgmapAnimationCoordinator_constructor(BgmapAnimationCoordinator this, const CharSet charSet)
{
	ASSERT(this, "BgmapAnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE(charSet);
}

// class destructor
void BgmapAnimationCoordinator_destructor(BgmapAnimationCoordinator this)
{
	ASSERT(this, "BgmapAnimationCoordinator::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

void BgmapAnimationCoordinator_addAnimationController(BgmapAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "BgmapAnimationCoordinator::addAnimationController: null this");
	ASSERT(animationController, "BgmapAnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList_find(this->animationControllers, animationController), "BgmapAnimationCoordinator::addAnimationController: animationController already registered");

	if(VirtualList_front(this->animationControllers))
	{
		AnimationController firstAnimationController = __GET_CAST(AnimationController, VirtualList_front(this->animationControllers));
	
		ASSERT(firstAnimationController, "BgmapAnimationCoordinator::addAnimationController: null firstAnimationController");
	
		if(AnimationController_isPlaying(firstAnimationController))
		{
			AnimationController_playAnimationFunction(animationController, AnimationController_getPlayingAnimationFunction(firstAnimationController));
			s8 currentFrame = AnimationController_getActualFrame(firstAnimationController);
			s8 frameDelay = AnimationController_getFrameDelay(firstAnimationController);
			AnimationController_setActualFrame(animationController, currentFrame);
			AnimationController_setFrameDelay(animationController, frameDelay);
		}
	}
	
	VirtualList_pushBack(this->animationControllers, animationController);
}

void BgmapAnimationCoordinator_removeAnimationController(BgmapAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "BgmapAnimationCoordinator::removeAnimationController: null this");

	VirtualList_removeElement(this->animationControllers, animationController);
}

