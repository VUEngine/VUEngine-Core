/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS

//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <StageEditor.h>

#include "StageEditorState.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StageEditorState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(StageEditor::getInstance());
}
//---------------------------------------------------------------------------------------------------------
void StageEditorState::destructor()
{
	// destroy base
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool StageEditorState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_A) && (userInput->holdKey & K_B) && (userInput->releasedKey & K_RL));
}
//---------------------------------------------------------------------------------------------------------

#endif