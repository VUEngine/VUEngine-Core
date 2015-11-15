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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <I18n.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern LangROMDef* __LANGUAGES[];


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define I18n_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* currently active language */												\
	u8 ActiveLanguage;															\

// define the I18n
__CLASS_DEFINITION(I18n, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void I18n_constructor(I18n this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(I18n);

// class's constructor
static void I18n_constructor(I18n this)
{
	ASSERT(this, "I18n::constructor: null this");

	__CONSTRUCT_BASE();

	this->ActiveLanguage = 0;
}

// class's destructor
void I18n_destructor(I18n this)
{
	ASSERT(this, "I18n::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// get localized string
char* I18n_getText(I18n this, int string)
{
	ASSERT(this, "I18n::getText: null this");

	return 0 <= string ? __LANGUAGES[this->ActiveLanguage]->language[string] : NULL;
}

// set the language
void I18n_setActiveLanguage(I18n this, u8 languageId)
{
	ASSERT(this, "I18n::setActiveLanguage: null this");

    this->ActiveLanguage = languageId;
}

// get all registered languages
LangDefinition * I18n_getLanguages(I18n this)
{
	ASSERT(this, "I18n::getLanguages: null this");

    return (LangDefinition *)__LANGUAGES;
}

// get the id of the currently active language
u8 I18n_getActiveLanguage(I18n this)
{
	ASSERT(this, "I18n::getActiveLanguage: null this");

    return this->ActiveLanguage;
}

// get the name of the currently active language
char* I18n_getActiveLanguageName(I18n this)
{
	ASSERT(this, "I18n::getActiveLanguageName: null this");

    return (char*)__LANGUAGES[this->ActiveLanguage]->name;
}