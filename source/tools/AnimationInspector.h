/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	const AnimationFunction** animationFunctions;
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
	int32 mode;

	/// @publicsection
	static AnimationInspector getInstance();
	override void update();
	override void show();
	override void hide();
	override void processUserInput(uint16 pressedKey);
}

#endif
