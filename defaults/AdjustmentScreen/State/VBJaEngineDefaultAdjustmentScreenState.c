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
#include <VBJaEngineDefaultAdjustmentScreenState.h>
#include <VBJaEngineDefaultPrecautionScreenState.h>

extern StageROMDef VBJAENGINE_DEFAULT_ADJUSTMENT_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEngineDefaultAdjustmentScreenState_destructor(VBJaEngineDefaultAdjustmentScreenState this);
static void VBJaEngineDefaultAdjustmentScreenState_constructor(VBJaEngineDefaultAdjustmentScreenState this);
static void VBJaEngineDefaultAdjustmentScreenState_enter(VBJaEngineDefaultAdjustmentScreenState this, void* owner);
static void VBJaEngineDefaultAdjustmentScreenState_execute(VBJaEngineDefaultAdjustmentScreenState this, void* owner);
static void VBJaEngineDefaultAdjustmentScreenState_exit(VBJaEngineDefaultAdjustmentScreenState this, void* owner);
static void VBJaEngineDefaultAdjustmentScreenState_resume(VBJaEngineDefaultAdjustmentScreenState this, void* owner);
static bool VBJaEngineDefaultAdjustmentScreenState_handleMessage(VBJaEngineDefaultAdjustmentScreenState this, void* owner, Telegram telegram);
static void VBJaEngineDefaultAdjustmentScreenState_processInput(VBJaEngineDefaultAdjustmentScreenState this, u16 pressedKey);
void VBJaEngineDefaultAdjustmentScreenState_setNextstate(VBJaEngineDefaultAdjustmentScreenState this, GameState nextState);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEngineDefaultAdjustmentScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEngineDefaultAdjustmentScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEngineDefaultAdjustmentScreenState_constructor(VBJaEngineDefaultAdjustmentScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEngineDefaultAdjustmentScreenState_setNextstate(this, (GameState)VBJaEngineDefaultPrecautionScreenState_getInstance());
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_DEFAULT_ADJUSTMENT_SCREEN_ST;
}

// class's destructor
static void VBJaEngineDefaultAdjustmentScreenState_destructor(VBJaEngineDefaultAdjustmentScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEngineDefaultAdjustmentScreenState_enter(VBJaEngineDefaultAdjustmentScreenState this, void* owner)
{
	GameState_loadStage((GameState)this, this->stageDefinition, true, true);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEngineDefaultAdjustmentScreenState_execute(VBJaEngineDefaultAdjustmentScreenState this, void* owner)
{
 	// call base
	GameState_execute((GameState)this, owner);
}

// state's exit
static void VBJaEngineDefaultAdjustmentScreenState_exit(VBJaEngineDefaultAdjustmentScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEngineDefaultAdjustmentScreenState_resume(VBJaEngineDefaultAdjustmentScreenState this, void* owner)
{
	GameState_resume((GameState)this, owner);
	
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
static bool VBJaEngineDefaultAdjustmentScreenState_handleMessage(VBJaEngineDefaultAdjustmentScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEngineDefaultAdjustmentScreenState_processInput(VBJaEngineDefaultAdjustmentScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEngineDefaultAdjustmentScreenState_processInput(VBJaEngineDefaultAdjustmentScreenState this, u16 pressedKey)
{
	Game_changeState(Game_getInstance(), this->nextState);
}

void VBJaEngineDefaultAdjustmentScreenState_setNextstate(VBJaEngineDefaultAdjustmentScreenState this, GameState nextState)
{
	this->nextState = nextState;
}
