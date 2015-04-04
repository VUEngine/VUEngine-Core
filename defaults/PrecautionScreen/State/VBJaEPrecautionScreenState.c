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
#include <Printing.h>
#include <Screen.h>
#include <MessageDispatcher.h>
#include <VBJaEPrecautionScreenState.h>
#include <VBJaEAdjustmentScreenState.h>

extern StageROMDef VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEPrecautionScreenState_destructor(VBJaEPrecautionScreenState this);
static void VBJaEPrecautionScreenState_constructor(VBJaEPrecautionScreenState this);
static void VBJaEPrecautionScreenState_enter(VBJaEPrecautionScreenState this, void* owner);
static void VBJaEPrecautionScreenState_execute(VBJaEPrecautionScreenState this, void* owner);
static void VBJaEPrecautionScreenState_exit(VBJaEPrecautionScreenState this, void* owner);
static void VBJaEPrecautionScreenState_resume(VBJaEPrecautionScreenState this, void* owner);
static bool VBJaEPrecautionScreenState_handleMessage(VBJaEPrecautionScreenState this, void* owner, Telegram telegram);
static void VBJaEPrecautionScreenState_processInput(VBJaEPrecautionScreenState this, u16 pressedKey);
static void VBJaEPrecautionScreenState_print(VBJaEPrecautionScreenState this);
void VBJaEPrecautionScreenState_setNextstate(VBJaEPrecautionScreenState this, GameState nextState);
void VBJaEPrecautionScreenState_setPrecautionString(VBJaEPrecautionScreenState this, char* string);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEPrecautionScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEPrecautionScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEPrecautionScreenState_constructor(VBJaEPrecautionScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEPrecautionScreenState_setNextstate(this, __UPCAST(GameState, VBJaEAdjustmentScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST;
    this->precautionString = "     IMPORTANT:\n\nREAD INSTRUCTION AND\n\nPRECAUTION BOOKLETS\n\n  BEFORE OPERATING";
}

// class's destructor
static void VBJaEPrecautionScreenState_destructor(VBJaEPrecautionScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEPrecautionScreenState_enter(VBJaEPrecautionScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, false);

    VBJaEPrecautionScreenState_print(this);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEPrecautionScreenState_execute(VBJaEPrecautionScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaEPrecautionScreenState_exit(VBJaEPrecautionScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEPrecautionScreenState_resume(VBJaEPrecautionScreenState this, void* owner)
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

    VBJaEPrecautionScreenState_print(this);
}

// state's on message
static bool VBJaEPrecautionScreenState_handleMessage(VBJaEPrecautionScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEPrecautionScreenState_processInput(VBJaEPrecautionScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEPrecautionScreenState_processInput(VBJaEPrecautionScreenState this, u16 pressedKey)
{
	Game_changeState(Game_getInstance(), this->nextState);
}

static void VBJaEPrecautionScreenState_print(VBJaEPrecautionScreenState this)
{
    Printing_text(Printing_getInstance(), this->precautionString, 14, 6, NULL);
}

void VBJaEPrecautionScreenState_setNextstate(VBJaEPrecautionScreenState this, GameState nextState)
{
	this->nextState = nextState;
}

void VBJaEPrecautionScreenState_setPrecautionString(VBJaEPrecautionScreenState this, char* string)
{
    this->precautionString = string;
}