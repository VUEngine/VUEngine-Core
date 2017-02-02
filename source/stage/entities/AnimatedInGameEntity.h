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

#ifndef ANIMATED_IN_GAME_ENTITY_H_
#define ANIMATED_IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimatedInGameEntity_METHODS(ClassName)															\
		InGameEntity_METHODS(ClassName)																	\

#define AnimatedInGameEntity_SET_VTABLE(ClassName)														\
		InGameEntity_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, ready);											\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, update);											\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, transform);										\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, resume);											\

#define AnimatedInGameEntity_ATTRIBUTES																	\
		/* super's attributes */																		\
		InGameEntity_ATTRIBUTES																			\
		/* Pointer to the ROM definition */																\
		AnimatedInGameEntityDefinition* animatedInGameEntityDefinition;									\
		/* Pointer to the animation description */														\
		AnimationDescription* animationDescription;														\
		/* used to know if gap must be changed */														\
		Direction previousDirection;																	\
		/* need to save for pausing */																	\
		char* currentAnimationName;																		\

__CLASS(AnimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimatedInGameEntity, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name);

void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name);
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this);
void AnimatedInGameEntity_setDefinition(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition);
void AnimatedInGameEntity_ready(AnimatedInGameEntity this, u32 recursive);
void AnimatedInGameEntity_transform(AnimatedInGameEntity this, const Transformation* environmentTransform);
void AnimatedInGameEntity_update(AnimatedInGameEntity this, u32 elapsedTime);
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause);
void AnimatedInGameEntity_playAnimation(AnimatedInGameEntity this, char* animationName);
bool AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this);
bool AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName);
int AnimatedInGameEntity_updateSpritePosition(AnimatedInGameEntity this);
AnimationDescription* AnimatedInGameEntity_getAnimationDescription(AnimatedInGameEntity this);
void AnimatedInGameEntity_setAnimationDescription(AnimatedInGameEntity this, AnimationDescription* animationDescription);
void AnimatedInGameEntity_resume(AnimatedInGameEntity this);


#endif
