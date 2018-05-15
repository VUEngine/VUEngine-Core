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

#ifndef DEBUG_H_
#define DEBUG_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods

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


singleton class Debug : Object
{
	/**
	 * @var GameState	gameState
	 * @brief			current in game state
	 * @memberof		Debug
	 */
	GameState gameState;
	/**
	 * @var VirtualList	pages
	 * @brief			pages
	 * @memberof		Debug
	 */
	VirtualList pages;
	/**
	 * @var VirtualList	subPages
	 * @brief			sub pages
	 * @memberof		Debug
	 */
	VirtualList subPages;
	/**
	 * @var VirtualNode	currentPage
	 * @brief			current page
	 * @memberof		Debug
	 */
	VirtualNode currentPage;
	/**
	 * @var VirtualNode	currentSubPage
	 * @brief			current sub page
	 * @memberof		Debug
	 */
	VirtualNode currentSubPage;
	/**
	 * @var u8			currentLayer
	 * @brief			current layer
	 * @memberof		Debug
	 */
	u8 currentLayer;
	/**
	 * @var int			bgmapSegment
	 * @brief			current bgmap
	 * @memberof		Debug
	 */
	int bgmapSegment;
	/**
	 * @var int			objectSegment
	 * @brief			current obj segment
	 * @memberof		Debug
	 */
	int objectSegment;
	/**
	 * @var int			charSegment
	 * @brief			current char segment
	 * @memberof		Debug
	 */
	int charSegment;
	/**
	 * @var int			sramPage
	 * @brief			current page in sram inspector
	 * @memberof		Debug
	 */
	int sramPage;
	/**
	 * @var PixelVector		mapDisplacement
	 * @brief			window to look into bgmap memory
	 * @memberof		Debug
	 */
	PixelVector mapDisplacement;
	/**
	 * @var void 		(*update)(void	*)
	 * @brief			update function pointer
	 * @memberof		Debug
	 */
	void (*update)(void *);

	static Debug getInstance();
	void update(Debug this);
	void render(Debug this);
	void show(Debug this, GameState gameState);
	void hide(Debug this);
	void processUserInput(Debug this, u16 pressedKey);
}


#endif
