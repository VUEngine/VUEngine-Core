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

#include <AnimationCoordinatorFactory.h>
#include <BgmapAnimationCoordinator.h>
#include <ObjectAnimationCoordinator.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationCoordinatorFactory_ATTRIBUTES									\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* entities that use bgmap sprites */										\
	VirtualList animationCoordinators;											\

__CLASS_DEFINITION(AnimationCoordinatorFactory, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimationCoordinatorFactory_constructor(AnimationCoordinatorFactory this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(AnimationCoordinatorFactory);

//class constructor
static void AnimationCoordinatorFactory_constructor(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::constructor: null this");

	__CONSTRUCT_BASE();

	this->animationCoordinators = __NEW(VirtualList);
}

// class destructor
void AnimationCoordinatorFactory_destructor(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::destructor: null this");
	
	AnimationCoordinatorFactory_reset(this);
	__DELETE(this->animationCoordinators);
	this->animationCoordinators = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

void AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory this)
{
	ASSERT(this, "AnimationCoordinatorFactory::reset: null this");

	VirtualNode node = VirtualList_begin(this->animationCoordinators);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		__DELETE(VirtualNode_getData(node));
	}

	VirtualList_clear(this->animationCoordinators);
}

AnimationCoordinator AnimationCoordinatorFactory_getCoordinator(AnimationCoordinatorFactory this, AnimationController animationController, Sprite sprite, CharSet charSet)
{
	ASSERT(this, "AnimationCoordinatorFactory::getCoordinator: null this");

	if(__ANIMATED_SHARED != CharSet_getAllocationType(charSet))
	{
		return NULL;
	}
	
	// try to find an already created coordinator
	VirtualNode node = VirtualList_begin(this->animationCoordinators);
	for(;node; node = VirtualNode_getNext(node))
	{
		AnimationCoordinator animationCoordinator = __UPCAST(AnimationCoordinator, VirtualNode_getData(node));
		
		if(AnimationCoordinator_getCharSet(animationCoordinator) == charSet)
		{
			__VIRTUAL_CALL(void, AnimationCoordinator, addAnimationController, animationCoordinator, animationController);
			return animationCoordinator;
		}
	}
	
	AnimationCoordinator animationCoordinator = NULL;
	
	if(__UPCAST(BgmapAnimatedSprite, sprite))
	{
		animationCoordinator = __UPCAST(AnimationCoordinator, __NEW(BgmapAnimationCoordinator, charSet));
	}
	else if(__UPCAST(ObjectAnimatedSprite, sprite))
	{
		animationCoordinator = __UPCAST(AnimationCoordinator, __NEW(ObjectAnimationCoordinator, charSet));
	}
	else
	{
		NM_ASSERT(this, "AnimationCoordinatorFactory::getCoordinator: invalid sprite type");
	}

	// create a new coordinator
	__VIRTUAL_CALL(void, AnimationCoordinator, addAnimationController, animationCoordinator, animationController);

	VirtualList_pushBack(this->animationCoordinators, animationCoordinator);
	
	return animationCoordinator;
}
