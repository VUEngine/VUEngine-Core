#ifndef LANGUAGES_DEFAULT_H_
#define LANGUAGES_DEFAULT_H_


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/*
 * This default LanguageStrings enum is always defined, even if the __CUSTOM_LANGUAGES macro is defined,
 * so that the default splash screens know these string identifiers.
 * In your custom LanguageStrings enum, you HAVE TO define these first!
 */

enum DefaultLanguageStrings
{
    /* Splash Screens */
	STR_PRECAUTION_SCREEN,
	STR_AUTOMATIC_PAUSE,
	STR_AUTOMATIC_PAUSE_EXPLANATION,
    STR_AUTOMATIC_PAUSE_TEXT,
    STR_ON,
    STR_OFF,
    STR_LANGUAGE_SELECT,
};


#endif
