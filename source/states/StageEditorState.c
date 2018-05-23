/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StageEditorState.h>
#include <StageEditor.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>
#include <KeyPadManager.h>


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
 * Method called when the Game's StateMachine enters to this state
 *
 * @param owner		StateMachine's owner
 */
void StageEditorState::enter(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);
	GameState::pauseClocks(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
	StageEditor::show(StageEditor::getInstance(), GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
}

/**
 * Method called when by the StateMachine's update method
 *
 * @param owner		StateMachine's owner
 */
void StageEditorState::execute(void* owner __attribute__ ((unused)))
{
	StageEditor::update(StageEditor::getInstance());
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @param owner		StateMachine's owner
 */
void StageEditorState::exit(void* owner __attribute__ ((unused)))
{
	StageEditor::hide(StageEditor::getInstance());
	GameState::resumeClocks(GameState::safeCast(StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
	Base::exit(this, owner);
}

/**
 * Process user input
 *
 * @param userInput		User input
 */
void StageEditorState::processUserInput(UserInput userInput)
{
	StageEditor::processUserInput(StageEditor::getInstance(), userInput.pressedKey);
}
