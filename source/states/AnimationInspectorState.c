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

#include <AnimationInspectorState.h>
#include <AnimationInspector.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>
#include <KeyPadManager.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimationInspectorState::constructor(AnimationInspectorState this);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	AnimationInspectorState
 * @extends GameState
 * @ingroup states
 */



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationInspectorState::getInstance()
 * @memberof	AnimationInspectorState
 * @public
 *
 * @return		AnimationInspectorState instance
 */
__SINGLETON(AnimationInspectorState);

/**
 * Class constructor
 *
 * @memberof	AnimationInspectorState
 * @private
 *
 * @param this	Function scope
 */
void __attribute__ ((noinline)) AnimationInspectorState::constructor(AnimationInspectorState this)
{
	ASSERT(this, "AnimationInspectorState::constructor: null this");

	Base::constructor();
}

/**
 * Class destructor
 *
 * @memberof	AnimationInspectorState
 * @private
 *
 * @param this	Function scope
 */
void AnimationInspectorState::destructor(AnimationInspectorState this)
{
	ASSERT(this, "AnimationInspectorState::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @memberof		AnimationInspectorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void AnimationInspectorState::enter(AnimationInspectorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "AnimationInspectorState::enter: null this");

	Base::enter(this, owner);
	GameState::pauseClocks(__SAFE_CAST(GameState, StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
	AnimationInspector::show(AnimationInspector::getInstance(), __SAFE_CAST(GameState, StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		AnimationInspectorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void AnimationInspectorState::execute(AnimationInspectorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "AnimationInspectorState::execute: null this");

	AnimationInspector::update(AnimationInspector::getInstance());
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @memberof		AnimationInspectorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void AnimationInspectorState::exit(AnimationInspectorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "AnimationInspectorState::exit: null this");

	AnimationInspector::hide(AnimationInspector::getInstance());
	GameState::resumeClocks(__SAFE_CAST(GameState, StateMachine::getPreviousState(Game::getStateMachine(Game::getInstance()))));
	Base::exit(this, owner);
}

/**
 * Process user input
 *
 * @memberof			AnimationInspectorState
 * @private
 *
 * @param this			Function scope
 * @param userInput		User input
 */
void AnimationInspectorState::processUserInput(AnimationInspectorState this __attribute__ ((unused)), UserInput userInput)
{
	ASSERT(this, "AnimationInspectorState::processUserInput: null this");

	AnimationInspector::processUserInput(AnimationInspector::getInstance(), userInput.pressedKey);
}
