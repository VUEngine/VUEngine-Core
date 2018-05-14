/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <I18n.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern LangROMDef* __LANGUAGES[];


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	I18n
 * @extends Object
 * @ingroup base
 * @brief	Handles internationalization of text output and thus allows multiple selectable languages.
 */
__CLASS_DEFINITION(I18n, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void I18n::constructor(I18n this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			I18n::getInstance()
 * @memberof	I18n
 * @public
 *
 * @return		I18n instance
 */
__SINGLETON(I18n);

/**
 * Class constructor
 *
 * @memberof	I18n
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) I18n::constructor(I18n this)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->activeLanguage = 0;
}

/**
 * Class destructor
 *
 * @memberof	I18n
 * @public
 *
 * @param this	Function scope
 */
void I18n::destructor(I18n this)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Get localized string
 *
 * @memberof		I18n
 * @public
 *
 * @param this		Function scope
 * @param string	The identifier of the string to localize
 *
 * @return localized string or NULL if no translation could be found
 */
const char* I18n::getText(I18n this, int string)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::getText: null this");

	return 0 <= string ? __LANGUAGES[this->activeLanguage]->language[string] : NULL;
}

/**
 * Set the active language
 *
 * @memberof			I18n
 * @public
 *
 * @param this			Function scope
 * @param languageId	ID of the language to make active
 */
void I18n::setActiveLanguage(I18n this, u8 languageId)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::setActiveLanguage: null this");

	this->activeLanguage = languageId;
}

/**
 * Get all registered languages
 *
 * @memberof	I18n
 * @public
 *
 * @param this	Function scope
 *
 * @return		Array of LangDefinition pointers
 */
LangDefinition * I18n::getLanguages(I18n this __attribute__ ((unused)))
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::getLanguages: null this");

	return (LangDefinition *)__LANGUAGES;
}

/**
 * Retrieves ID of the currently active language
 *
 * @memberof	I18n
 * @public
 *
 * @param this	Function scope
 *
 * @return		ID of currently active language
 */
u8 I18n::getActiveLanguage(I18n this)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::getActiveLanguage: null this");

	return this->activeLanguage;
}

/**
 * Retrieves name of the currently active language
 *
 * @memberof	I18n
 * @public
 *
 * @param this	Function scope
 *
 * @return		Name of currently active language
 */
char* I18n::getActiveLanguageName(I18n this)
{
	ASSERT(__SAFE_CAST(I18n, this), "I18n::getActiveLanguageName: null this");

	return (char*)__LANGUAGES[this->activeLanguage]->name;
}
