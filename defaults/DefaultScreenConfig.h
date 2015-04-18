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

#ifndef DEFAULT_SCREEN_STATE_H_
#define DEFAULT_SCREEN_STATE_H_

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <VBJaEAdjustmentScreenState.h>
#include <VBJaEAutoPauseSelectScreenState.h>
#include <VBJaESplashScreenState.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

// Precaution screen
extern VBJaEAdjustmentScreenState   *__PRECAUTION_SCREEN_NEXT_STATE();
extern const int     __PRECAUTION_SCREEN_INITIAL_DELAY;
extern const int     __PRECAUTION_SCREEN_TEXT;
extern const char*   __PRECAUTION_SCREEN_TEXT_FONT;

// Adjustment screen
extern VBJaEAutoPauseSelectScreenState   (*__ADJUSTMENT_SCREEN_NEXT_STATE)();

// Language selection screen
extern VBJaESplashScreenState   (*__LANGUAGE_SELECT_SCREEN_NEXT_STATE)();
extern char*         __LANGUAGE_SELECT_SCREEN_LIST_SELECTOR;
extern const int     __LANGUAGE_SELECT_SCREEN_TITLE;
extern const char*   __LANGUAGE_SELECT_SCREEN_TITLE_FONT;

// VBJaEngine splash screen
extern VBJaESplashScreenState   (*__VBJAENGINE_SPLASH_SCREEN_NEXT_STATE)();


#endif