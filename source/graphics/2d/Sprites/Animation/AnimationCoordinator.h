/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

class AnimationCoordinator : Object
{
	/**
	* @var VirtualList 		animationControllers
	* @brief					Controllers to sync
	* @memberof				AnimationController
	*/
	VirtualList animationControllers;
	/**
	* @var CharSetDefinition 	charSetDefinition
	* @brief					Charset definition shared among entities
	* @memberof				AnimationController
	*/
	const CharSetDefinition* charSetDefinition;

	void constructor(AnimationCoordinator this, const CharSetDefinition* charSetDefinition);
	const CharSetDefinition* getCharSetDefinition(AnimationCoordinator this);
	bool playAnimation(AnimationCoordinator this, AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName);
	virtual void addAnimationController(AnimationCoordinator this, AnimationController animationController) = 0;
	virtual void removeAnimationController(AnimationCoordinator this, AnimationController animationController) = 0;
}


#endif
