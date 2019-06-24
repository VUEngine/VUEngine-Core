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
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// handy macros
#define __TRANSLATE(id)				I18n_getText(I18n_getInstance(), id)

// max length of a font's name
#define __MAX_LANGUAGE_NAME_LENGTH	32


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

enum I18nEvents
{
	kEventLanguageChanged = kLastEngineEvent + 1
};


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A language spec
 * @memberof I18n
 */
typedef struct LangSpec
{
	/// language name
	char name[__MAX_LANGUAGE_NAME_LENGTH];

	/// pointer to a representative entity (i.e. flag)
	EntitySpec* entitySpec;

	/// language strings
	const char** language;

} LangSpec;

/**
 * A LangSpec that is stored in ROM
 * @memberof I18n
 */
typedef const LangSpec LangROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Handles internationalization of text output and thus allows for multiple selectable languages.
 * @ingroup base
 */
singleton class I18n : Object
{
	// Currently active language
	u8 activeLanguage;

	/// @publicsection
	static I18n getInstance();
	u8 getActiveLanguage();
	char* getActiveLanguageName();
	LangSpec * getLanguages();
	const char* getText(int string);
	void setActiveLanguage(u8 languageId);
}


#endif
