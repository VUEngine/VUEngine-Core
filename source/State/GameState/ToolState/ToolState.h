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
// CLASS'S DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Tool;
class ToolState;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ToolState
///
/// Inherits from GameState
///
/// Defines an interface for game states that uses various debug tools.
abstract class ToolState : GameState
{
	/// The tool that the state uses
	Tool tool;

	/// Game's current state
	GameState currentGameState;

	/// @publicsection

	/// Retrive the tool state that is unlocked by the provided user input.
	/// @param userInput: User's keypad intpu
	/// @return The ToolState that is unlocked if any
	static ToolState get(const UserInput* userInput);

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

	/// Stream in or out the stage actors within or outside the camera's range.
	/// @return True if at least some actor was streamed in or out
	override bool stream();

	/// Set the VUEngine's current game state.
	/// @param currentGameState: Game's current game state
	void setCurrentGameState(GameState currentGameState);

	/// Set the VUEngine's current game state.
	/// @return Game's current game state
	GameState getCurrentGameState();

	/// Check if the provided user input unlocks the tool managed by this state.
	/// @return True if the input matches the combination defined by the state
	virtual bool isKeyCombination(const UserInput* userInput) = 0;
}

#endif
