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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Actor.h>
#include <Tool.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A struct to map an animated actor spec to a name
/// @memberof AnimationInspector
typedef struct UserActor
{
	/// Specification for an animated actor
	const ActorSpec* actorSpec;

	/// Animated actor spec's name
	const char* name;

} UserActor;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	/// Selector for the animated actors
	OptionsSelector actorSelector;

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
