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
