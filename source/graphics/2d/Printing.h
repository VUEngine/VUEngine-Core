/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef PRINTING_H_
#define	PRINTING_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

// max length of a font's name
#define __MAX_FONT_NAME_LENGTH		16

// printing modes
#define __PRINTING_MODE_DEFAULT		0
#define __PRINTING_MODE_DEBUG		1

// default font special chars
#define __CHAR_BATTERY				"\x01\x02"

#define __CHAR_LINE_TOP_LEFT		"\x03"
#define __CHAR_LINE_TOP_RIGHT		"\x04"
#define __CHAR_LINE_BOTTOM_LEFT		"\x05"
#define __CHAR_LINE_BOTTOM_RIGHT	"\x06"
#define __CHAR_LINE_VERTICAL		"\x07"
#define __CHAR_LINE_HORIZONTAL		"\x08"

#define __CHAR_SELECTOR				"\x0B"
#define __CHAR_SELECTOR_LEFT		"\x0C"

#define __CHAR_ARROW_UP				"\x0E"
#define __CHAR_ARROW_DOWN			"\x0F"
#define __CHAR_ARROW_LEFT			"\x10"
#define __CHAR_ARROW_RIGHT			"\x11"

#define __CHAR_START_BUTTON			"\x15"
#define __CHAR_SELECT_BUTTON		"\x16"
#define __CHAR_A_BUTTON				"\x13"
#define __CHAR_B_BUTTON				"\x14"
#define __CHAR_L_TRIGGER			"\x17"
#define __CHAR_R_TRIGGER			"\x18"
#define __CHAR_D_PAD				"\x19"
#define __CHAR_D_PAD_UP				"\x1A"
#define __CHAR_D_PAD_DOWN			"\x1B"
#define __CHAR_D_PAD_LEFT			"\x1C"
#define __CHAR_D_PAD_RIGHT			"\x1D"
#define __CHAR_L					"\x1E"
#define __CHAR_R					"\x1F"

#define __CHAR_L_D_PAD				"\x1E\x19"
#define __CHAR_L_D_PAD_UP			"\x1E\x1A"
#define __CHAR_L_D_PAD_DOWN			"\x1E\x1B"
#define __CHAR_L_D_PAD_LEFT			"\x1E\x1C"
#define __CHAR_L_D_PAD_RIGHT		"\x1E\x1D"
#define __CHAR_R_D_PAD				"\x1F\x19"
#define __CHAR_R_D_PAD_UP			"\x1F\x1A"
#define __CHAR_R_D_PAD_DOWN			"\x1F\x1B"
#define __CHAR_R_D_PAD_LEFT			"\x1F\x1C"
#define __CHAR_R_D_PAD_RIGHT		"\x1F\x1D"

#define __CHAR_DARK_RED_BOX			"\x8D"
#define __CHAR_MEDIUM_RED_BOX		"\x8F"
#define __CHAR_BRIGHT_RED_BOX		"\x90"

#define __CHAR_CHECKBOX_UNCHECKED	"\x81"
#define __CHAR_CHECKBOX_CHECKED		"\x9D"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Printing_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Printing_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\

// declare class
__CLASS(Printing);

// declare class attributes
#define Printing_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/*
		 * @var VirtualList fonts
		 * @brief			A list of loaded fonts and their respective CharSets
		 * @memberof		Printing
		 */																								\
		VirtualList fonts;																				\
		/*
		 * @var u16			gx
		 * @brief			x coordinate for printing WORLD
		 * @memberof		Printing
		 */																								\
		u16 gx;																						\
		/*
		 * @var u16			gy
		 * @brief			y coordinate for printing WORLD
		 * @memberof		Printing
		 */																								\
		u16 gy;																						\
		/*
		 * @var u8			mode
		 * @brief			Printing mode (Default or Debug)
		 * @memberof		Printing
		 */																								\
		u8 mode;																						\
		/*
		 * @var u8			palette
		 * @brief			Palette to use for printing
		 * @memberof		Printing
		 */																								\
		u8 palette;																						\

typedef struct PixelSize FontSize;

/**
 * A font
 *
 * @memberof 	Printing
 */
typedef struct FontDefinition
{
	/// font charset definition pointer
	CharSetDefinition* charSetDefinition;

	/// at which character number the font starts
	s16 offset;

	/// size of a single character (in chars) ({width, height})
	FontSize fontSize;

	/// font's name
	char name[__MAX_FONT_NAME_LENGTH];

} FontDefinition;

/**
 * A FontDefinition that is stored in ROM
 *
 * @memberof 	Printing
 */
typedef const FontDefinition FontROMDef;

/**
 * A FontDefinition plus a the offset of its charset in memory
 *
 * @memberof 	Printing
 */
typedef struct FontData
{
	/// Font definition
	const struct FontDefinition* fontDefinition;

	/// Offset of font in char memory
	u32 offset;

} FontData;

/**
 * A FontData that is stored in ROM
 *
 * @memberof 	Printing
 */
typedef const FontData FontROMData;


//---------------------------------------------------------------------------------------------------------
//												PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Printing Printing_getInstance();

void Printing_destructor(Printing this);

void Printing_clear(Printing this);
void Printing_float(Printing this, float value, u8 x, u8 y, const char* font);
FontData* Printing_getFontByName(Printing this, const char* font);
FontSize Printing_getTextSize(Printing this, const char* string, const char* font);
void Printing_hex(Printing this, WORD value, u8 x, u8 y, u8 length, const char* font);
void Printing_int(Printing this, int value, u8 x, u8 y, const char* font);
void Printing_loadDebugFont(Printing this);
void Printing_loadFonts(Printing this, FontDefinition** fontDefinitions);
void Printing_render(Printing this, int textLayer);
void Printing_reset(Printing this);
void Printing_setDebugMode(Printing this);
void Printing_setPalette(Printing this, u8 palette);
void Printing_setWorldCoordinates(Printing this, u16 gx, u16 gy);
void Printing_resetWorldCoordinates(Printing this);
int Printing_getPixelCount(Printing this);
void Printing_text(Printing this, const char *string, int x, int y, const char* font);


#endif
