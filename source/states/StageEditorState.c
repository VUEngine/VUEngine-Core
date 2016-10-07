/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifdef __STAGE_EDITOR


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StageEditorState.h>
#include <StageEditor.h>
#include <Game.h>
#include <MessageDispatcher.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void StageEditorState_destructor(StageEditorState this);
static void StageEditorState_constructor(StageEditorState this);
static void StageEditorState_enter(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)));
static void StageEditorState_execute(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)));
static void StageEditorState_exit(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)));
static bool StageEditorState_processMessage(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)), Telegram telegram);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define StageEditorState_ATTRIBUTES																		\
        /* inherits */																					\
        GameState_ATTRIBUTES																			\

__CLASS_DEFINITION(StageEditorState, GameState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(StageEditorState);

// class's constructor
static void __attribute__ ((noinline)) StageEditorState_constructor(StageEditorState this)
{
	__CONSTRUCT_BASE(GameState);
}

// class's destructor
static void StageEditorState_destructor(StageEditorState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void StageEditorState_enter(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)))
{
	GameState_pauseClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	StageEditor_start(StageEditor_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's execute
static void StageEditorState_execute(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)))
{
	StageEditor_update(StageEditor_getInstance());
}

// state's exit
static void StageEditorState_exit(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)))
{
	StageEditor_stop(StageEditor_getInstance());
	GameState_resumeClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's on message
static bool StageEditorState_processMessage(StageEditorState this __attribute__ ((unused)), void* owner  __attribute__ ((unused)), Telegram telegram)
{
	// process message
	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:
			{
				MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, StageEditor_getInstance()), kKeyPressed, ((u16*)Telegram_getExtraInfo(telegram)));
			}
			break;
	}

	return true;
}


#endif
