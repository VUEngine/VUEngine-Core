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

#ifdef __DEBUG_TOOLS

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DebugState.h>
#include <Debug.h>
#include <Game.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void DebugState_destructor(DebugState this);
static void DebugState_constructor(DebugState this);
static void DebugState_enter(DebugState this, void* owner);
static void DebugState_execute(DebugState this, void* owner);
static void DebugState_exit(DebugState this, void* owner);
static bool DebugState_handleMessage(DebugState this, void* owner, Telegram telegram);


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern State __CONCAT(START_LEVEL, _getInstance)();

enum Screens
{
	kPvbScreen = 0,
	kPrecautionScreen,
	kVbJaeScreen,
	kDebugExitScreen
};


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define DebugState_ATTRIBUTES														\
																					\
	/* inherits */																	\
	GameState_ATTRIBUTES															\

__CLASS_DEFINITION(DebugState, GameState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(DebugState);

// class's constructor
static void DebugState_constructor(DebugState this)
{
	__CONSTRUCT_BASE();
}

// class's destructor
static void DebugState_destructor(DebugState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void DebugState_enter(DebugState this, void* owner)
{
	Clock_pause(Game_getInGameClock(Game_getInstance()), true);
	Debug_show(Debug_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

// state's execute
static void DebugState_execute(DebugState this, void* owner)
{
	Debug_update(Debug_getInstance());
}

// state's exit
static void DebugState_exit(DebugState this, void* owner)
{
	Debug_hide(Debug_getInstance());
	Clock_pause(Game_getInGameClock(Game_getInstance()), false);
}

// state's on message
static bool DebugState_handleMessage(DebugState this, void* owner, Telegram telegram)
{
	// process message
	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:
			{
				u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

				if(pressedKey & K_LL)
				{
					Debug_showPreviousPage(Debug_getInstance());
				}
				else if(pressedKey & K_LR)
				{
					Debug_showNextPage(Debug_getInstance());
				}
				else if(pressedKey & K_LU)
				{
					Debug_showPreviousSubPage(Debug_getInstance());
				}
				else if(pressedKey & K_LD)
				{
					Debug_showNextSubPage(Debug_getInstance());
				}
				else if(pressedKey & K_RL)
				{
					Debug_diplaceLeft(Debug_getInstance());
				}
				else if(pressedKey & K_RR)
				{
					Debug_diplaceRight(Debug_getInstance());
				}
				else if(pressedKey & K_RU)
				{
					Debug_diplaceUp(Debug_getInstance());
				}
				else if(pressedKey & K_RD)
				{
					Debug_diplaceDown(Debug_getInstance());
				}
			}
			break;
	}

	return true;
}


#endif