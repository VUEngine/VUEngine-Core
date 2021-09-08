/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

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
	uint8 type;
	/// callback function to be executed for this menu option
	void (*callback)(Object);
	/// scope of callback function
	Object callbackScope;

} Option;


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

/**
 * The types of an Option
 *
 * @memberof	OptionsSelector
 */
enum OptionTypes
{
	/// a string
	kString,
	/// a float
	kFloat,
	/// an integer
	kInt,
	/// a short integer
	kShortInt,
	/// a char
	kChar
};


//---------------------------------------------------------------------------------------------------------
//											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Utility class to render a menu
/// @ingroup tools
class OptionsSelector : Object
{
	// List of pages, each being a VirtualLists of Options
	VirtualList pages;
	// Current page node
	VirtualNode currentPage;
	// Current option node
	VirtualNode currentOption;
	// Printing column
	uint8 x;
	// Printing row
	uint8 y;
	// Number of columns per page
	uint16 cols;
	// Number of rows per page
	uint16 rows;
	// Width of a column (in chars)
	uint8 columnWidth;
	// Total number of options
	int totalOptions;
	// Current page index
	int currentPageIndex;
	// Current option index
	int currentOptionIndex;
	// Selection mark character
	char* mark;
	// Font to use for printing the OptionsSelector
	char* font;

	/// @publicsection
	void constructor(uint16 cols, uint16 rows, char* font);
	void doCurrentSelectionCallback();
	void setColumnWidth(uint8 width);
	void setMarkCharacter(char* mark);
	uint8 getWidth();
	void setOptions(VirtualList options);
	void selectNext();
	void selectPrevious();
	bool selectNextColumn();
	bool selectPreviousColumn();
	bool setSelectedOption(int optionIndex);
	int getSelectedOption();
	void printOptions(uint8 x, uint8 y);
	int getNumberOfOptions();
}


#endif
