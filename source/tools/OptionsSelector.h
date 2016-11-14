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

#ifndef OPTIONS_SELECTOR_H_
#define OPTIONS_SELECTOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __OPTIONS_SELECT_MAX_COLS		__SCREEN_WIDTH >> 5
#define __OPTIONS_SELECT_MAX_ROWS		__SCREEN_HEIGHT >> 3


//---------------------------------------------------------------------------------------------------------
// 											CLASS' DECLARATION
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
		VirtualList pages;																				\
		VirtualNode currentPage;																		\
		VirtualNode currentOption;																		\
		u8 x;																							\
		u8 y;																							\
		u8 cols;																						\
		u8 rows;																						\
		u8 columnWidth;																					\
		int totalOptions;																				\
		int currentPageIndex;																			\
		int currentOptionIndex;																			\
		char* mark;																						\
		char* font;																						\

// declare the optionsselector class
__CLASS(OptionsSelector);

typedef struct Option
{
	void* value;
    u8 type;
	void (*callback)(Object);
	Object callbackScope;

} Option;

enum OptionTypes
{
	kString,
	kInt,
	kFloat
};


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
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


#endif
