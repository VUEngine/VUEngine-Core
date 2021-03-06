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
	int (*classSizeFunction)(void);
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
	s8 currentSprite;
	// part of bgmap memory current viewed
	u8 viewedMapPart;
	// current bgmap
	int bgmapSegment;
	// current obj segment
	int objectSegment;
	// current char segment
	int charSegment;
	// current page in sram inspector
	int sramPage;
	// update function pointer
	void (*update)(void *);

	/// @publicsection
	static Debug getInstance();
	override void update();
	override void render();
	override void show();
	override void hide();
	override void processUserInput(u16 pressedKey);
}

#endif
