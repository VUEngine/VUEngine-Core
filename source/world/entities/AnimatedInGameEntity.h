/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

#ifndef ANIMATED_IN_GAME_ENTITY_H_
#define ANIMATED_IN_GAME_ENTITY_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <InGameEntity.h>
#include <AnimatedSprite.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// defines an AnimatedInGameEntity
typedef struct AnimatedInGameEntityDefinition{

	// It has an InGameEntity at the beggining
	InGameEntityDefinition inGameEntityDefinition;
	
	// the animation
	AnimationDescription* animationDescription;

	// animation to play automatically
	char* initialAnimation;
	
}AnimatedInGameEntityDefinition;


typedef const AnimatedInGameEntityDefinition AnimatedInGameEntityROMDef;



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




// declare the virtual methods
#define AnimatedInGameEntity_METHODS													\
		InGameEntity_METHODS															\


#define AnimatedInGameEntity_SET_VTABLE(ClassName)										\
		InGameEntity_SET_VTABLE(ClassName)												\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, update);							\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, transform);						\
		__VIRTUAL_SET(ClassName, AnimatedInGameEntity, getScale);						\
		

	
#define AnimatedInGameEntity_ATTRIBUTES													\
																						\
	/* super's attributes */															\
	InGameEntity_ATTRIBUTES;															\
																						\
	/* Pointer to the ROM definition */													\
	AnimatedInGameEntityDefinition* animatedInGameEntityDefinition;						\
																						\
	/* Pointer to the animation description */											\
	AnimationDescription* animationDescription;											\
																						\
	Direction previousDirection;														\
																						\
	/* clock to pass to the animated sprites */											\
	Clock clock;																		\


__CLASS(AnimatedInGameEntity);													


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(AnimatedInGameEntity, __PARAMETERS(AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, int ID));

// class's constructor
void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, int ID);

// class's destructor
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this);

// graphically refresh of characters that are visible
void AnimatedInGameEntity_transform(AnimatedInGameEntity this, Transformation* environmentTransform);

// execute character's logic
void AnimatedInGameEntity_update(AnimatedInGameEntity this);

// allocate a write in graphic memory again
void AnimatedInGameEntity_resetMemoryState(AnimatedInGameEntity this, int worldLayer);		

// retrieve character's scale
Scale AnimatedInGameEntity_getScale(AnimatedInGameEntity this);

// pause animation
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause);

// play an animation
void AnimatedInGameEntity_playAnimation(AnimatedInGameEntity this, char* animationName);

// is play an animation
int AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this);

// is animation selected
int AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName);

// check if must update sprite's position
int AnimatedInGameEntity_updateSpritePosition(AnimatedInGameEntity this);

// get animation definition
AnimationDescription* AnimatedInGameEntity_getAnimationDescription(AnimatedInGameEntity this);

// set animation description
void AnimatedInGameEntity_setAnimationDescription(AnimatedInGameEntity this, AnimationDescription* animationDescription);

// set animation clock
void AnimatedInGameEntity_setClock(AnimatedInGameEntity this, Clock clock);

#endif
