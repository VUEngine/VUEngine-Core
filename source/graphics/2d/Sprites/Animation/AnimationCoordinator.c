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

#include <string.h>
#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(AnimationCoordinator, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

//class constructor
void AnimationCoordinator_constructor(AnimationCoordinator this, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "AnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->animationControllers = __NEW(VirtualList);
	this->charSetDefinition = charSetDefinition;
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

const CharSetDefinition* AnimationCoordinator_getCharSetDefinition(AnimationCoordinator this)
{
	ASSERT(this, "AnimationCoordinator::getCharSetDefinition: null this");

	return this->charSetDefinition;
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
