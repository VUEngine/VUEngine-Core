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
#include <VBJaELangSelectScreenState.h>
#include <VBJaESplashScreenState.h>
#include <I18n.h>

extern StageROMDef EMPTY_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaELangSelectScreenState_destructor(VBJaELangSelectScreenState this);
static void VBJaELangSelectScreenState_constructor(VBJaELangSelectScreenState this);
static void VBJaELangSelectScreenState_enter(VBJaELangSelectScreenState this, void* owner);
static void VBJaELangSelectScreenState_execute(VBJaELangSelectScreenState this, void* owner);
static void VBJaELangSelectScreenState_exit(VBJaELangSelectScreenState this, void* owner);
static void VBJaELangSelectScreenState_resume(VBJaELangSelectScreenState this, void* owner);
static bool VBJaELangSelectScreenState_handleMessage(VBJaELangSelectScreenState this, void* owner, Telegram telegram);
static void VBJaELangSelectScreenState_processInput(VBJaELangSelectScreenState this, u16 pressedKey);
static void VBJaELangSelectScreenState_print(VBJaELangSelectScreenState this);
void VBJaELangSelectScreenState_setTitleString(VBJaELangSelectScreenState this, char* string);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaELangSelectScreenState, GameState);
__SINGLETON_DYNAMIC(VBJaELangSelectScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaELangSelectScreenState_constructor(VBJaELangSelectScreenState this)
{
	__CONSTRUCT_BASE();

	VBJaELangSelectScreenState_setNextstate(this, __UPCAST(GameState, VBJaESplashScreenState_getInstance()));
	this->stageDefinition = (StageDefinition*)&EMPTY_ST;
	this->titleString = "Language Selection";

    u8 activeLanguage = I18n_getActiveLanguage(I18n_getInstance());
	this->languageSelector = OptionsSelector_new(1, 8, "\x10", kString);

	VirtualList languageNames = VirtualList_new();

	int i = 0;
	for (;  i < I18n_getLanguageCount(I18n_getInstance()); i++)
	{
		I18n_setActiveLanguage(I18n_getInstance(), i);
		VirtualList_pushBack(languageNames, I18n_getActiveLanguageName(I18n_getInstance()));
	}

	I18n_setActiveLanguage(I18n_getInstance(), activeLanguage);

    OptionsSelector_setOptions(this->languageSelector, languageNames);
	__DELETE(languageNames);
}

// class's destructor
static void VBJaELangSelectScreenState_destructor(VBJaELangSelectScreenState this)
{
	if (this->languageSelector)
	{
		__DELETE(this->languageSelector);
	}

	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaELangSelectScreenState_enter(VBJaELangSelectScreenState this, void* owner)
{
	GameState_loadStage(__UPCAST(GameState, this), this->stageDefinition, true, true);

	VBJaELangSelectScreenState_print(this);
	
	Screen_FXFadeIn(Screen_getInstance(), 16);
}

// state's execute
static void VBJaELangSelectScreenState_execute(VBJaELangSelectScreenState this, void* owner)
{
 	// call base
	GameState_execute(__UPCAST(GameState, this), owner);
}

// state's exit
static void VBJaELangSelectScreenState_exit(VBJaELangSelectScreenState this, void* owner)
{
	Screen_FXFadeOut(Screen_getInstance(), 16);

	// destroy the state
	__DELETE(this);
}

// state's resume
static void VBJaELangSelectScreenState_resume(VBJaELangSelectScreenState this, void* owner)
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
	
	VBJaELangSelectScreenState_print(this);
}

// state's on message
static bool VBJaELangSelectScreenState_handleMessage(VBJaELangSelectScreenState this, void* owner, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kKeyPressed:
		{
            u16 pressedKey = *((u16*)Telegram_getExtraInfo(telegram));

            VBJaELangSelectScreenState_processInput(VBJaELangSelectScreenState_getInstance(), pressedKey);
        }
        break;
	}

	return false;
}

static void VBJaELangSelectScreenState_processInput(VBJaELangSelectScreenState this, u16 pressedKey)
{
	if (pressedKey & K_LU)
	{
		OptionsSelector_selectPrevious(this->languageSelector);
	}
	else if (pressedKey & K_LD)
	{
		OptionsSelector_selectNext(this->languageSelector);
	}
	else if (pressedKey & K_A)
	{
	    I18n_setActiveLanguage(I18n_getInstance(), OptionsSelector_getSelectedOption(this->languageSelector));
	    Game_changeState(Game_getInstance(), __UPCAST(GameState, this->nextState));
	}
}

static void VBJaELangSelectScreenState_print(VBJaELangSelectScreenState this)
{
    u8 strHeaderXPos = (48 - strlen(this->titleString)) >> 1;

    Printing_text(Printing_getInstance(), this->titleString, strHeaderXPos, 8, NULL);

	OptionsSelector_showOptions(this->languageSelector, strHeaderXPos, 11);
}

void VBJaELangSelectScreenState_setNextstate(VBJaELangSelectScreenState this, GameState nextState)
{
    this->nextState = nextState;
}

void VBJaELangSelectScreenState_setTitleString(VBJaELangSelectScreenState this, char* string)
{
    this->titleString = string;
}