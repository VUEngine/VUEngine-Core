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

#include <string.h>

#include <Game.h>
#include <Screen.h>
#include <MessageDispatcher.h>
#include <VBJaESplashScreenState.h>

extern StageROMDef VBJAENGINE_SPLASH_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaESplashScreenState_destructor(VBJaESplashScreenState this);
static void VBJaESplashScreenState_constructor(VBJaESplashScreenState this);
static void VBJaESplashScreenState_enter(VBJaESplashScreenState this, void* owner);
static void VBJaESplashScreenState_execute(VBJaESplashScreenState this, void* owner);
static void VBJaESplashScreenState_exit(VBJaESplashScreenState this, void* owner);
static void VBJaESplashScreenState_resume(VBJaESplashScreenState this, void* owner);
static bool VBJaESplashScreenState_handleMessage(VBJaESplashScreenState this, void* owner, Telegram telegram);
static void VBJaESplashScreenState_processInput(VBJaESplashScreenState this, u16 pressedKey);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaESplashScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaESplashScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaESplashScreenState_constructor(VBJaESplashScreenState this)
{
	__CONSTRUCT_BASE();

	this->stageDefinition = (StageDefinition*)&VBJAENGINE_SPLASH_SCREEN_ST;
}

// class's destructor
static void VBJaESplashScreenState_destructor(VBJaESplashScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaESplashScreenState_enter(VBJaESplashScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, false);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaESplashScreenState_execute(VBJaESplashScreenState this, void* owner)
{
    VBVec3D translation =
    {
        ITOFIX19_13(1),
        ITOFIX19_13(0),
        ITOFIX19_13(0)
    };

    Screen_move(Screen_getInstance(), translation, false);

 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaESplashScreenState_exit(VBJaESplashScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaESplashScreenState_resume(VBJaESplashScreenState this, void* owner)
{
	GameState_resume(__UPCAST(GameState, this), owner);
	
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
	Screen_FXFadeIn(Screen_getInstance(), 16 >> 1);

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
static bool VBJaESplashScreenState_handleMessage(VBJaESplashScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaESplashScreenState_processInput(VBJaESplashScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaESplashScreenState_processInput(VBJaESplashScreenState this, u16 pressedKey)
{
	Game_changeState(Game_getInstance(), __UPCAST(GameState, this->nextState));
}

void VBJaESplashScreenState_setNextstate(VBJaESplashScreenState this, GameState nextState)
{
    this->nextState = nextState;
}