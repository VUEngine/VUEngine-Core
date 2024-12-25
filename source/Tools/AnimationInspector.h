/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_INSPECTOR_H_
#define ANIMATION_INSPECTOR_H_

//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <AnimatedEntity.h>
#include <Tool.h>

//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A struct to map an animated entity spec to a name
/// @memberof AnimationInspector
typedef struct UserAnimatedEntity
{
	/// Specification for an animated entity
	const AnimatedEntitySpec* animatedEntitySpec;

	/// Animated entity spec's name
	const char* name;

} UserAnimatedEntity;

//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class AnimationInspector
///
/// Inherits from Tool
///
/// Implements a tool that manipulates animations.
singleton class AnimationInspector : Tool
{
	/// Currently inspected sprite
	Sprite sprite;

	/// Animations to apply to the animated sprite
	const AnimationFunction** animationFunctions;

	/// Currently applied animation
	AnimationFunction animationFunction;

	/// Selector for the animated entities
	OptionsSelector animatedEntitySelector;

	/// Selector for the animated sprite sprite selector
	OptionsSelector spriteSelector;

	/// Selector for the animations to play
	OptionsSelector animationsSelector;

	/// Selector for the animation's properties
	OptionsSelector animationEditionSelector;

	/// Selector for the animation's frames
	OptionsSelector frameEditionSelector;

	/// Inspector's state
	int32 state;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return AnimationInspector singleton
	static AnimationInspector getInstance();

	/// Update the tool's state.
	override void update();

	/// Show the tool.
	override void show();

	/// Hide the tool.
	override void hide();

	/// Process the provided user pressed key.
	/// @param pressedKey: User pressed key
	override void processUserInput(uint16 pressedKey);
}

#endif
