/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TOOL_H_
#define TOOL_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>
#include <Stage.h>


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Tool
///
/// Inherits from Object
///
/// Defines an interface for debugging tools.
/// @ingroup tools
abstract class Tool : Object
{
	/// @protectedsection

	/// The stage to work with
	Stage stage;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Set the stage to work with.
	/// @param stage: Stage to work with
	void setStage(Stage stage);

	/// Process the provided user pressed key.
	/// @param pressedKey: User pressed key
	virtual void processUserInput(uint16 pressedKey) = 0;

	/// Dimm down the game.
	virtual void dimmGame();

	/// Light up the game.
	virtual void lightUpGame();

	/// Update the tool's state.
	virtual void update() = 0;

	/// Show the tool.
	virtual void show() = 0;

	/// Hide the tool.
	virtual void hide() = 0;
}

#endif
