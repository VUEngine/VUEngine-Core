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
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define OptionsSelector_METHODS													\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define OptionsSelector_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)											\

// declare a OptionsSelector
__CLASS(OptionsSelector);

// for debugging
typedef struct Option
{
	int (*classSizeFunction)(void);
	char* name;

}ClassSizeData;

enum OptionTypes
{
	kString,
	kInt,
	kFloat,
	kCount
};


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OptionsSelector, int cols, int rows, char* mark, int type);

void OptionsSelector_constructor(OptionsSelector this, int cols, int rows, char* mark, int type);
void OptionsSelector_destructor(OptionsSelector this);
void OptionsSelector_setOptions(OptionsSelector this, VirtualList optionsNames);
void OptionsSelector_selectNext(OptionsSelector this);
void OptionsSelector_selectPrevious(OptionsSelector this);
int OptionsSelector_getSelectedOption(OptionsSelector this);
void OptionsSelector_showOptions(OptionsSelector this, int x, int y);


#endif