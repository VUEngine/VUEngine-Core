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
//											CLASS'S DECLARATION
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


class AnimatedEntity : Entity
{
	/* Pointer to the ROM definition */
	AnimatedEntityDefinition* animatedEntityDefinition;
	/* Pointer to the animation description */
	AnimationDescription* animationDescription;
	/* direction */
	Direction direction;
	/* need to save for pausing */
	char* currentAnimationName;

	void constructor(AnimatedEntity this, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name);
	AnimationDescription* getAnimationDescription(AnimatedEntity this);
	s8 getActualFrame(AnimatedEntity this);
	int getNumberOfFrames(AnimatedEntity this);
	bool isAnimationLoaded(AnimatedEntity this, char* functionName);
	bool isPlayingAnimation(AnimatedEntity this);
	void nextFrame(AnimatedEntity this);
	void pauseAnimation(AnimatedEntity this, int pause);
	void playAnimation(AnimatedEntity this, char* animationName);
	void previousFrame(AnimatedEntity this);
	void setAnimationDescription(AnimatedEntity this, AnimationDescription* animationDescription);
	override void ready(AnimatedEntity this, bool recursive);
	override void update(AnimatedEntity this, u32 elapsedTime);
	override void resume(AnimatedEntity this);
	override void setDefinition(AnimatedEntity this, void* animatedEntityDefinition);
}


#endif
