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
#include <Printing.h>
#include <MessageDispatcher.h>
#include <VBJaEAutoPauseSelectScreenState.h>
#include <VBJaEAutoPauseScreenState.h>
#include <VBJaELangSelectScreenState.h>

extern StageROMDef EMPTY_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEAutoPauseSelectScreenState_destructor(VBJaEAutoPauseSelectScreenState this);
static void VBJaEAutoPauseSelectScreenState_constructor(VBJaEAutoPauseSelectScreenState this);
static void VBJaEAutoPauseSelectScreenState_enter(VBJaEAutoPauseSelectScreenState this, void* owner);
static void VBJaEAutoPauseSelectScreenState_execute(VBJaEAutoPauseSelectScreenState this, void* owner);
static void VBJaEAutoPauseSelectScreenState_exit(VBJaEAutoPauseSelectScreenState this, void* owner);
static void VBJaEAutoPauseSelectScreenState_resume(VBJaEAutoPauseSelectScreenState this, void* owner);
static bool VBJaEAutoPauseSelectScreenState_handleMessage(VBJaEAutoPauseSelectScreenState this, void* owner, Telegram telegram);
static void VBJaEAutoPauseSelectScreenState_print(VBJaEAutoPauseSelectScreenState this);
static void VBJaEAutoPauseSelectScreenState_renderSelection(VBJaEAutoPauseSelectScreenState this);
static void VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState this, u16 pressedKey);
void VBJaEAutoPauseSelectScreenState_setExplanationString(VBJaEAutoPauseSelectScreenState this, char* string);
void VBJaEAutoPauseSelectScreenState_setTitleString(VBJaEAutoPauseSelectScreenState this, char* string);
void VBJaEAutoPauseSelectScreenState_setOnString(VBJaEAutoPauseSelectScreenState this, char* string);
void VBJaEAutoPauseSelectScreenState_setOffString(VBJaEAutoPauseSelectScreenState this, char* string);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEAutoPauseSelectScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEAutoPauseSelectScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEAutoPauseSelectScreenState_constructor(VBJaEAutoPauseSelectScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEAutoPauseSelectScreenState_setNextstate(this, __UPCAST(GameState, VBJaELangSelectScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&EMPTY_ST;
    this->selection = true;
    this->titleString = "Automatic Pause";
    this->explanationString = "The Automatic Pause feature will\nremind you to take a break from\nplaying approx. every 30 minutes";
    this->onString = "On";
    this->offString = "Off";
}

// class's destructor
static void VBJaEAutoPauseSelectScreenState_destructor(VBJaEAutoPauseSelectScreenState this)
{	
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEAutoPauseSelectScreenState_enter(VBJaEAutoPauseSelectScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, false);

	VBJaEAutoPauseSelectScreenState_print(this);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEAutoPauseSelectScreenState_execute(VBJaEAutoPauseSelectScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaEAutoPauseSelectScreenState_exit(VBJaEAutoPauseSelectScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEAutoPauseSelectScreenState_resume(VBJaEAutoPauseSelectScreenState this, void* owner)
{
	GameState_resume(__UPCAST(GameState, this), owner);
	
	VBJaEAutoPauseSelectScreenState_print(this);
}

// state's on message
static bool VBJaEAutoPauseSelectScreenState_handleMessage(VBJaEAutoPauseSelectScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEAutoPauseSelectScreenState_print(VBJaEAutoPauseSelectScreenState this)
{
    u8 strHeaderXPos = (48 - strlen(this->titleString)) >> 1;
    Printing_text(Printing_getInstance(), this->titleString, strHeaderXPos, 8, NULL);

    Printing_text(Printing_getInstance(), this->explanationString, 8, 11, NULL);

    VBJaEAutoPauseSelectScreenState_renderSelection(this);
}

static void VBJaEAutoPauseSelectScreenState_renderSelection(VBJaEAutoPauseSelectScreenState this)
{
    // get strings and determine lengths
    u8 strOnLength = strlen(this->onString);
    u8 strOffLength = strlen(this->offString);
    u8 optionsGap = 3;
    u8 selectionStart = (48 - (strOnLength + optionsGap + strOffLength)) >> 1;

    // clear options area
    Printing_text(Printing_getInstance(), "                                                ", 0, 16, NULL);
    Printing_text(Printing_getInstance(), "                                                ", 0, 17, NULL);
    Printing_text(Printing_getInstance(), "                                                ", 0, 18, NULL);

    // print options
    Printing_text(Printing_getInstance(), this->onString, selectionStart, 17, NULL);
    Printing_text(Printing_getInstance(), this->offString, selectionStart + 3 + strOnLength, 17, NULL);

    // print selector
    u8 optionStart = this->selection ? selectionStart - 1 : selectionStart - 1 + optionsGap + strOnLength;
    u8 optionEnd = this->selection ? optionStart + 1 + strOnLength : optionStart + 1 + strOffLength;
    Printing_text(Printing_getInstance(), "\x02\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07", optionStart, 16, NULL);
    Printing_text(Printing_getInstance(), "\x03               ", optionEnd, 16, NULL);
    Printing_text(Printing_getInstance(), "\x06", optionStart, 17, NULL);
    Printing_text(Printing_getInstance(), "\x06", optionEnd, 17, NULL);
    Printing_text(Printing_getInstance(), "\x04\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07", optionStart, 18, NULL);
    Printing_text(Printing_getInstance(), "\x05               ", optionEnd, 18, NULL);
}

static void VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState this, u16 pressedKey)
{
	if ((pressedKey & K_LL) || (pressedKey & K_LR))
	{
	    this->selection = !this->selection;
	    VBJaEAutoPauseSelectScreenState_renderSelection(this);
	}
	else if ((pressedKey & K_A) || (pressedKey & K_STA))
	{
		Game_setAutomaticPauseState(Game_getInstance(), this->selection ? __UPCAST(GameState, VBJaEAutoPauseScreenState_getInstance()): NULL);
	    Game_changeState(Game_getInstance(), this->nextState);
	}
}

void VBJaEAutoPauseSelectScreenState_setNextstate(VBJaEAutoPauseSelectScreenState this, GameState nextState)
{
    this->nextState = nextState;
}

void VBJaEAutoPauseSelectScreenState_setExplanationString(VBJaEAutoPauseSelectScreenState this, char* string)
{
    this->explanationString = string;
}

void VBJaEAutoPauseSelectScreenState_setTitleString(VBJaEAutoPauseSelectScreenState this, char* string)
{
    this->titleString = string;
}

void VBJaEAutoPauseSelectScreenState_setOnString(VBJaEAutoPauseSelectScreenState this, char* string)
{
    this->onString = string;
}

void VBJaEAutoPauseSelectScreenState_setOffString(VBJaEAutoPauseSelectScreenState this, char* string)
{
    this->offString = string;
}