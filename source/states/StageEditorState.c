/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void StageEditorState_destructor(StageEditorState this);
static void StageEditorState_constructor(StageEditorState this);
static void StageEditorState_enter(StageEditorState this, void* owner);
static void StageEditorState_execute(StageEditorState this, void* owner);
static void StageEditorState_exit(StageEditorState this, void* owner);
static void StageEditorState_onUserInput(StageEditorState this, Object eventFirer);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define StageEditorState_ATTRIBUTES																		\
		GameState_ATTRIBUTES																			\

/**
 * @class	StageEditorState
 * @extends GameState
 * @ingroup states
 */
__CLASS_DEFINITION(StageEditorState, GameState);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StageEditorState_getInstance()
 * @memberof	StageEditorState
 * @public
 *
 * @return		StageEditorState instance
 */
__SINGLETON(StageEditorState);

/**
 * Class constructor
 *
 * @memberof	StageEditorState
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) StageEditorState_constructor(StageEditorState this)
{
	__CONSTRUCT_BASE(GameState);
}

/**
 * Class destructor
 *
 * @memberof	StageEditorState
 * @private
 *
 * @param this	Function scope
 */
static void StageEditorState_destructor(StageEditorState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @memberof		StageEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void StageEditorState_enter(StageEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	GameState_pauseClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	StageEditor_show(StageEditor_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	Object_addEventListener(__SAFE_CAST(Object, Game_getInstance()), __SAFE_CAST(Object, this), (EventListener)StageEditorState_onUserInput, kEventUserInput);
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		StageEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void StageEditorState_execute(StageEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	StageEditor_update(StageEditor_getInstance());
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @memberof		StageEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void StageEditorState_exit(StageEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	Object_removeEventListener(__SAFE_CAST(Object, Game_getInstance()), __SAFE_CAST(Object, this), (EventListener)StageEditorState_onUserInput, kEventUserInput);
	StageEditor_hide(StageEditor_getInstance());
	GameState_resumeClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

/**
 * Process user input
 *
 * @memberof			StageEditorState
 * @private
 *
 * @param this			Function scope
 * @param eventFirer	KeypadManager
 */
static void StageEditorState_onUserInput(StageEditorState this __attribute__ ((unused)), Object eventFirer __attribute__ ((unused)))
{
	StageEditor_processUserInput(StageEditor_getInstance(), KeypadManager_getUserInput(KeypadManager_getInstance()).pressedKey);
}
