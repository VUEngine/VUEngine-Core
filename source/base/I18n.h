/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef I18N_H_
#define I18N_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

// max length of a font's name
#define __MAX_LANGUAGE_NAME_LENGTH	32


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define I18n_METHODS(ClassName)																					\
		Object_METHODS(ClassName)																					\

// declare the virtual methods which are redefined
#define I18n_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\

// declare a I18n
__CLASS(I18n);

typedef struct LangDefinition
{
	// language name
	char name[__MAX_LANGUAGE_NAME_LENGTH];

    // language strings
	char* language[];

} LangDefinition;

typedef const LangDefinition LangROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

I18n I18n_getInstance();

void I18n_destructor(I18n this);
char* I18n_getText(I18n this, int string);
void I18n_setActiveLanguage(I18n this, u8 languageId);
LangDefinition * I18n_getLanguages(I18n this);
u8 I18n_getActiveLanguage(I18n this);
char* I18n_getActiveLanguageName(I18n this);


#endif
