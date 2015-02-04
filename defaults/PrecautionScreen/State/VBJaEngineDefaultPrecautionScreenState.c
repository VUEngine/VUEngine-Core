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
#include <VBJaEngineDefaultPrecautionScreenState.h>
#include <VBJaEngineDefaultAutomaticPauseSelectionScreenState.h>

extern StageROMDef VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEngineDefaultPrecautionScreenState_destructor(VBJaEngineDefaultPrecautionScreenState this);
static void VBJaEngineDefaultPrecautionScreenState_constructor(VBJaEngineDefaultPrecautionScreenState this);
static void VBJaEngineDefaultPrecautionScreenState_enter(VBJaEngineDefaultPrecautionScreenState this, void* owner);
static void VBJaEngineDefaultPrecautionScreenState_execute(VBJaEngineDefaultPrecautionScreenState this, void* owner);
static void VBJaEngineDefaultPrecautionScreenState_exit(VBJaEngineDefaultPrecautionScreenState this, void* owner);
static void VBJaEngineDefaultPrecautionScreenState_resume(VBJaEngineDefaultPrecautionScreenState this, void* owner);
static bool VBJaEngineDefaultPrecautionScreenState_handleMessage(VBJaEngineDefaultPrecautionScreenState this, void* owner, Telegram telegram);
static void VBJaEngineDefaultPrecautionScreenState_processInput(VBJaEngineDefaultPrecautionScreenState this, u16 pressedKey);
static void VBJaEngineDefaultPrecautionScreenState_print(VBJaEngineDefaultPrecautionScreenState this);
void VBJaEngineDefaultPrecautionScreenState_setNextstate(VBJaEngineDefaultPrecautionScreenState this, GameState nextState);
void VBJaEngineDefaultPrecautionScreenState_setPrecautionString(VBJaEngineDefaultPrecautionScreenState this, char* string);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEngineDefaultPrecautionScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEngineDefaultPrecautionScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEngineDefaultPrecautionScreenState_constructor(VBJaEngineDefaultPrecautionScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEngineDefaultPrecautionScreenState_setNextstate(this, __UPCAST(GameState, VBJaEngineDefaultAutomaticPauseSelectionScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST;
    this->precautionString = "           Important:\n\nRead Instruction and Precaution\n\n   Booklets before operating";
}

// class's destructor
static void VBJaEngineDefaultPrecautionScreenState_destructor(VBJaEngineDefaultPrecautionScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEngineDefaultPrecautionScreenState_enter(VBJaEngineDefaultPrecautionScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, true, true);

    VBJaEngineDefaultPrecautionScreenState_print(this);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEngineDefaultPrecautionScreenState_execute(VBJaEngineDefaultPrecautionScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaEngineDefaultPrecautionScreenState_exit(VBJaEngineDefaultPrecautionScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEngineDefaultPrecautionScreenState_resume(VBJaEngineDefaultPrecautionScreenState this, void* owner)
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

    VBJaEngineDefaultPrecautionScreenState_print(this);
}

// state's on message
static bool VBJaEngineDefaultPrecautionScreenState_handleMessage(VBJaEngineDefaultPrecautionScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEngineDefaultPrecautionScreenState_processInput(VBJaEngineDefaultPrecautionScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEngineDefaultPrecautionScreenState_processInput(VBJaEngineDefaultPrecautionScreenState this, u16 pressedKey)
{
	Game_changeState(Game_getInstance(), this->nextState);
}

static void VBJaEngineDefaultPrecautionScreenState_print(VBJaEngineDefaultPrecautionScreenState this)
{
    Printing_text(Printing_getInstance(), this->precautionString, 8, 6, NULL);
}

void VBJaEngineDefaultPrecautionScreenState_setNextstate(VBJaEngineDefaultPrecautionScreenState this, GameState nextState)
{
	this->nextState = nextState;
}

void VBJaEngineDefaultPrecautionScreenState_setPrecautionString(VBJaEngineDefaultPrecautionScreenState this, char* string)
{
    this->precautionString = string;
}