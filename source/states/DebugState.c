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

#ifdef __DEBUG_TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DebugState.h>
#include <Debug.h>
#include <Game.h>
#include <Telegram.h>
#include <KeyPadManager.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void DebugState_destructor(DebugState this);
static void DebugState_constructor(DebugState this);
static void DebugState_enter(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void DebugState_execute(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static void DebugState_exit(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)));
static bool DebugState_processMessage(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)), Telegram telegram);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define DebugState_ATTRIBUTES																			\
		/* inherits */																					\
		GameState_ATTRIBUTES																			\

/**
 * @class	DebugState
 * @extends GameState
 * @ingroup states
 */
__CLASS_DEFINITION(DebugState, GameState);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			DebugState_getInstance()
 * @memberof	DebugState
 * @public
 *
 * @return		DebugState instance
 */
__SINGLETON(DebugState);

/**
 * Class constructor
 *
 * @memberof	DebugState
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) DebugState_constructor(DebugState this)
{
	__CONSTRUCT_BASE(GameState);
}

/**
 * Class destructor
 *
 * @memberof	DebugState
 * @private
 *
 * @param this	Function scope
 */
static void DebugState_destructor(DebugState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Method called when the Game's StateMachine enters to this state
 *
 * @memberof		DebugState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void DebugState_enter(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	GameState_pauseClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
	Debug_show(Debug_getInstance(), __SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		DebugState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void DebugState_execute(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	Debug_update(Debug_getInstance());
}

/**
 * Method called when the Game's StateMachine exits from this state
 *
 * @memberof		DebugState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
static void DebugState_exit(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	Debug_hide(Debug_getInstance());
	GameState_resumeClocks(__SAFE_CAST(GameState, StateMachine_getPreviousState(Game_getStateMachine(Game_getInstance()))));
}

/**
 * Method called when the Game's StateMachine receives a message to be processed
 *
 * @memberof		DebugState
 * @private
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 *
 * @return 			True if no further processing of the message is required
 */
static bool DebugState_processMessage(DebugState this __attribute__ ((unused)), void* owner __attribute__ ((unused)), Telegram telegram)
{
	// process message
	switch(Telegram_getMessage(telegram))
	{
		case kKeyPressed:
			{
				u32 pressedKey = *((u32*)Telegram_getExtraInfo(telegram));

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
					Debug_displaceLeft(Debug_getInstance());
				}
				else if(pressedKey & K_RR)
				{
					Debug_displaceRight(Debug_getInstance());
				}
				else if(pressedKey & K_RU)
				{
					Debug_displaceUp(Debug_getInstance());
				}
				else if(pressedKey & K_RD)
				{
					Debug_displaceDown(Debug_getInstance());
				}
			}
			break;
	}

	return true;
}


#endif
