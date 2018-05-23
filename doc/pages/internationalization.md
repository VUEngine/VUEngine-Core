Internationalization
====================

The Internationalization ("I18n") class allows you to add multiple selectable languages to your project.


Defining strings
----------------

Create an enum that defines all translatable strings that are in the game.

    enum {
        STR_HELLO,
        STR_VUENGINE_ROCKS
    };


Languages files
---------------

For each language, create a LangROMDef, which consists of a language name string and an array of strings. Make sure to keep the string order as defined in the enum above. You'll want to create one file per language definition. Ensure these files are encoded in ANSI for special characters ("umlauts") to work.

The following gives an example for an English language definition.

    LangROMDef LANGUAGE_EN =
    {
        // Language Name
        "English",
        
        {
            // STR_HELLO
            "Hello",
            
            // STR_VUENGINE_ROCKS
            "VUEngine rocks! :-)",
        },
    };


Registering languages
---------------------

During the initialization phase of your game, inform the engine about available languages by registering each one using the `I18n::registerLanguage` method.

    I18n::registerLanguage(I18n::getInstance(), &LANGUAGE_EN);


Setting active language
-----------------------

You can set the active language using the `I18n::setActiveLanguage` method. Each registered language is identified by an integer in the order they were registered, starting with 0. By default, the first registered language is used.

    I18n::setActiveLanguage(I18n::getInstance(), 0);

Alternatively, you can also use the `I18n::setActiveLanguageByName` method to set the active language by one of the language names defined in your LangROMDefs.

    I18n::setActiveLanguageByName(I18n::getInstance(), "English");


Getting a translated string
---------------------------

The `I18n::getText` method retrieves a string by a given identifier in the currently active language.

    I18n::getText(I18n::getInstance(), STR_HELLO);
