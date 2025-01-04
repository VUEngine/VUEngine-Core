/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TOOL_STATE_H_
#define TOOL_STATE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <GameState.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//											CLASS'S DECLARATION

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


class Tool;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class ToolState
///
/// Inherits from GameState
///
/// Defines an interface for game states that uses various debug tools.
abstract class ToolState : GameState
{
	/// The tool that the state uses
	Tool tool;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Prepares the object to enter this state.
	/// @param owner: Object that is entering in this state
	override void enter(void* owner);

	/// Updates the object in this state.
	/// @param owner: Object that is in this state
	override void execute(void* owner);
	
	/// Prepares the object to exit this state.
	/// @param owner: Object that is exiting this state
	override void exit(void* owner);

	/// Process the provided user input.
	/// @param userInput: Struct with the current user input information
	override void processUserInput(const UserInput*  userInput);

	/// Stream in or out the stage entities within or outside the camera's range.
	/// @return True if at least some entity was streamed in or out
	override bool stream();

	/// Check if the provided user input unlocks the tool managed by this state.
	/// @return True if the input matches the combination defined by the state
	virtual bool isKeyCombination(const UserInput* userInput) = 0;
}


#endif
