/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TOOL_STATE_H_
#define TOOL_STATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Tool;

/// @ingroup states
abstract class ToolState : GameState
{
	Tool tool;

	/// @publicsection
	void constructor();

	virtual bool isKeyCombination(const UserInput* userInput) = 0;

	override void enter(void* owner);
	override void execute(void* owner);
	override void exit(void* owner);
	override void processUserInput(const UserInput*  userInput);
	override bool stream();
}


#endif
