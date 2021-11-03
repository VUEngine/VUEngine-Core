/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StageEditorState.h>
#include <StageEditor.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StageEditorState::getInstance()
 * @memberof	StageEditorState
 * @public
 * @return		StageEditorState instance
 */


/**
 * Class constructor
 *
 * @private
 */
void StageEditorState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(StageEditor::getInstance());
}

/**
 * Class destructor
 *
 * @private
 */
void StageEditorState::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Check if key combinations invokes me
 *
 * @public
 * @param userInput		UserInput
 *
 * @return bool
 */
bool StageEditorState::isKeyCombination(UserInput userInput)
{
	return ((userInput.holdKey & K_LT) && (userInput.holdKey & K_RT) && (userInput.releasedKey & K_RL));
}

#endif