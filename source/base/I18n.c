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

extern LangROMSpec* __LANGUAGES[];


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/// Get instance
/// @fn			I18n::getInstance()
/// @memberof	I18n
/// @public
/// @return		I18n instance


/**
 * Class constructor
 * @private
 */
void I18n::constructor()
{
	Base::constructor();

	this->activeLanguage = 0;
}

/**
 * Class destructor
 */
void I18n::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Get localized string
 * @param string	The identifier of the string to localize
 * @return 		localized string or NULL if no translation could be found
 */
const char* I18n::getText(int string)
{
	// TODO: check if __LANGUAGES is empty
	return 0 <= string ? __LANGUAGES[this->activeLanguage]->language[string] : NULL;
}

/**
 * Set the active language
 * @param languageId	ID of the language to make active
 */
void I18n::setActiveLanguage(u8 languageId)
{
	this->activeLanguage = languageId;
}

/**
 * Get all registered languages
 * @return		Array of LangSpec pointers
 */
LangSpec * I18n::getLanguages()
{
	return (LangSpec *)__LANGUAGES;
}

/**
 * Retrieves ID of the currently active language
 * @return		ID of currently active language
 */
u8 I18n::getActiveLanguage()
{
	return this->activeLanguage;
}

/**
 * Retrieves name of the currently active language
 * @return	Name of currently active language
 */
char* I18n::getActiveLanguageName()
{
	// TODO: check if __LANGUAGES is empty, return "none" if so
	return (char*)__LANGUAGES[this->activeLanguage]->name;
}
