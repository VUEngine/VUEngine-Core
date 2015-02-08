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
#include <VBJaEAutomaticPauseSelectionScreenState.h>
#include <VBJaEAutomaticPauseScreenState.h>
#include <VBJaELanguageSelectionScreenState.h>

extern StageROMDef EMPTY_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEAutomaticPauseSelectionScreenState_destructor(VBJaEAutomaticPauseSelectionScreenState this);
static void VBJaEAutomaticPauseSelectionScreenState_constructor(VBJaEAutomaticPauseSelectionScreenState this);
static void VBJaEAutomaticPauseSelectionScreenState_enter(VBJaEAutomaticPauseSelectionScreenState this, void* owner);
static void VBJaEAutomaticPauseSelectionScreenState_execute(VBJaEAutomaticPauseSelectionScreenState this, void* owner);
static void VBJaEAutomaticPauseSelectionScreenState_exit(VBJaEAutomaticPauseSelectionScreenState this, void* owner);
static void VBJaEAutomaticPauseSelectionScreenState_resume(VBJaEAutomaticPauseSelectionScreenState this, void* owner);
static bool VBJaEAutomaticPauseSelectionScreenState_handleMessage(VBJaEAutomaticPauseSelectionScreenState this, void* owner, Telegram telegram);
static void VBJaEAutomaticPauseSelectionScreenState_print(VBJaEAutomaticPauseSelectionScreenState this);
static void VBJaEAutomaticPauseSelectionScreenState_renderSelection(VBJaEAutomaticPauseSelectionScreenState this);
static void VBJaEAutomaticPauseSelectionScreenState_processInput(VBJaEAutomaticPauseSelectionScreenState this, u16 pressedKey);
void VBJaEAutomaticPauseSelectionScreenState_setExplanationString(VBJaEAutomaticPauseSelectionScreenState this, char* string);
void VBJaEAutomaticPauseSelectionScreenState_setTitleString(VBJaEAutomaticPauseSelectionScreenState this, char* string);
void VBJaEAutomaticPauseSelectionScreenState_setOnString(VBJaEAutomaticPauseSelectionScreenState this, char* string);
void VBJaEAutomaticPauseSelectionScreenState_setOffString(VBJaEAutomaticPauseSelectionScreenState this, char* string);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEAutomaticPauseSelectionScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaEAutomaticPauseSelectionScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEAutomaticPauseSelectionScreenState_constructor(VBJaEAutomaticPauseSelectionScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaEAutomaticPauseSelectionScreenState_setNextstate(this, __UPCAST(GameState, VBJaELanguageSelectionScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&EMPTY_ST;
    this->selection = true;
    this->titleString = "Automatic Pause";
    this->explanationString = "The Automatic Pause feature will\nremind you to take a break from\nplaying approx. every 30 minutes.";
    this->onString = "On";
    this->offString = "Off";
}

// class's destructor
static void VBJaEAutomaticPauseSelectionScreenState_destructor(VBJaEAutomaticPauseSelectionScreenState this)
{	
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEAutomaticPauseSelectionScreenState_enter(VBJaEAutomaticPauseSelectionScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, true, true);

	VBJaEAutomaticPauseSelectionScreenState_print(this);

	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaEAutomaticPauseSelectionScreenState_execute(VBJaEAutomaticPauseSelectionScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaEAutomaticPauseSelectionScreenState_exit(VBJaEAutomaticPauseSelectionScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaEAutomaticPauseSelectionScreenState_resume(VBJaEAutomaticPauseSelectionScreenState this, void* owner)
{
	GameState_resume(__UPCAST(GameState, this), owner);
	
	VBJaEAutomaticPauseSelectionScreenState_print(this);
}

// state's on message
static bool VBJaEAutomaticPauseSelectionScreenState_handleMessage(VBJaEAutomaticPauseSelectionScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEAutomaticPauseSelectionScreenState_processInput(VBJaEAutomaticPauseSelectionScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaEAutomaticPauseSelectionScreenState_print(VBJaEAutomaticPauseSelectionScreenState this)
{
    u8 strHeaderXPos = (48 - strlen(this->titleString)) >> 1;
    Printing_text(Printing_getInstance(), this->titleString, strHeaderXPos, 8, NULL);

    Printing_text(Printing_getInstance(), this->explanationString, 8, 11, NULL);

    VBJaEAutomaticPauseSelectionScreenState_renderSelection(this);
}

static void VBJaEAutomaticPauseSelectionScreenState_renderSelection(VBJaEAutomaticPauseSelectionScreenState this)
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

static void VBJaEAutomaticPauseSelectionScreenState_processInput(VBJaEAutomaticPauseSelectionScreenState this, u16 pressedKey)
{
	if ((pressedKey & K_LL) || (pressedKey & K_LR))
	{
	    this->selection = !this->selection;
	    VBJaEAutomaticPauseSelectionScreenState_renderSelection(this);
	}
	else if ((pressedKey & K_A) || (pressedKey & K_STA))
	{
		Game_setAutomaticPauseState(Game_getInstance(), this->selection ? __UPCAST(GameState, VBJaEAutomaticPauseScreenState_getInstance()): NULL);
	    Game_changeState(Game_getInstance(), this->nextState);
	}
}

void VBJaEAutomaticPauseSelectionScreenState_setNextstate(VBJaEAutomaticPauseSelectionScreenState this, GameState nextState)
{
    this->nextState = nextState;
}

void VBJaEAutomaticPauseSelectionScreenState_setExplanationString(VBJaEAutomaticPauseSelectionScreenState this, char* string)
{
    this->explanationString = string;
}

void VBJaEAutomaticPauseSelectionScreenState_setTitleString(VBJaEAutomaticPauseSelectionScreenState this, char* string)
{
    this->titleString = string;
}

void VBJaEAutomaticPauseSelectionScreenState_setOnString(VBJaEAutomaticPauseSelectionScreenState this, char* string)
{
    this->onString = string;
}

void VBJaEAutomaticPauseSelectionScreenState_setOffString(VBJaEAutomaticPauseSelectionScreenState this, char* string)
{
    this->offString = string;
}