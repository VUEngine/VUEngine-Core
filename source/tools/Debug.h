/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef DEBUG_H_
#define DEBUG_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Tool.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * For debugging
 *
 * @memberof	Debug
 */
typedef struct ClassSizeData
{
	/// size
	int32 (*classSizeFunction)(void);
	/// name
	char* name;

} ClassSizeData;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup tools
singleton class Debug : Tool
{
	// pages
	VirtualList pages;
	// sub pages
	VirtualList subPages;
	// current page
	VirtualNode currentPage;
	// current sub page
	VirtualNode currentSubPage;
	// current layer
	int8 currentSprite;
	// part of bgmap memory current viewed
	uint8 viewedMapPart;
	// current bgmap
	int32 bgmapSegment;
	// current obj segment
	int32 objectSegment;
	// current char segment
	int32 charSegment;
	// current page in sram inspector
	int32 sramPage;
	// update function pointer
	void (*update)(void *);

	/// @publicsection
	static Debug getInstance();
	override void update();
	override void render();
	override void show();
	override void hide();
	override void processUserInput(uint16 pressedKey);
}

#endif
