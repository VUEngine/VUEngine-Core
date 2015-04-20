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

#include <LanguagesDefault.h>
#include <VBJaEPrecautionScreenState.h>
#include <VBJaEAdjustmentScreenState.h>
#include <VBJaEAutoPauseSelectScreenState.h>
#include <VBJaELangSelectScreenState.h>
#include <VBJaESplashScreenState.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern StageROMDef EMPTY_ST;
extern StageROMDef VBJAENGINE_ADJUSTMENT_SCREEN_ST;
extern StageROMDef VBJAENGINE_SPLASH_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
// ---------------------------------------------------------------------------------------------------------

// Precaution screen
VBJaEPrecautionScreenState      (*__INITIAL_SPLASH_SCREEN_STATE)() =            VBJaEPrecautionScreenState_getInstance;
VBJaEAdjustmentScreenState      (*__PRECAUTION_SCREEN_NEXT_STATE)() =           VBJaEAdjustmentScreenState_getInstance;
const StageROMDef*                __PRECAUTION_SCREEN_STAGE =                   &EMPTY_ST;
const int                         __PRECAUTION_SCREEN_INITIAL_DELAY =           2000; // 2 seconds, as defined by Nintendo
const int                         __PRECAUTION_SCREEN_TEXT =                    STR_PRECAUTION_SCREEN;
const char*                       __PRECAUTION_SCREEN_TEXT_FONT =               NULL;

// Adjustment screen
VBJaEAutoPauseSelectScreenState (*__ADJUSTMENT_SCREEN_NEXT_STATE)() =           VBJaEAutoPauseSelectScreenState_getInstance;
const StageROMDef*                __ADJUSTMENT_SCREEN_STAGE =                   &VBJAENGINE_ADJUSTMENT_SCREEN_ST;

// Auto Pause selection screen
VBJaELangSelectScreenState      (*__AUTO_PAUSE_SELECT_SCREEN_NEXT_STATE)() =    VBJaELangSelectScreenState_getInstance;
const StageROMDef*                __AUTO_PAUSE_SELECT_SCREEN_STAGE =            &EMPTY_ST;
const int                         __AUTO_PAUSE_SELECT_SCREEN_TITLE =            STR_AUTOMATIC_PAUSE;
const char*                       __AUTO_PAUSE_SELECT_SCREEN_TITLE_FONT =       NULL;
const int                         __AUTO_PAUSE_SELECT_SCREEN_EXPLANATION =      STR_AUTOMATIC_PAUSE_EXPLANATION;
const char*                       __AUTO_PAUSE_SELECT_SCREEN_EXPLANATION_FONT = NULL;
const int                         __AUTO_PAUSE_SELECT_SCREEN_ON =               STR_ON;
const char*                       __AUTO_PAUSE_SELECT_SCREEN_ON_FONT =          NULL;
const int                         __AUTO_PAUSE_SELECT_SCREEN_OFF =              STR_OFF;
const char*                       __AUTO_PAUSE_SELECT_SCREEN_OFF_FONT =         NULL;

// Language selection screen
VBJaESplashScreenState          (*__LANGUAGE_SELECT_SCREEN_NEXT_STATE)() =      VBJaESplashScreenState_getInstance;
const StageROMDef*                __LANGUAGE_SELECT_SCREEN_STAGE =              &EMPTY_ST;
char*                             __LANGUAGE_SELECT_SCREEN_LIST_SELECTOR =      "\xB";
const int                         __LANGUAGE_SELECT_SCREEN_TITLE =              STR_LANGUAGE_SELECT;
const char*                       __LANGUAGE_SELECT_SCREEN_TITLE_FONT =         NULL;

// VBJaEngine splash screen
VBJaELangSelectScreenState      (*__VBJAENGINE_SPLASH_SCREEN_NEXT_STATE)() =    VBJaELangSelectScreenState_getInstance;
const StageROMDef*                __VBJAENGINE_SPLASH_SCREEN_STAGE =            &VBJAENGINE_SPLASH_SCREEN_ST;

// Auto Pause screen
const StageROMDef*                __AUTO_PAUSE_SCREEN_STAGE =                   &EMPTY_ST;
const int                         __AUTO_PAUSE_SCREEN_TITLE =                   STR_AUTOMATIC_PAUSE;
const char*                       __AUTO_PAUSE_SCREEN_TITLE_FONT =              NULL;
const int                         __AUTO_PAUSE_SCREEN_TEXT =                    STR_AUTOMATIC_PAUSE_TEXT;
const char*                       __AUTO_PAUSE_SCREEN_TEXT_FONT =               NULL;