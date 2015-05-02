/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectAnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


__CLASS_DEFINITION(ObjectAnimationCoordinator, AnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectAnimationCoordinator, const CharSet charSet)
__CLASS_NEW_END(ObjectAnimationCoordinator, charSet);

//class constructor
void ObjectAnimationCoordinator_constructor(ObjectAnimationCoordinator this, const CharSet charSet)
{
	ASSERT(this, "ObjectAnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE(charSet);
}

// class destructor
void ObjectAnimationCoordinator_destructor(ObjectAnimationCoordinator this)
{
	ASSERT(this, "ObjectAnimationCoordinator::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

void ObjectAnimationCoordinator_addAnimationController(ObjectAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "ObjectAnimationCoordinator::addAnimationController: null this");
	ASSERT(animationController, "ObjectAnimationCoordinator::addAnimationController: null animationController");
	ASSERT(!VirtualList_find(this->animationControllers, animationController), "ObjectAnimationCoordinator::addAnimationController: animationController already registered");

	if(VirtualList_begin(this->animationControllers))
	{
		AnimationController firstAnimationController = __UPCAST(AnimationController, VirtualList_front(this->animationControllers));
	
		ASSERT(firstAnimationController, "ObjectAnimationCoordinator::addAnimationController: null firstAnimationController");

		AnimationController_stop(animationController);
	}
	
	VirtualList_pushBack(this->animationControllers, animationController);
}

void ObjectAnimationCoordinator_removeAnimationController(ObjectAnimationCoordinator this, AnimationController animationController)
{
	ASSERT(this, "ObjectAnimationCoordinator::removeAnimationController: null this");
	ASSERT(VirtualList_begin(this->animationControllers), "ObjectAnimationCoordinator::removeAnimationController: null this");

	bool mustChangeLeader = animationController == __UPCAST(AnimationController, VirtualList_front(this->animationControllers));
	VirtualList_removeElement(this->animationControllers, animationController);
	
	if(mustChangeLeader && VirtualList_begin(this->animationControllers))
	{
		AnimationController firstAnimationController = __UPCAST(AnimationController, VirtualList_front(this->animationControllers));
		
		if(firstAnimationController)
		{
			if(AnimationController_isPlaying(animationController))
			{
				AnimationController_playAnimationFunction(firstAnimationController, AnimationController_getPlayingAnimationFunction(animationController));
				s8 currentFrame = AnimationController_getActualFrame(firstAnimationController);
				s8 frameDelay = AnimationController_getFrameDelay(firstAnimationController);
				AnimationController_setActualFrame(firstAnimationController, currentFrame);
				AnimationController_setFrameDelay(firstAnimationController, frameDelay);
				AnimationController_stop(animationController);
			}
		}
	}
}

