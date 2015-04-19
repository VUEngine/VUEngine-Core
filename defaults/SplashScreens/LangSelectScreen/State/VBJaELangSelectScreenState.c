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
#include <Printing.h>
#include <MessageDispatcher.h>
#include <I18n.h>
#include <VBJaELangSelectScreenState.h>
#include <DefaultScreenConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern LangROMDef* __LANGUAGES[];


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaELangSelectScreenState_destructor(VBJaELangSelectScreenState this);
static void VBJaELangSelectScreenState_constructor(VBJaELangSelectScreenState this);
static void VBJaELangSelectScreenState_processInput(VBJaELangSelectScreenState this, u16 releasedKey);
static void VBJaELangSelectScreenState_print(VBJaELangSelectScreenState this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaELangSelectScreenState, SplashScreenState);
__SINGLETON_DYNAMIC(VBJaELangSelectScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaELangSelectScreenState_constructor(VBJaELangSelectScreenState this)
{
	__CONSTRUCT_BASE();

	SplashScreenState_setNextstate(__UPCAST(SplashScreenState, this), __UPCAST(GameState, __LANGUAGE_SELECT_SCREEN_NEXT_STATE()));
	this->stageDefinition = (StageDefinition*)__LANGUAGE_SELECT_SCREEN_STAGE;

    u8 activeLanguage = I18n_getActiveLanguage(I18n_getInstance());
	this->languageSelector = OptionsSelector_new(1, 8, __LANGUAGE_SELECT_SCREEN_LIST_SELECTOR, kString);

	VirtualList languageNames = VirtualList_new();

	int i = 0;
	for (; __LANGUAGES[i]; i++)
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

void VBJaELangSelectScreenState_processInput(VBJaELangSelectScreenState this, u16 releasedKey)
{
	if ((releasedKey & K_LU) || (releasedKey & K_RU))
	{
		OptionsSelector_selectPrevious(this->languageSelector);
	}
    else if ((releasedKey & K_LD) || (releasedKey & K_RD))
	{
		OptionsSelector_selectNext(this->languageSelector);
	}
	else if ((releasedKey & K_A) || (releasedKey & K_STA))
	{
	    I18n_setActiveLanguage(I18n_getInstance(), OptionsSelector_getSelectedOption(this->languageSelector));
	    Game_changeState(Game_getInstance(), __UPCAST(GameState, this->nextState));
	}
}

static void VBJaELangSelectScreenState_print(VBJaELangSelectScreenState this)
{
    char* strLanguageSelect = I18n_getText(I18n_getInstance(), __LANGUAGE_SELECT_SCREEN_TITLE);
    Size size = Printing_getTextSize(Printing_getInstance(), strLanguageSelect, __LANGUAGE_SELECT_SCREEN_TITLE_FONT);

    u8 strHeaderXPos = (__SCREEN_WIDTH >> 4) - (size.x >> 1);

    Printing_text(Printing_getInstance(), strLanguageSelect, strHeaderXPos, 8, __LANGUAGE_SELECT_SCREEN_TITLE_FONT);

	OptionsSelector_showOptions(this->languageSelector, strHeaderXPos, 9 + size.y);
}

