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

#ifndef OPTIONS_SELECTOR_H_
#define OPTIONS_SELECTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __OPTIONS_SELECT_MAX_COLS		__SCREEN_WIDTH >> 5
#define __OPTIONS_SELECT_MAX_ROWS		__SCREEN_HEIGHT >> 3


//---------------------------------------------------------------------------------------------------------
//											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define OptionsSelector_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define OptionsSelector_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

// class attributes
#define OptionsSelector_ATTRIBUTES																		\
		Object_ATTRIBUTES																				\
		/**
		 * @var VirtualList pages
		 * @brief			List of pages, each being a VirtualLists of Options
		 * @memberof		OptionsSelector
		 */																								\
		VirtualList pages;																				\
		/**
		 * @var VirtualNode currentPage
		 * @brief			Current page node
		 * @memberof		OptionsSelector
		 */																								\
		VirtualNode currentPage;																		\
		/**
		 * @var VirtualNode currentOption
		 * @brief			Current option node
		 * @memberof		OptionsSelector
		 */																								\
		VirtualNode currentOption;																		\
		/**
		 * @var u8			x
		 * @brief			Printing column
		 * @memberof		OptionsSelector
		 */																								\
		u8 x;																							\
		/**
		 * @var u8			y
		 * @brief			Printing row
		 * @memberof		OptionsSelector
		 */																								\
		u8 y;																							\
		/**
		 * @var u8			cols
		 * @brief			Number of columns per page
		 * @memberof		OptionsSelector
		 */																								\
		u8 cols;																						\
		/**
		 * @var u8			rows
		 * @brief			Number of rows per page
		 * @memberof		OptionsSelector
		 */																								\
		u8 rows;																						\
		/**
		 * @var u8			columnWidth
		 * @brief			Width of a column (in chars)
		 * @memberof		OptionsSelector
		 */																								\
		u8 columnWidth;																					\
		/**
		 * @var int		 	totalOptions
		 * @brief			Total number of options
		 * @memberof		OptionsSelector
		 */																								\
		int totalOptions;																				\
		/**
		 * @var int		 	currentPageIndex
		 * @brief			Current page index
		 * @memberof		OptionsSelector
		 */																								\
		int currentPageIndex;																			\
		/**
		 * @var int		 	currentOptionIndex
		 * @brief			Current option index
		 * @memberof		OptionsSelector
		 */																								\
		int currentOptionIndex;																			\
		/**
		 * @var char*		mark
		 * @brief			Selection mark character
		 * @memberof		OptionsSelector
		 */																								\
		char* mark;																						\
		/**
		 * @var char*		font
		 * @brief			Font to use for printing the OptionsSelector
		 * @memberof		OptionsSelector
		 */																								\
		char* font;																						\

// declare the optionsselector class
__CLASS(OptionsSelector);

/**
 * An option of the OptionsSelector
 *
 * @memberof	OptionsSelector
 */
typedef struct Option
{
	/// value of Option
	void* value;
	/// OptionType
	u8 type;
	/// callback function to be executed for this menu option
	void (*callback)(Object);
	/// scope of callback function
	Object callbackScope;

} Option;

/**
 * The types of an Option
 *
 * @memberof	OptionsSelector
 */
enum OptionTypes
{
	/// a string
	kString,
	/// an integer
	kInt,
	/// a float
	kFloat
};


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OptionsSelector, u8 cols, u8 rows, char* font);

void OptionsSelector_constructor(OptionsSelector this, u8 cols, u8 rows, char* font);
void OptionsSelector_destructor(OptionsSelector this);

void OptionsSelector_doCurrentSelectionCallback(OptionsSelector this);
void OptionsSelector_setColumnWidth(OptionsSelector this, u8 width);
void OptionsSelector_setMarkCharacter(OptionsSelector this, char* mark);
u8 OptionsSelector_getWidth(OptionsSelector this);
void OptionsSelector_setOptions(OptionsSelector this, VirtualList options);
void OptionsSelector_selectNext(OptionsSelector this);
void OptionsSelector_selectPrevious(OptionsSelector this);
bool OptionsSelector_selectNextColumn(OptionsSelector this);
bool OptionsSelector_selectPreviousColumn(OptionsSelector this);
bool OptionsSelector_setSelectedOption(OptionsSelector this, int optionIndex);
int OptionsSelector_getSelectedOption(OptionsSelector this);
void OptionsSelector_printOptions(OptionsSelector this, u8 x, u8 y);
int OptionsSelector_getNumberOfOptions(OptionsSelector this);


#endif
