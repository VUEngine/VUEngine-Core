/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Tool.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A struct to map a class size function to a name
/// @memberof Debug
typedef struct ClassSizeData
{
	/// Class' method
	int32 (*classSizeFunction)(void);

	/// Class' name
	char* name;

} ClassSizeData;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Debug
///
/// Inherits from Tool
///
/// Implements a tool that displays various debug information.
singleton class Debug : Tool
{
	/// Degug pages
	VirtualList pages;

	/// Debug sub pages
	VirtualList subPages;

	/// Current page's node
	VirtualNode currentPage;

	/// Current sub page's node
	VirtualNode currentSubPage;

	/// Current sprite's index
	int8 spriteIndex;

	/// Currently displayed BGMAP segment
	int8 bgmapSegment;

	/// Currently displayed part of BGMAP memory
	uint8 bgmapSegmentDiplayedSection;

	/// Currently displayed OBJECT segment
	int32 objectSegment;

	/// Currently displayed CHAR memory segment
	int32 charSegment;

	/// Currently displayed SRAM page
	int32 sramPage;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return Debug singleton
	static Debug getInstance();

	/// Update the tool's state.
	override void update();

	/// Show the tool.
	override void show();

	/// Hide the tool.
	override void hide();

	/// Process the provided user pressed key.
	/// @param pressedKey: User pressed key
	override void processUserInput(uint16 pressedKey);
}

#endif
