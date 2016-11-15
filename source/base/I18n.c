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

/**
 * @class       I18n
 * @extends     Object
 * @brief       Handles internationalization of text output and thus allows multiple selectable languages.
 */
__CLASS_DEFINITION(I18n, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void I18n_constructor(I18n this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn          I18n_getInstance()
 * @memberof    I18n
 * @public
 *
 * @return      I18n instance
 */
__SINGLETON(I18n);

/**
 * Class constructor
 *
 * @memberof    I18n
 * @private
 *
 * @param this  Function scope
 */
static void __attribute__ ((noinline)) I18n_constructor(I18n this)
{
	ASSERT(this, "I18n::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->activeLanguage = 0;
}

/**
 * Class destructor
 *
 * @memberof    I18n
 * @public
 *
 * @param this  Function scope
 */
void I18n_destructor(I18n this)
{
	ASSERT(this, "I18n::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Get localized string
 *
 * @memberof        I18n
 * @public
 *
 * @param this      Function scope
 * @param string    The identifier of the string to localize
 *
 * @return localized string or NULL if no translation could be found
 */
const char* I18n_getText(I18n this, int string)
{
	ASSERT(this, "I18n::getText: null this");

	return 0 <= string ? __LANGUAGES[this->activeLanguage]->language[string] : NULL;
}

/**
 * Set the active language
 *
 * @memberof            I18n
 * @public
 *
 * @param this          Function scope
 * @param languageId    ID of the language to make active
 */
void I18n_setActiveLanguage(I18n this, u8 languageId)
{
	ASSERT(this, "I18n::setActiveLanguage: null this");

    this->activeLanguage = languageId;
}

/**
 * Get all registered languages
 *
 * @memberof    I18n
 * @public
 *
 * @param this  Function scope
 *
 * @return      Array of LangDefinition pointers
 */
LangDefinition * I18n_getLanguages(I18n this __attribute__ ((unused)))
{
	ASSERT(this, "I18n::getLanguages: null this");

    return (LangDefinition *)__LANGUAGES;
}

/**
 * Retrieves ID of the currently active language
 *
 * @memberof    I18n
 * @public
 *
 * @param this  Function scope
 *
 * @return      ID of currently active language
 */
u8 I18n_getActiveLanguage(I18n this)
{
	ASSERT(this, "I18n::getActiveLanguage: null this");

    return this->activeLanguage;
}

/**
 * Retrieves name of the currently active language
 *
 * @memberof    I18n
 * @public
 *
 * @param this  Function scope
 *
 * @return      Name of currently active language
 */
char* I18n_getActiveLanguageName(I18n this)
{
	ASSERT(this, "I18n::getActiveLanguageName: null this");

    return (char*)__LANGUAGES[this->activeLanguage]->name;
}
