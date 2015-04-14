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
#include <I18n.h>
#include <LanguagesDefault.h>
#include <VBJaEAutoPauseSelectScreenState.h>
#include <VBJaEAutoPauseScreenState.h>
#include <VBJaELangSelectScreenState.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

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
static void VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState this, u16 releasedKey);


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
		case kKeyReleased:
		{
            u16 releasedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState_getInstance(), releasedKey);
        }
        break;
	}

	return false;
}

static void VBJaEAutoPauseSelectScreenState_print(VBJaEAutoPauseSelectScreenState this)
{
    char* strAutomaticPause = I18n_getText(I18n_getInstance(), STR_AUTOMATIC_PAUSE);
    char* strAutomaticPauseExplanation = I18n_getText(I18n_getInstance(), STR_AUTOMATIC_PAUSE_EXPLANATION);

    u8 strHeaderXPos = (48 - strlen(strAutomaticPause)) >> 1;
    Printing_text(Printing_getInstance(), strAutomaticPause, strHeaderXPos, 8, NULL);

    Printing_text(Printing_getInstance(), strAutomaticPauseExplanation, 8, 11, NULL);

    VBJaEAutoPauseSelectScreenState_renderSelection(this);
}

static void VBJaEAutoPauseSelectScreenState_renderSelection(VBJaEAutoPauseSelectScreenState this)
{
    char* strOn = I18n_getText(I18n_getInstance(), STR_ON);
    char* strOff = I18n_getText(I18n_getInstance(), STR_OFF);

    // get strings and determine lengths
    u8 strOnLength = strlen(strOn);
    u8 strOffLength = strlen(strOff);
    u8 optionsGap = 3;
    u8 selectionStart = (48 - (strOnLength + optionsGap + strOffLength)) >> 1;

    // clear options area
    Printing_text(Printing_getInstance(), "                                                ", 0, 16, NULL);
    Printing_text(Printing_getInstance(), "                                                ", 0, 17, NULL);
    Printing_text(Printing_getInstance(), "                                                ", 0, 18, NULL);

    // print options
    Printing_text(Printing_getInstance(), strOn, selectionStart, 17, NULL);
    Printing_text(Printing_getInstance(), strOff, selectionStart + 3 + strOnLength, 17, NULL);

    // print selector
    u8 optionStart = this->selection ? selectionStart - 1 : selectionStart - 1 + optionsGap + strOnLength;
    u8 optionEnd = this->selection ? optionStart + 1 + strOnLength : optionStart + 1 + strOffLength;
    Printing_text(Printing_getInstance(), "\x03\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", optionStart, 16, NULL);
    Printing_text(Printing_getInstance(), "\x04               ", optionEnd, 16, NULL);
    Printing_text(Printing_getInstance(), "\x07", optionStart, 17, NULL);
    Printing_text(Printing_getInstance(), "\x07", optionEnd, 17, NULL);
    Printing_text(Printing_getInstance(), "\x05\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", optionStart, 18, NULL);
    Printing_text(Printing_getInstance(), "\x06               ", optionEnd, 18, NULL);
}

static void VBJaEAutoPauseSelectScreenState_processInput(VBJaEAutoPauseSelectScreenState this, u16 releasedKey)
{
	if ((releasedKey & K_LL) || (releasedKey & K_LR))
	{
	    this->selection = !this->selection;
	    VBJaEAutoPauseSelectScreenState_renderSelection(this);
	}
	else if ((releasedKey & K_A) || (releasedKey & K_STA))
	{
		Game_setAutomaticPauseState(Game_getInstance(), this->selection ? __UPCAST(GameState, VBJaEAutoPauseScreenState_getInstance()): NULL);
	    Game_changeState(Game_getInstance(), this->nextState);
	}
}

void VBJaEAutoPauseSelectScreenState_setNextstate(VBJaEAutoPauseSelectScreenState this, GameState nextState)
{
    this->nextState = nextState;
}