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

#ifndef ANIMATED_ENTITY_H_
#define ANIMATED_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>
#include <BgmapAnimatedSprite.h>
#include <ObjectAnimatedSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines an AnimatedEntity
typedef struct AnimatedEntityDefinition
{
	// it has an Entity at the beginning
	EntityDefinition entityDefinition;

	// the animation
	AnimationDescription* animationDescription;

	// animation to play automatically
	char* initialAnimation;

} AnimatedEntityDefinition;

typedef const AnimatedEntityDefinition AnimatedEntityROMDef;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimatedEntity_METHODS(ClassName)																\
		Entity_METHODS(ClassName)																		\

#define AnimatedEntity_SET_VTABLE(ClassName)															\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, AnimatedEntity, ready);												\
		__VIRTUAL_SET(ClassName, AnimatedEntity, update);												\
		__VIRTUAL_SET(ClassName, AnimatedEntity, resume);												\
		__VIRTUAL_SET(ClassName, AnimatedEntity, setDefinition);										\

#define AnimatedEntity_ATTRIBUTES																		\
		/* super's attributes */																		\
		Entity_ATTRIBUTES																				\
		/* Pointer to the ROM definition */																\
		AnimatedEntityDefinition* animatedEntityDefinition;												\
		/* Pointer to the animation description */														\
		AnimationDescription* animationDescription;														\
		/* direction */																					\
		Direction direction;																			\
		/* need to save for pausing */																	\
		char* currentAnimationName;																		\

__CLASS(AnimatedEntity);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimatedEntity, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name);

void AnimatedEntity_constructor(AnimatedEntity this, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name);
void AnimatedEntity_destructor(AnimatedEntity this);

AnimationDescription* AnimatedEntity_getAnimationDescription(AnimatedEntity this);
s8 AnimatedEntity_getActualFrame(AnimatedEntity this);
int AnimatedEntity_getNumberOfFrames(AnimatedEntity this);
bool AnimatedEntity_isAnimationLoaded(AnimatedEntity this, char* functionName);
bool AnimatedEntity_isPlayingAnimation(AnimatedEntity this);
void AnimatedEntity_nextFrame(AnimatedEntity this);
void AnimatedEntity_pauseAnimation(AnimatedEntity this, int pause);
void AnimatedEntity_playAnimation(AnimatedEntity this, char* animationName);
void AnimatedEntity_previousFrame(AnimatedEntity this);
void AnimatedEntity_ready(AnimatedEntity this, bool recursive);
void AnimatedEntity_resume(AnimatedEntity this);
void AnimatedEntity_setAnimationDescription(AnimatedEntity this, AnimationDescription* animationDescription);
void AnimatedEntity_setDefinition(AnimatedEntity this, void* animatedEntityDefinition);
void AnimatedEntity_update(AnimatedEntity this, u32 elapsedTime);


#endif
