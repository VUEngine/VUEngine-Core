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

#include <Object.h>
#include <AnimatedEntity.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods

/**
 * For animation
 *
 * @memberof	AnimationInspector
 */
typedef struct UserAnimatedEntity
{
	/// definition
	const AnimatedEntityDefinition* animatedEntityDefinition;
	/// name
	const char* name;

} UserAnimatedEntity;


singleton class AnimationInspector : Object
{
	/**
	 * @var GameState 				gameState
	 * @brief						current in game gameState
	 * @memberof					AnimationInspector
	 */
	GameState gameState;
	/**
	 * @var Sprite 					animatedSprite
	 * @brief						current animated sprite
	 * @memberof					AnimationInspector
	 */
	Sprite animatedSprite;
	/**
	 * @var AnimationDescription* 	animationDescription
	 * @brief						current animation description
	 * @memberof					AnimationInspector
	 */
	AnimationDescription* animationDescription;
	/**
	 * @var AnimationFunction 		animationFunction
	 * @brief						current animation function
	 * @memberof					AnimationInspector
	 */
	AnimationFunction animationFunction;
	/**
	 * @var OptionsSelector 		animatedEntitySelector
	 * @brief						animated in game entity selector
	 * @memberof					AnimationInspector
	 */
	OptionsSelector animatedEntitySelector;
	/**
	 * @var OptionsSelector 		spriteSelector
	 * @brief						animated sprite selector
	 * @memberof					AnimationInspector
	 */
	OptionsSelector spriteSelector;
	/**
	 * @var OptionsSelector 		animationsSelector
	 * @brief						animations selector
	 * @memberof					AnimationInspector
	 */
	OptionsSelector animationsSelector;
	/**
	 * @var OptionsSelector 		animationEditionSelector
	 * @brief						animation edition selector
	 * @memberof					AnimationInspector
	 */
	OptionsSelector animationEditionSelector;
	/**
	 * @var OptionsSelector 		frameEditionSelector
	 * @brief						frame edition selector
	 * @memberof					AnimationInspector
	 */
	OptionsSelector frameEditionSelector;
	/**
	 * @var int mode
	 * @brief						mode
	 * @memberof					AnimationInspector
	 */
	int mode;

	static AnimationInspector getInstance();
	void update(AnimationInspector this);
	void show(AnimationInspector this, GameState gameState);
	void hide(AnimationInspector this);
	void processUserInput(AnimationInspector this, u16 pressedKey);
}


#endif
