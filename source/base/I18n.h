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

#ifndef I18N_H_
#define I18N_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

// max length of a font's name
#define __MAX_LANGUAGE_NAME_LENGTH	32


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods

/**
 * A language
 *
 * @memberof 	I18n
 */
typedef struct LangDefinition
{
	/// Language name
	char name[__MAX_LANGUAGE_NAME_LENGTH];

	/// Language strings
	const char** language;

} LangDefinition;

/**
 * A LangDefinition that is stored in ROM
 *
 * @memberof 	I18n
 */
typedef const LangDefinition LangROMDef;


singleton class I18n : Object
{
	/**
	* @var u8		activeLanguage
	* @brief		Currently active language
	* @memberof	I18n
	*/
	u8 activeLanguage;

	static I18n getInstance();
	u8 getActiveLanguage(I18n this);
	char* getActiveLanguageName(I18n this);
	LangDefinition * getLanguages(I18n this);
	const char* getText(I18n this, int string);
	void setActiveLanguage(I18n this, u8 languageId);
}


#endif
