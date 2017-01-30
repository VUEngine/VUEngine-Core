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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationCoordinatorFactory.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationCoordinatorFactory_ATTRIBUTES                                                          \
        /* super's attributes */                                                                        \
        Object_ATTRIBUTES                                                                              \
        /* entities that use bgmap sprites */                                                           \
        VirtualList animationCoordinators;                                                              \

__CLASS_DEFINITION(AnimationCoordinatorFactory, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimationCoordinatorFactory_constructor(AnimationCoordinatorFactory this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(AnimationCoordinatorFactory);

//class constructor
static void __attribute__ ((noinline)) AnimationCoordinatorFactory_constructor(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->animationCoordinators = __NEW(VirtualList);
}

// class destructor
void AnimationCoordinatorFactory_destructor(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::destructor: null this");
	ASSERT(this->animationCoordinators, "AnimationCoordinatorFactory::destructor: null animationCoordinators");

	AnimationCoordinatorFactory_reset(this);
	__DELETE(this->animationCoordinators);
	this->animationCoordinators = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

void AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::reset: null this");

	VirtualNode node = this->animationCoordinators->head;

	for(; node; node = node->next)
	{
		__DELETE(node->data);
	}

	VirtualList_clear(this->animationCoordinators);
}

AnimationCoordinator AnimationCoordinatorFactory_getCoordinator(AnimationCoordinatorFactory this, AnimationController animationController, Sprite sprite, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "AnimationCoordinatorFactory::getCoordinator: null this");
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

					if(AnimationCoordinator_getCharSetDefinition(animationCoordinator) == charSetDefinition)
					{
						__VIRTUAL_CALL(AnimationCoordinator, addAnimationController, animationCoordinator, animationController);
						return animationCoordinator;
					}
				}

				AnimationCoordinator animationCoordinator = NULL;

				if(__GET_CAST(BgmapAnimatedSprite, sprite))
				{
					animationCoordinator = __SAFE_CAST(AnimationCoordinator, __NEW(BgmapAnimationCoordinator, charSetDefinition));
				}
				else if(__GET_CAST(ObjectAnimatedSprite, sprite))
				{
					animationCoordinator = __SAFE_CAST(AnimationCoordinator, __NEW(ObjectAnimationCoordinator, charSetDefinition));
				}
				else
				{
					NM_ASSERT(this, "AnimationCoordinatorFactory::getCoordinator: invalid sprite type");
				}

				// create a new coordinator
				__VIRTUAL_CALL(AnimationCoordinator, addAnimationController, animationCoordinator, animationController);

				VirtualList_pushBack(this->animationCoordinators, animationCoordinator);

				return animationCoordinator;
			}
			break;
	}

	return NULL;

}
