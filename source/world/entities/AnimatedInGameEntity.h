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

#ifndef ANIMATED_IN_GAME_ENTITY_H_
#define ANIMATED_IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines an AnimatedInGameEntity
typedef struct AnimatedInGameEntityDefinition
{
	// it has an InGameEntity at the beginning
	InGameEntityDefinition inGameEntityDefinition;

	// the animation
	AnimationDescription* animationDescription;

	// animation to play automatically
	char* initialAnimation;

} AnimatedInGameEntityDefinition;

typedef const AnimatedInGameEntityDefinition AnimatedInGameEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimatedInGameEntity_METHODS(ClassName)																	\
		InGameEntity_METHODS(ClassName)																			\

#define AnimatedInGameEntity_SET_VTABLE(ClassName)														\
		InGameEntity_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, ready);										    \
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, update);											\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, transform);										\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, resume);											\

#define AnimatedInGameEntity_ATTRIBUTES																	\
        /* super's attributes */																		\
        InGameEntity_ATTRIBUTES																		    \
        /* Pointer to the ROM definition */																\
        AnimatedInGameEntityDefinition* animatedInGameEntityDefinition;									\
        /* Pointer to the animation description */														\
        AnimationDescription* animationDescription;														\
        /* used to know if gap must be changed */														\
        Direction previousDirection;																	\
        /* clock to pass to the animated sprites */														\
        Clock clock;																					\
        /* need to save for pausing */																	\
        char* currentAnimationName;																		\

__CLASS(AnimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimatedInGameEntity, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, const char* const name);

void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, const char* const name);
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this);
void AnimatedInGameEntity_ready(AnimatedInGameEntity this);
void AnimatedInGameEntity_transform(AnimatedInGameEntity this, const Transformation* environmentTransform);
void AnimatedInGameEntity_update(AnimatedInGameEntity this);
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause);
void AnimatedInGameEntity_playAnimation(AnimatedInGameEntity this, char* animationName);
bool AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this);
bool AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName);
int AnimatedInGameEntity_updateSpritePosition(AnimatedInGameEntity this);
AnimationDescription* AnimatedInGameEntity_getAnimationDescription(AnimatedInGameEntity this);
void AnimatedInGameEntity_setAnimationDescription(AnimatedInGameEntity this, AnimationDescription* animationDescription);
void AnimatedInGameEntity_setClock(AnimatedInGameEntity this, Clock clock);
void AnimatedInGameEntity_resume(AnimatedInGameEntity this);


#endif
