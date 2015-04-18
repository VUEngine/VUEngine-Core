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
#include <VBJaEAdjustmentScreenState.h>
#include <VBJaEAutoPauseSelectScreenState.h>
#include <VBJaESplashScreenState.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
// ---------------------------------------------------------------------------------------------------------

// Precaution screen
VBJaEAdjustmentScreenState   (*__PRECAUTION_SCREEN_NEXT_STATE)() =        VBJaEAdjustmentScreenState_getInstance;
const int     __PRECAUTION_SCREEN_INITIAL_DELAY =        2000;
const int     __PRECAUTION_SCREEN_TEXT =                 STR_PRECAUTION_SCREEN;
const char*   __PRECAUTION_SCREEN_TEXT_FONT =            NULL;

// Adjustment screen
VBJaEAutoPauseSelectScreenState   (*__ADJUSTMENT_SCREEN_NEXT_STATE)() =        VBJaEAutoPauseSelectScreenState_getInstance;

// Language selection screen
VBJaESplashScreenState   (*__LANGUAGE_SELECT_SCREEN_NEXT_STATE)() =   VBJaESplashScreenState_getInstance;
char*         __LANGUAGE_SELECT_SCREEN_LIST_SELECTOR =   "\xB";
const int     __LANGUAGE_SELECT_SCREEN_TITLE =           STR_LANGUAGE_SELECT;
const char*   __LANGUAGE_SELECT_SCREEN_TITLE_FONT =      NULL;

// VBJaEngine splash screen
VBJaESplashScreenState   (*__VBJAENGINE_SPLASH_SCREEN_NEXT_STATE)() = VBJaESplashScreenState_getInstance;