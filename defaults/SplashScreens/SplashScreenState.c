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

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Game.h>
#include <Screen.h>
#include <MessageDispatcher.h>
#include <SplashScreenState.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(SplashScreenState, GameState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void SplashScreenState_constructor(SplashScreenState this)
{
	__CONSTRUCT_BASE();

	this->stageDefinition = NULL;
}

// class's destructor
void SplashScreenState_destructor(SplashScreenState this)
{
	// destroy the super object
	__DESTROY_BASE;
}

// state's enter
void SplashScreenState_enter(SplashScreenState this, void* owner)
{
	if(this->stageDefinition)
	{
		GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, NULL);
	}

    __VIRTUAL_CALL(void, SplashScreenState, print, this);
	
    Screen_startEffect(Screen_getInstance(), kFadeIn, 16);
}

// state's execute
void SplashScreenState_execute(SplashScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
void SplashScreenState_exit(SplashScreenState this, void* owner)
{
	Screen_startEffect(Screen_getInstance(), kFadeOut, 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
void SplashScreenState_resume(SplashScreenState this, void* owner)
{
	GameState_resume(__UPCAST(GameState, this), owner);

	__VIRTUAL_CALL(void, SplashScreenState, print, this);
	
#ifdef __DEBUG_TOOLS
	if (!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __STAGE_EDITOR
	if (!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __ANIMATION_EDITOR
	if (!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif

	// make a fade in
	    Screen_startEffect(Screen_getInstance(), kFadeIn, 16);

#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_EDITOR
	}
#endif
}

// state's on message
bool SplashScreenState_handleMessage(SplashScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            __VIRTUAL_CALL(void, SplashScreenState, processInput, this, pressedKey);
        }
        break;
	}

	return false;
}

void SplashScreenState_processInput(SplashScreenState this, u16 releasedKey)
{
	Game_changeState(Game_getInstance(), this->nextState);
}

void SplashScreenState_print(SplashScreenState this)
{
}

void SplashScreenState_setNextstate(SplashScreenState this, GameState nextState)
{
    this->nextState = nextState;
}