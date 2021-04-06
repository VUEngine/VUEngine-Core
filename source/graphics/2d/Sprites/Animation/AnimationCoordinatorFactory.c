/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

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
 * @return		AnimationCoordinatorFactory instance
 */


/**
 * Class constructor
 *
 * @private
 */
void AnimationCoordinatorFactory::constructor()
{
	Base::constructor();

	this->animationCoordinators = new VirtualList();
}

/**
 * Class destructor
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
 * @private
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
 * @param animationController
 * @param sprite
 * @param charSetSpec
 * @return						AnimationCoordinator instance
 */
AnimationCoordinator AnimationCoordinatorFactory::getCoordinator(AnimationController animationController, Object scope, const CharSetSpec* charSetSpec)
{
	ASSERT(charSetSpec, "AnimationCoordinatorFactory::getCoordinator: null charSetSpec");

	switch(charSetSpec->allocationType)
	{
		case __ANIMATED_SHARED_COORDINATED:
			{
				// try to find an already created coordinator
				VirtualNode node = this->animationCoordinators->head;
				for(;node; node = node->next)
				{
					AnimationCoordinator animationCoordinator = AnimationCoordinator::safeCast(node->data);

					if(AnimationCoordinator::getCharSetSpec(animationCoordinator) == charSetSpec)
					{
						AnimationCoordinator::addAnimationController(animationCoordinator, animationController);
						return animationCoordinator;
					}
				}

				AnimationCoordinator animationCoordinator = new AnimationCoordinator(charSetSpec, scope);

				// create a new coordinator
				AnimationCoordinator::addAnimationController(animationCoordinator, animationController);

				VirtualList::pushBack(this->animationCoordinators, animationCoordinator);

				return animationCoordinator;
			}
			break;
	}

	return NULL;
}
