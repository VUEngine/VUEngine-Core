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

#ifndef __CUSTOM_LANGUAGES

/*
 * IMPORTANT: Ensure that this file is encoded in Windows-1252 or ISO-8859-1 ("ANSI") to make use
 * of the full extended ASCII character set including special characters of European languages.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <I18n.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
//---------------------------------------------------------------------------------------------------------

const LangROMDef LANGUAGE_DE =
{
    // Language Name
    "Deutsch",

    // Strings
    {
        /* Splash Screens */

        //STR_PRECAUTION_SCREEN_TITLE:
        "WICHTIG:",
        //STR_PRECAUTION_SCREEN_TEXT:
        "LESEN SIE VOR DEM SPIELEN\n    DIE ANLEITUNG UND\n   SICHERHEITSHINWEISE",
        //STR_AUTOMATIC_PAUSE:
        "Automatische Pause",
        //STR_AUTOMATIC_PAUSE_EXPLANATION:
        " Die Automatische Pause Funktion\nerinnert dich daran, das Spiel ca.\n  alle 30 Minuten zu pausieren",
        //STR_AUTOMATIC_PAUSE_TEXT:
        "Bitte mache eine Pause!",
        //STR_ON:
        "An",
        //STR_OFF:
        "Aus",
        //STR_LANGUAGE_SELECT:
        "Sprachauswahl",
    },
};


#endif