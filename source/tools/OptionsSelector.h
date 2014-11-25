/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef OPTIONS_SELECTOR_H_
#define OPTIONS_SELECTOR_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <VirtualList.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// declare the virtual methods
#define OptionsSelector_METHODS								\
		Object_METHODS										\


// declare the virtual methods which are redefined
#define OptionsSelector_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)							\


// declare a OptionsSelector
__CLASS(OptionsSelector);


// for debugging
typedef struct Option {
	
	char* name;
	int (*classSizeFunction)(void);
	
}ClassSizeData;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(OptionsSelector, __PARAMETERS(int cols, int rows));

// class's constructor
void OptionsSelector_constructor(OptionsSelector this, int cols, int rows);

// class's destructor
void OptionsSelector_destructor(OptionsSelector this);

// set options
void OptionsSelector_setOptions(OptionsSelector this, VirtualList optionsNames);

// select next option
void OptionsSelector_selectNext(OptionsSelector this);

// select previous option
void OptionsSelector_selectPrevious(OptionsSelector this);

// retrieve selected options name
int OptionsSelector_getSelectedOption(OptionsSelector this);

// set options
void OptionsSelector_showOptions(OptionsSelector this, int x, int y);

#endif /*CLOCK_H_*/
