/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ANIMATION_INSPECTOR_STATE_H_
#define ANIMATION_INSPECTOR_STATE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ToolState.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class AnimationInspectorState
///
/// Inherits from ToolState
///
/// Implements a tool state to inspect animations.
singleton class AnimationInspectorState : ToolState
{
	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return AnimationInspectorState singleton
	static AnimationInspectorState getInstance();

	/// Check if the provided user input unlocks the tool managed by this state.
	/// @return True if the input matches the combination defined by the state
	override bool isKeyCombination(const UserInput* userInput);
}

#endif
