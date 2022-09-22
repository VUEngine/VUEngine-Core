/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OPTIONS_SELECTOR_H_
#define OPTIONS_SELECTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
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
	void (*callback)(ListenerObject);
	/// scope of callback function
	ListenerObject callbackScope;

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

enum OptionsAlignment
{
	kOptionsAlignLeft = 0,
	kOptionsAlignCenter,
	kOptionsAlignRight
};


//---------------------------------------------------------------------------------------------------------
//											CLASS' DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Utility class to render a menu
/// @ingroup tools
class OptionsSelector : ListenerObject
{
	// List of pages, each being a VirtualLists of Options
	VirtualList pages;
	// Current page node
	VirtualNode currentPage;
	// Current option node
	VirtualNode currentOption;
	// Printing column
	int8 x;
	int8 optionsLength;
	// Printing row
	int8 y;
	uint32 alignment;
	int8 spacing;
	// Number of columns per page
	uint16 cols;
	// Number of rows per page
	uint16 rows;
	// Width of a column (in chars)
	uint8 columnWidth;
	// Total number of options
	int32 totalOptions;
	// Current page index
	int32 currentPageIndex;
	// Current option index
	int32 currentOptionIndex;
	// Selection mark character
	char* leftMark;
	char* rightMark;
	// Font to use for printing the OptionsSelector
	char* font;

	/// @publicsection
	void constructor(uint16 cols, uint16 rows, char* font, char* leftMark, char* rightMark);
	void doCurrentSelectionCallback();
	void setColumnWidth(uint8 width);
	void setMarkCharacters(char* leftMark, char* rightMark);
	uint8 getWidth();
	void setOptions(VirtualList options);
	void selectNext();
	void selectPrevious();
	bool selectNextColumn();
	bool selectPreviousColumn();
	bool setSelectedOption(int32 optionIndex);
	int32 getSelectedOption();
	void printOptions(uint8 x, uint8 y, uint32 alignment, uint8 spacing);
	int32 getNumberOfOptions();
}


#endif
