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

#include <I18n.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define I18n_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* registered languages */													\
	const LangDefinition* languages[16];										\
																				\
	/* total number of registered languages */									\
	u8 languageCount;															\
																				\
	/* currently active language */												\
	u8 language;																\

// define the I18n
__CLASS_DEFINITION(I18n);


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
	this->languageCount = 0;
	this->language = 0;

	__CONSTRUCT_BASE(Object);
}

// class's destructor
void I18n_destructor(I18n this)
{
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// get localized string
char* I18n_getText(I18n this, int string)
{
	return this->languages[this->language]->language[string];
}

// set the language
void I18n_setLanguage(I18n this, const char* lang)
{
    u8 i;

    for (i = 0; i < this->languageCount; i++)
    {
        if (0 == strcmp(this->languages[i]->name, lang))
        {
	        this->language = i;
	        break;
        }
    }
}

// register a languages
void I18n_registerLanguage(I18n this, const LangDefinition* langDefinition)
{
	this->languages[this->languageCount++] = langDefinition;
}

// get all registered languages
LangDefinition* I18n_getLanguages(I18n this)
{
    return this->languages;
}