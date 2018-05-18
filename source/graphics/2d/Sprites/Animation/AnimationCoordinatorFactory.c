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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationCoordinatorFactory.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	AnimationCoordinatorFactory
 * @extends Object
 * @ingroup graphics-2d-sprites-animation
 */

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationCoordinatorFactory::getInstance()
 * @memberof	AnimationCoordinatorFactory
 * @public
 *
 * @return		AnimationCoordinatorFactory instance
 */


/**
 * Class constructor
 *
 * @memberof	AnimationCoordinatorFactory
 * @private
 *
 * @param this	Function scope
 */
void AnimationCoordinatorFactory::constructor()
{
	Base::constructor();

	this->animationCoordinators = new VirtualList();
}

/**
 * Class destructor
 *
 * @memberof	AnimationCoordinatorFactory
 * @public
 *
 * @param this	Function scope
 */
void AnimationCoordinatorFactory::destructor()
{
	ASSERT(this->animationCoordinators, "AnimationCoordinatorFactory::destructor: null animationCoordinators");

	AnimationCoordinatorFactory::reset(this);
	delete this->animationCoordinators;
	this->animationCoordinators = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset
 *
 * @memberof	AnimationCoordinatorFactory
 * @private
 *
 * @param this	Function scope
 */
void AnimationCoordinatorFactory::reset()
{
	VirtualNode node = this->animationCoordinators->head;

	for(; node; node = node->next)
	{
		delete node->data;
	}

	VirtualList::clear(this->animationCoordinators);
}

/**
 * Get Coordinator
 *
 * @memberof					AnimationCoordinatorFactory
 * @public
 *
 * @param this					Function scope
 * @param animationController
 * @param sprite
 * @param charSetDefinition
 *
 * @return						AnimationCoordinator instance
 */
AnimationCoordinator AnimationCoordinatorFactory::getCoordinator(AnimationController animationController, Sprite sprite, const CharSetDefinition* charSetDefinition)
{
	ASSERT(sprite, "AnimationCoordinatorFactory::getCoordinator: null sprite");
	ASSERT(charSetDefinition, "AnimationCoordinatorFactory::getCoordinator: null charSetDefinition");

	switch(charSetDefinition->allocationType)
	{
		case __ANIMATED_SHARED_COORDINATED:
			{
				// try to find an already created coordinator
				VirtualNode node = this->animationCoordinators->head;
				for(;node; node = node->next)
				{
					AnimationCoordinator animationCoordinator = __SAFE_CAST(AnimationCoordinator, node->data);

					if(AnimationCoordinator::getCharSetDefinition(animationCoordinator) == charSetDefinition)
					{
						 AnimationCoordinator::addAnimationController(animationCoordinator, animationController);
						return animationCoordinator;
					}
				}

				AnimationCoordinator animationCoordinator = NULL;

				if(__GET_CAST(BgmapAnimatedSprite, sprite))
				{
					animationCoordinator = __SAFE_CAST(AnimationCoordinator, new BgmapAnimationCoordinator(charSetDefinition));
				}
				else if(__GET_CAST(ObjectAnimatedSprite, sprite))
				{
					animationCoordinator = __SAFE_CAST(AnimationCoordinator, new ObjectAnimationCoordinator(charSetDefinition));
				}
				else
				{
					NM_ASSERT(this, "AnimationCoordinatorFactory::getCoordinator: invalid sprite type");
				}

				// create a new coordinator
				 AnimationCoordinator::addAnimationController(animationCoordinator, animationController);

				VirtualList::pushBack(this->animationCoordinators, animationCoordinator);

				return animationCoordinator;
			}
			break;
	}

	return NULL;
}
