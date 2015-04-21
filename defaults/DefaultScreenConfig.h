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

#ifndef DEFAULT_SCREEN_CONFIG_H_
#define DEFAULT_SCREEN_CONFIG_H_


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern GameState         (*__INITIAL_SPLASH_SCREEN_STATE)();

// Precaution screen
extern GameState         (*__PRECAUTION_SCREEN_NEXT_STATE)();
extern const int           __PRECAUTION_SCREEN_INITIAL_DELAY;
extern const int           __PRECAUTION_SCREEN_TITLE;
extern const char*         __PRECAUTION_SCREEN_TITLE_FONT;
extern const int           __PRECAUTION_SCREEN_TEXT;
extern const char*         __PRECAUTION_SCREEN_TEXT_FONT;
extern const StageROMDef*  __PRECAUTION_SCREEN_STAGE;

// Adjustment screen
extern GameState         (*__ADJUSTMENT_SCREEN_NEXT_STATE)();
extern const StageROMDef*  __ADJUSTMENT_SCREEN_STAGE;

// Auto Pause selection screen
extern GameState         (*__AUTO_PAUSE_SELECT_SCREEN_NEXT_STATE)();
extern const int           __AUTO_PAUSE_SELECT_SCREEN_TITLE;
extern const char*         __AUTO_PAUSE_SELECT_SCREEN_TITLE_FONT;
extern const int           __AUTO_PAUSE_SELECT_SCREEN_EXPLANATION;
extern const char*         __AUTO_PAUSE_SELECT_SCREEN_EXPLANATION_FONT;
extern const int           __AUTO_PAUSE_SELECT_SCREEN_ON;
extern const int           __AUTO_PAUSE_SELECT_SCREEN_OFF;
extern const char*         __AUTO_PAUSE_SELECT_SCREEN_OPTIONS_FONT;
extern const StageROMDef*  __AUTO_PAUSE_SELECT_SCREEN_STAGE;

// Language selection screen
extern GameState         (*__LANGUAGE_SELECT_SCREEN_NEXT_STATE)();
extern const StageROMDef*  __LANGUAGE_SELECT_SCREEN_STAGE;
extern char*               __LANGUAGE_SELECT_SCREEN_LIST_SELECTOR;
extern const int           __LANGUAGE_SELECT_SCREEN_TITLE;
extern const char*         __LANGUAGE_SELECT_SCREEN_TITLE_FONT;

// VBJaEngine splash screen
extern GameState         (*__VBJAENGINE_SPLASH_SCREEN_NEXT_STATE)();
extern const StageROMDef*  __VBJAENGINE_SPLASH_SCREEN_STAGE;

// Auto Pause screen
extern const StageROMDef*  __AUTO_PAUSE_SCREEN_STAGE;
extern const int           __AUTO_PAUSE_SCREEN_TITLE;
extern const char*         __AUTO_PAUSE_SCREEN_TITLE_FONT;
extern const int           __AUTO_PAUSE_SCREEN_TEXT;
extern const char*         __AUTO_PAUSE_SCREEN_TEXT_FONT;


#endif