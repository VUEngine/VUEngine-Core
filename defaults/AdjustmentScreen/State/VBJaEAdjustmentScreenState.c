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
#include <VBJaEAdjustmentScreenState.h>
#include <VBJaEPrecautionScreenState.h>

extern StageROMDef VBJAENGINE_DEFAULT_ADJUSTMENT_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEAdjustmentScreenState_destructor(VBJaEAdjustmentScreenState this);
static void VBJaEAdjustmentScreenState_constructor(VBJaEAdjustmentScreenState this);
static void VBJaEAdjustmentScreenState_enter(VBJaEAdjustmentScreenState this, void* owner);
static void VBJaEAdjustmentScreenState_execute(VBJaEAdjustmentScreenState this, void* owner);
static void VBJaEAdjustmentScreenState_exit(VBJaEAdjustmentScreenState this, void* owner);
static void VBJaEAdjustmentScreenState_resume(VBJaEAdjustmentScreenState this, void* owner);
static bool VBJaEAdjustmentScreenState_handleMessage(VBJaEAdjustmentScreenState this, void* owner, Telegram telegram);
static void VBJaEAdjustmentScreenState_processInput(VBJaEAdjustmentScreenState this, u16 pressedKey);
void VBJaEAdjustmentScreenState_setNextstate(VBJaEAdjustmentScreenState this, GameState nextState);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEAdjustmentScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEAdjustmentScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEAdjustmentScreenState_constructor(VBJaEAdjustmentScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEAdjustmentScreenState_setNextstate(this, __UPCAST(GameState, VBJaEPrecautionScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_DEFAULT_ADJUSTMENT_SCREEN_ST;
}

// class's destructor
static void VBJaEAdjustmentScreenState_destructor(VBJaEAdjustmentScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEAdjustmentScreenState_enter(VBJaEAdjustmentScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, true, true);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEAdjustmentScreenState_execute(VBJaEAdjustmentScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaEAdjustmentScreenState_exit(VBJaEAdjustmentScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEAdjustmentScreenState_resume(VBJaEAdjustmentScreenState this, void* owner)
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
static bool VBJaEAdjustmentScreenState_handleMessage(VBJaEAdjustmentScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEAdjustmentScreenState_processInput(VBJaEAdjustmentScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEAdjustmentScreenState_processInput(VBJaEAdjustmentScreenState this, u16 pressedKey)
{
	Game_changeState(Game_getInstance(), this->nextState);
}

void VBJaEAdjustmentScreenState_setNextstate(VBJaEAdjustmentScreenState this, GameState nextState)
{
	this->nextState = nextState;
}
