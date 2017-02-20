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

#include <AnimationEditorState.h>
#include <AnimationEditor.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>
#include <KeypadManager.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimationEditorState_destructor(AnimationEditorState this);
static void AnimationEditorState_constructor(AnimationEditorState this);
static void AnimationEditorState_enter(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void AnimationEditorState_execute(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void AnimationEditorState_exit(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void AnimationEditorState_onUserInput(AnimationEditorState this __attribute__ ((unused)), Object eventFirer __attribute__ ((unused)));


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define AnimationEditorState_ATTRIBUTES																	\
		GameState_ATTRIBUTES																			\

/**
 * @class	AnimationEditorState
 * @extends GameState
 * @ingroup states
 */
__CLASS_DEFINITION(AnimationEditorState, GameState);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			AnimationEditorState_getInstance()
 * @memberof	AnimationEditorState
 * @public
 *
 * @return		AnimationEditorState instance
 */
__SINGLETON(AnimationEditorState);

/**
 * Class constructor
 *
 * @memberof	AnimationEditorState
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) AnimationEditorState_constructor(AnimationEditorState this)
{
	__CONSTRUCT_BASE(GameState);
}

/**
 * Class destructor
 *
 * @memberof	AnimationEditorState
 * @private
 *
 * @param this	Function scope
 */
static void AnimationEditorState_destructor(AnimationEditorState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @memberof		AnimationEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void AnimationEditorState_enter(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	GameState_pauseClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	AnimationEditor_show(AnimationEditor_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	Object_addEventListener(__SAFE_CAST(Object, Game_getInstance()), __SAFE_CAST(Object, this), (EventListener)AnimationEditorState_onUserInput, kEventUserInput);
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		AnimationEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void AnimationEditorState_execute(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	AnimationEditor_update(AnimationEditor_getInstance());
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @memberof		AnimationEditorState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void AnimationEditorState_exit(AnimationEditorState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	Object_removeEventListener(__SAFE_CAST(Object, Game_getInstance()), __SAFE_CAST(Object, this), (EventListener)AnimationEditorState_onUserInput, kEventUserInput);
	AnimationEditor_hide(AnimationEditor_getInstance());
	GameState_resumeClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

/**
 * Process user input
 *
 * @memberof			AnimationEditorState
 * @private
 *
 * @param this			Function scope
 * @param eventFirer	KeypadManager
 */
static void AnimationEditorState_onUserInput(AnimationEditorState this __attribute__ ((unused)), Object eventFirer __attribute__ ((unused)))
{
	AnimationEditor_processUserInput(AnimationEditor_getInstance(), KeypadManager_getUserInput(KeypadManager_getInstance()).pressedKey);
}
