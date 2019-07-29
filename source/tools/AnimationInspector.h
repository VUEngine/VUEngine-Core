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

#ifndef ANIMATION_INSPECTOR_H_
#define ANIMATION_INSPECTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Tool.h>
#include <AnimatedEntity.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * For animation
 *
 * @memberof	AnimationInspector
 */
typedef struct UserAnimatedEntity
{
	/// spec
	const AnimatedEntitySpec* animatedEntitySpec;
	/// name
	const char* name;

} UserAnimatedEntity;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup tools
singleton class AnimationInspector : Tool
{
	// current animated sprite
	Sprite animatedSprite;
	// current animation description
	AnimationDescription* animationDescription;
	// current animation function
	AnimationFunction animationFunction;
	// animated in game entity selector
	OptionsSelector animatedEntitySelector;
	// animated sprite selector
	OptionsSelector spriteSelector;
	// animations selector
	OptionsSelector animationsSelector;
	// animation edition selector
	OptionsSelector animationEditionSelector;
	// frame edition selector
	OptionsSelector frameEditionSelector;
	// mode
	int mode;

	/// @publicsection
	static AnimationInspector getInstance();
	override void update();
	override void show();
	override void hide();
	override void processUserInput(u16 pressedKey);
}


#endif
