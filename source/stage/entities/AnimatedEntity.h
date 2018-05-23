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
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

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

/// @ingroup stage-entities
class AnimatedEntity : Entity
{
	// Pointer to the ROM definition
	AnimatedEntityDefinition* animatedEntityDefinition;
	// Pointer to the animation description
	AnimationDescription* animationDescription;
	// direction
	Direction direction;
	// need to save for pausing
	char* currentAnimationName;

	/// @publicsection
	void constructor(AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name);
	AnimationDescription* getAnimationDescription();
	s8 getActualFrame();
	int getNumberOfFrames();
	bool isAnimationLoaded(char* functionName);
	bool isPlayingAnimation();
	void nextFrame();
	void pauseAnimation(int pause);
	void playAnimation(char* animationName);
	void previousFrame();
	void setAnimationDescription(AnimationDescription* animationDescription);
	override void ready(bool recursive);
	override void update(u32 elapsedTime);
	override void resume();
	override void setDefinition(void* animatedEntityDefinition);
}


#endif
