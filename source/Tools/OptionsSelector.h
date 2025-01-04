/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OPTIONS_SELECTOR_H_
#define OPTIONS_SELECTOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __OPTIONS_SELECT_MAX_COLS __SCREEN_WIDTH >> 5
#define __OPTIONS_SELECT_MAX_ROWS __SCREEN_HEIGHT >> 3

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An option for the options selector
/// @memberof OptionsSelector
typedef struct Option
{
	/// Option's value
	void* value;

	/// Option's type
	uint8 type;

	/// Callback function to be executed for this menu option
	void (*callback)(ListenerObject);

	/// Scope of callback function
	ListenerObject scope;

} Option;

/// Types of Options
/// @memberof OptionsSelector
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

/// Types of text alignments
/// @memberof OptionsSelector
enum OptionsAlignment
{
	kOptionsAlignLeft = 0,
	kOptionsAlignCenter,
	kOptionsAlignRight
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Sprite
///
/// Inherits from VisualComponent
///
/// Implements a simple interactive menu.
class OptionsSelector : ListenerObject
{
	/// @protectedsection

	/// List of pages, each being a VirtualLists of Options
	VirtualList pages;

	/// Current page's node
	VirtualNode currentPage;

	/// Current option's node
	VirtualNode currentOption;

	/// Total number of options
	int32 totalOptions;

	/// Current page index
	int32 currentPageIndex;

	// Current option index
	int32 currentOptionIndex;

	/// Screen x coordinate where to print
	int8 x;

	/// Screen y coordinate where to print
	int8 y;

	/// The maximum length of all the options
	int8 optionsLength;

	/// Text alignement
	uint32 alignment;

	/// Text row spacing
	int8 spacing;

	/// Number of columns per page
	uint16 cols;

	/// Number of rows per page
	uint16 rows;

	/// Width of a column (in chars)
	uint8 columnWidth;

	/// Left selection mark character
	char* leftMark;

	/// Right selection mark character
	char* rightMark;

	/// Font to use for printing the options
	char* font;

	/// @publicsection

	/// Class' constructor
	/// @param cols: Number of columns per page
	/// @param rows: Number of rows per page
	/// @param font: Font to use for printing the options
	/// @param leftMark: Left selection mark character
	/// @param rightMark: Right selection mark character
	void constructor(uint16 cols, uint16 rows, char* font, char* leftMark, char* rightMark);

	/// Set the maximum with of each column.
	/// @param width: Columns' with
	void setColumnWidth(uint8 width);

	/// Set the markers characters.
	/// @param leftMark: Left selection mark character
	/// @param rightMark: Right selection mark character
	void setMarkCharacters(char* leftMark, char* rightMark);

	/// Set the available options.
	/// @param options: List of options to set
	void setOptions(VirtualList options);

	/// Set the selection option.
	/// @param optionIndex: Index of the option to select
	bool setSelectedOption(int32 optionIndex);

	/// Select the next option.
	void selectNext();

	/// Select the previous option.
	void selectPrevious();

	/// Retrieve the selected option's index.
	/// @return Index of the selected option
	int32 getSelectedOption();

	/// Retrieve the total number of options.
	/// @return The total number of options
	int32 getNumberOfOptions();

	/// Print the options.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param alignment: Text alignment
	/// @param spacing: Text spacing
	void print(uint8 x, uint8 y, uint32 alignment, uint8 spacing);
}

#endif
