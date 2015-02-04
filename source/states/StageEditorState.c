/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
static void StageEditorState_enter(StageEditorState this, void* owner);
static void StageEditorState_execute(StageEditorState this, void* owner);
static void StageEditorState_exit(StageEditorState this, void* owner);
static bool StageEditorState_handleMessage(StageEditorState this, void* owner, Telegram telegram);


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern State __CONCAT(START_LEVEL, _getInstance)();

enum Screens
{
	kPvbScreen = 0,
	kPrecautionScreen,
	kVbJaeScreen,
	kStageEditorExitScreen
};


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define StageEditorState_ATTRIBUTES													\
																					\
	/* inherits */																	\
	GameState_ATTRIBUTES															\

__CLASS_DEFINITION(StageEditorState, GameState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(StageEditorState);

// class's constructor
static void StageEditorState_constructor(StageEditorState this)
{
	__CONSTRUCT_BASE();
}

// class's destructor
static void StageEditorState_destructor(StageEditorState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void StageEditorState_enter(StageEditorState this, void* owner)
{
	Clock_pause(Game_getInGameClock(Game_getInstance()), true);

	StageEditor_start(StageEditor_getInstance(), __UPCAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's execute
static void StageEditorState_execute(StageEditorState this, void* owner)
{
	StageEditor_update(StageEditor_getInstance());
}

// state's exit
static void StageEditorState_exit(StageEditorState this, void* owner)
{
	StageEditor_stop(StageEditor_getInstance());
	Clock_pause(Game_getInGameClock(Game_getInstance()), false);
}

// state's on message
static bool StageEditorState_handleMessage(StageEditorState this, void* owner, Telegram telegram)
{
	// process message
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
			{
				MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, StageEditor_getInstance()), kKeyPressed, ((u16*)Telegram_getExtraInfo(telegram)));
			}
			break;
	}

	return true;
}


#endif