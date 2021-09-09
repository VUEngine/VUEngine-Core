Printing
========

The Printing Layer
------------------

Out of the Virtual Boy's 32 available Worlds, VUEngine always uses the lowest available one for text
output. It provides methods to output different variable types to this so-called *Printing Layer*.

- `Printing::text`
- `Printing::int32`
- `Printing::float`
- `Printing::hex`

Here's an example that outputs the string "Hello, World!" at position {10, 0} using a custom font
registered under the name "CustomFont".

    Printing::text(
        "Hello, World!",			// text
        10,							// x position
        0,							// y position
        "CustomFont",				// font name
    );

The whole printing layer can be cleared with the `Printing::clear` method.

Some aspects of the Printing Layer can be modified in your projects's `config.h` file.

The palette for printing can be set to one of the 4 available one with the
`__PRINTING_PALETTE` setting.

An offset of the printing layer can be defined using the
`__PRINTING_BGMAP_X_OFFSET`, `__PRINTING_BGMAP_Y_OFFSET` and
`__PRINTING_BGMAP_PARALLAX_OFFSET` settings.


Font management
---------------

VUEngine comes with a default font for writing to the Printing Layer, but you can replace it with any number of custom fonts. To tell the engine to ignore the default font and load your custom font(s) instead, you have to define the `__CUSTOM_FONTS` macro in your game's `config.h` file.

    #define __CUSTOM_FONTS

With that macro defined, the engine expects you to define a *NULL terminated* array of pointers to `FontROMDef` definitions called `__FONTS`. The following example defines a single 8x8 pixel font as a direct replacement for the built-in default font.

    extern BYTE font8x8Tiles[];

    FontROMDef FONT_8x8 =
    {
	    font8x8Tiles,			// font chars definition pointer
        256,					// number of characters in font
        0,						// character number at which the font starts, allows you to skip the control characters for example
        {1, 1},					// size of a single character (in chars) ({width, height})
        "Font8x8",				// font's name
    };

    const FontROMDef* __FONTS[] =
    {
        &FONT_8x8,
        NULL
    };

Note that the first font in the `__FONTS` array is always the default one, and is used when `NULL` is passed to the various Printing methods instead of a font name.

The engine's default font will not be loaded when custom fonts are enabled. If you still want to use it, simply add `&DEFAULT_FONT` to your `__FONTS` array. It will then be accessible under the name "VUEngine". Don't forget to also declare it as extern in your font definition file, like so:

    extern FontROMDef DEFAULT_FONT;
