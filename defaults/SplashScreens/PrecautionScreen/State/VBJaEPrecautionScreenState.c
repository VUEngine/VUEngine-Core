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
#include <I18n.h>
#include <VBJaEPrecautionScreenState.h>
#include <DefaultScreenConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern StageROMDef EMPTY_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEPrecautionScreenState_destructor(VBJaEPrecautionScreenState this);
static void VBJaEPrecautionScreenState_constructor(VBJaEPrecautionScreenState this);
static void VBJaEPrecautionScreenState_enter(VBJaEPrecautionScreenState this, void* owner);
static void VBJaEPrecautionScreenState_resume(VBJaEPrecautionScreenState this, void* owner);
static void VBJaEPrecautionScreenState_print(VBJaEPrecautionScreenState this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEPrecautionScreenState, SplashScreenState);
__SINGLETON_DYNAMIC(VBJaEPrecautionScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEPrecautionScreenState_constructor(VBJaEPrecautionScreenState this)
{
	__CONSTRUCT_BASE();

	SplashScreenState_setNextstate(__UPCAST(SplashScreenState, this), __UPCAST(GameState, __PRECAUTION_SCREEN_NEXT_STATE()));
	this->stageDefinition = (StageDefinition*)&EMPTY_ST;
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
    SplashScreenState_enter(__UPCAST(SplashScreenState, this), owner);

    VBJaEPrecautionScreenState_print(this);

	Screen_FXFadeIn(Screen_getInstance(), 16);

    // show this screen for at least (2) seconds, as defined by Nintendo in the official development manual
	Clock_delay(Game_getClock(Game_getInstance()), __PRECAUTION_SCREEN_INITIAL_DELAY);
}

// state's resume
static void VBJaEPrecautionScreenState_resume(VBJaEPrecautionScreenState this, void* owner)
{
    SplashScreenState_resume(__UPCAST(SplashScreenState, this), owner);

    VBJaEPrecautionScreenState_print(this);
}

static void VBJaEPrecautionScreenState_print(VBJaEPrecautionScreenState this)
{
    // TODO: Automatically center text
    Printing_text(Printing_getInstance(), I18n_getText(I18n_getInstance(), __PRECAUTION_SCREEN_TEXT), 14, 10, __PRECAUTION_SCREEN_TEXT_FONT);
}

