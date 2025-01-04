/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STAGE_EDITOR_STATE_H_
#define STAGE_EDITOR_STATE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ToolState.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class StageEditorState
///
/// Inherits from ToolState
///
/// Implements a tool state to manipulate stages.
singleton class StageEditorState : ToolState
{
	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return StageEditorState singleton
	static StageEditorState getInstance();

	/// Check if the provided user input unlocks the tool managed by this state.
	/// @return True if the input matches the combination defined by the state
	override bool isKeyCombination(const UserInput* userInput);
}

#endif