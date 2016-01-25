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

#include <string.h>
#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(AnimationCoordinator, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

//class constructor
void AnimationCoordinator_constructor(AnimationCoordinator this, CharSet charSet)
{
	ASSERT(this, "AnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE();

	this->animationControllers = __NEW(VirtualList);
	this->charSet = charSet;
}

// class destructor
void AnimationCoordinator_destructor(AnimationCoordinator this)
{
	ASSERT(this, "AnimationCoordinator::destructor: null this");
	ASSERT(this->animationControllers, "AnimationCoordinator::destructor: null animationControllers");

	__DELETE(this->animationControllers);
	this->animationControllers = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

const CharSet AnimationCoordinator_getCharSet(AnimationCoordinator this)
{
	ASSERT(this, "AnimationCoordinator::getCharSet: null this");

	return this->charSet;
}

bool AnimationCoordinator_playAnimation(AnimationCoordinator this, AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName)
{
	ASSERT(this, "AnimationCoordinator::playAnimation: null this");

	if(this->animationControllers->head)
	{
		AnimationController firstAnimationController = __SAFE_CAST(AnimationController, VirtualList_front(this->animationControllers));

		if(animationController == firstAnimationController)
		{
			return true;
		}

		// only if not playing already
		if(!AnimationController_isPlaying(firstAnimationController) || strncmp(functionName, AnimationController_getPlayingAnimationFunction(firstAnimationController)->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
		{
			// first animate the frame
			AnimationController_play(firstAnimationController, animationDescription, functionName);
		}
		
		return false;
	}
	
	return true;
}
