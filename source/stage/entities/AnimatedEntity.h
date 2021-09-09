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

typedef struct AnimatedEntitySpec
{
	// it has an Entity at the beginning
	EntitySpec entitySpec;

	// the animation
	AnimationDescription* animationDescription;

	// animation to play automatically
	char* initialAnimation;

} AnimatedEntitySpec;

typedef const AnimatedEntitySpec AnimatedEntityROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class AnimatedEntity : Entity
{
	// Pointer to the ROM spec
	AnimatedEntitySpec* animatedEntitySpec;
	// Pointer to the animation description
	AnimationDescription* animationDescription;
	// need to save for pausing
	char* currentAnimationName;

	/// @publicsection
	void constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name);
	AnimationDescription* getAnimationDescription();
	int16 getActualFrame();
	int32 getNumberOfFrames();
	bool isAnimationLoaded(char* functionName);
	bool isPlayingAnimation();
	void nextFrame();
	void pauseAnimation(bool pause);
	void playAnimation(char* animationName);
	void previousFrame();
	void setActualFrame(int16 frame);
	void setAnimationDescription(AnimationDescription* animationDescription);
	void onAnimationCompleteHide(Object eventFirer);
	virtual void animate();
	override void ready(bool recursive);
	override void update(uint32 elapsedTime);
	override void resume();
	override void setSpec(void* animatedEntitySpec);
}


#endif
