/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <OptionsSelector.h>
#include <Game.h>
#include <Optics.h>
#include <FrameRate.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <CharSetManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <HardwareManager.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Printing.h>
#include <MiscStructs.h>

#include <Clock.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <CharSet.h>
#include <Sprite.h>
#include <Texture.h>
#include <Body.h>

#include <Body.h>
#include <Circle.h>
#include <Cuboid.h>
#include <Shape.h>
#include <Actor.h>
#include <Entity.h>
#include <Container.h>
#include <Entity.h>
#include <Entity.h>
#include <GameState.h>
#include <Stage.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	OptionsSelector
 * @extends Object
 * @ingroup tools
 * @brief	Utility class to render a menu
 */
__CLASS_DEFINITION(OptionsSelector, Object);
__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(Printing);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void OptionsSelector_flushPages(OptionsSelector this);
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark);


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OptionsSelector, u8 cols, u8 rows, char* font)
__CLASS_NEW_END(OptionsSelector, cols, rows, font);

/**
 * Class constructor
 *
 * @memberof	OptionsSelector
 * @private
 *
 * @param this	Function scope
 * @param cols	Number of columns
 * @param rows	Number of rows
 * @param font	Font to use for printing selector
 */
void OptionsSelector_constructor(OptionsSelector this, u8 cols, u8 rows, char* font)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->pages = NULL;
	this->currentPage = NULL;
	this->currentOption = NULL;
	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
	this->x = 0;
	this->y = 0;
	this->cols = ((0 < cols) && (cols <= __OPTIONS_SELECT_MAX_COLS)) ? cols : 1;
	this->rows = ((0 < rows) && (rows <= __OPTIONS_SELECT_MAX_ROWS)) ? rows : __OPTIONS_SELECT_MAX_ROWS;
	this->totalOptions = 0;
	this->mark = __CHAR_SELECTOR;
	this->font = font;
	this->columnWidth = (__SCREEN_WIDTH_IN_CHARS) / this->cols;
}

/**
 * Class destructor
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 */
void OptionsSelector_destructor(OptionsSelector this)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::destructor: null this");

	OptionsSelector_flushPages(this);

	// allow a new construct
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Flush internal list of pages and options
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 */
static void OptionsSelector_flushPages(OptionsSelector this)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::flushPages: null this");

	if(this->pages)
	{
		VirtualNode node = this->pages->head;

		for(; node; node = node->next)
		{
			ASSERT(node->data, "flushPages: null node data");

			VirtualNode optionsNode = (__SAFE_CAST(VirtualList, node->data))->head;

			for(; optionsNode; optionsNode = optionsNode->next)
			{
				__DELETE_BASIC(optionsNode->data);
			}

			__DELETE(node->data);
		}

		__DELETE(this->pages);
	}

	this->pages = NULL;
}

/**
 * Set character to use as selection mark
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 * @param mark	Selection mark character
 */
void OptionsSelector_setMarkCharacter(OptionsSelector this, char* mark)
{
	this->mark = mark;
}

/**
 * Set column width
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 * @param width Width (in font chars)
 */
void OptionsSelector_setColumnWidth(OptionsSelector this, u8 width)
{
	FontData* fontData = Printing_getFontByName(Printing_getInstance(), this->font);

	// add space for selection mark, consider font width
	width = ((width + 1) * fontData->fontDefinition->fontSize.x);

	if((0 < width) && (width <= (__SCREEN_WIDTH_IN_CHARS)))
	{
		this->columnWidth = width;
	}
}

/**
 * Get total width of options selector (in chars)
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 *
 * @return		Total width of options selector (in chars)
 */
u8 OptionsSelector_getWidth(OptionsSelector this)
{
	return this->columnWidth * this->cols;
}

/**
 * Set options
 *
 * @memberof		OptionsSelector
 * @public
 *
 * @param this		Function scope
 * @param options	List of options
 */
void OptionsSelector_setOptions(OptionsSelector this, VirtualList options)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::setOptions: null this");
	ASSERT(options, "OptionsSelector::setOptions: null options");

	OptionsSelector_flushPages(this);

	this->pages = __NEW(VirtualList);

	this->totalOptions = VirtualList_getSize(options);

	int optionsPerPage = this->cols * this->rows;
	int numberOfPages = (int)(this->totalOptions / optionsPerPage);
	numberOfPages += (0 < (this->totalOptions % optionsPerPage)) ? 1 : 0;

	VirtualNode node = options->head;

	ASSERT(VirtualList_getSize(options), "OptionsSelector::setOptions: empty options");

	if(0 < VirtualList_getSize(options))
	{
		int page = 0;

		for(; page < numberOfPages && node; page++)
		{
			VirtualList pageOptions = __NEW(VirtualList);

			int counter = 0;
			for(; node && counter < optionsPerPage; counter++, node = node->next)
			{
				VirtualList_pushBack(pageOptions, node->data);
			}

			VirtualList_pushBack(this->pages, pageOptions);
		}

		this->currentPage = this->pages->head;
		ASSERT(VirtualList_getSize(this->pages), "OptionsSelector::setOptions: empty pages");

		this->currentOption = this->currentPage ? (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head : NULL;
	}

	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
}

/**
 * Select next option
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 */
void OptionsSelector_selectNext(OptionsSelector this)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::selectNext: null this");

	if(this->currentOption)
	{
		// remove previous selection mark
		OptionsSelector_printSelectorMark(this, " ");

		// get next option
		this->currentOption = this->currentOption->next;
		this->currentOptionIndex++;

		// if there's no next option on the current page
		if(!this->currentOption)
		{
			// if there's more than 1 page, go to next page
			if(VirtualList_getSize(this->pages) > 1)
			{
				// select next page
				this->currentPage = this->currentPage->next;
				this->currentPageIndex++;

				// if next page does not exist
				if(!this->currentPage)
				{
					this->currentPage = this->pages->head;
					this->currentPageIndex = 0;
					this->currentOptionIndex = 0;
				}

				// get new option
				this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head;
				ASSERT(this->currentOption, "selectNext: null current option");

				// render new page
				OptionsSelector_printOptions(this, this->x, this->y);
			}
			else
			{
				// wrap around and select first option
				this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head;
				this->currentOptionIndex = 0;
			}
		}

		// print new selection mark
		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

/**
 * Select previous option
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 */
void OptionsSelector_selectPrevious(OptionsSelector this)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::selectPrevious: null this");

	if(this->currentOption)
	{
		// remove previous selection mark
		OptionsSelector_printSelectorMark(this, " ");

		// get previous option
		this->currentOption = VirtualNode_getPrevious(this->currentOption);
		this->currentOptionIndex--;

		// if there's no previous option on the current page
		if(!this->currentOption)
		{
			// if there's more than 1 page
			if(VirtualList_getSize(this->pages) > 1)
			{
				// select previous page
				this->currentPage = VirtualNode_getPrevious(this->currentPage);
				this->currentPageIndex--;

				// if previous page does not exist, go to last page
				if(!this->currentPage)
				{
					this->currentPage = this->pages->tail;
					this->currentPageIndex = VirtualList_getSize(this->pages) - 1;
					this->currentOptionIndex = this->totalOptions - 1;
				}

				// get new option
				this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->tail;
				ASSERT(this->currentOption, "selectPrevious: current option data");

				// render new page
				OptionsSelector_printOptions(this, this->x, this->y);
			}
			else
			{
				// wrap around and select last option
				this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->tail;
				this->currentOptionIndex = this->totalOptions - 1;
			}
		}

		// print new selection mark
		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

/**
 * Set selected option
 *
 * @memberof			OptionsSelector
 * @public
 *
 * @param this			Function scope
 * @param optionIndex	Index of desired option
 *
 * @return				Boolean that indicated whether a new option was selected
 */
bool OptionsSelector_setSelectedOption(OptionsSelector this, int optionIndex)
{
	bool changed = false;

	// check if desired option index is within bounds
	if(optionIndex >= 0 && optionIndex <= this->totalOptions)
	{
		if(optionIndex < this->currentOptionIndex)
		{
			// if desired option index is smaller than the current one, select previous until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector_selectNext(this);
				changed = true;
			}
		}
		else if(optionIndex > this->currentOptionIndex)
		{
			// if desired option index is larger than the current one, select next until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector_selectPrevious(this);
				changed = true;
			}
		}
	}

	return changed;
}

/**
 * Retrieve selected options index
 *
 * @memberof			OptionsSelector
 * @public
 *
 * @param this			Function scope
 *
 * @return				Index of selected option
 */
int OptionsSelector_getSelectedOption(OptionsSelector this)
{
	return this->currentOptionIndex;
}

/**
 * Print the list of options
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 * @param x	 X coordinate to start printing at (in chars)
 * @param y	 Y coordinate to start printing at (in chars)
 */
void OptionsSelector_printOptions(OptionsSelector this, u8 x, u8 y)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::printOptions: null this");

	if(this->currentPage && 0 < VirtualList_getSize(__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage))))
	{
		FontData* fontData = Printing_getFontByName(Printing_getInstance(), this->font);

		this->x = (x < (__SCREEN_WIDTH_IN_CHARS)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT_IN_CHARS)) ? y : 0;

		ASSERT(this->currentPage, "printOptions: currentPage");
		VirtualNode node = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head;

		int i = 0;
		for(; i < (this->rows * fontData->fontDefinition->fontSize.y); i++)
		{
			int j = 0;
			for(; (this->columnWidth * this->cols) > j; j++)
			{
				Printing_text(Printing_getInstance(), " ", x + j, y + i, this->font);
			}
		}

		for(; node; node = node->next)
		{
			ASSERT(node, "printOptions: push null node");
			ASSERT(node->data, "printOptions: push null node data");

			Option* option = VirtualNode_getData(node);

			switch(option->type)
			{
				case kString:
					Printing_text(Printing_getInstance(), (char*)option->value, x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;

				case kInt:
					Printing_int(Printing_getInstance(), *((int*)option->value), x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;

				case kFloat:
					Printing_float(Printing_getInstance(), *((float*)option->value), x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;
			}

			y += fontData->fontDefinition->fontSize.y;
			if((y >= (this->rows * fontData->fontDefinition->fontSize.y + this->y)) || (y >= (__SCREEN_HEIGHT_IN_CHARS)))
			{
				y = this->y;
				x += this->columnWidth;
			}
		}

		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

/**
 * Print the selector mark
 *
 * @memberof	OptionsSelector
 * @private
 *
 * @param this	Function scope
 * @param mark	The character to use
 */
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::printSelectorMark: null this");

	if(this->currentPage)
	{
		FontData* fontData = Printing_getFontByName(Printing_getInstance(), this->font);

		ASSERT(this->currentPage, "printSelectorMark: current page");
		ASSERT(VirtualNode_getData(this->currentPage), "printSelectorMark: null current data");

		int indexOption = this->currentOptionIndex - this->currentPageIndex * VirtualList_getSize(__SAFE_CAST(VirtualList, VirtualList_front(this->pages)));
		int optionColumn = (int)(indexOption / this->rows);
		int optionRow = indexOption - optionColumn * this->rows;
		optionColumn = this->columnWidth * optionColumn;

		Printing_text(
			Printing_getInstance(),
			mark,
			this->x + optionColumn,
			this->y + (optionRow * fontData->fontDefinition->fontSize.y),
			this->font
		);
	}
}

/**
 * Execute the callback of the currently selected option
 *
 * @memberof	OptionsSelector
 * @public
 *
 * @param this	Function scope
 */
void OptionsSelector_doCurrentSelectionCallback(OptionsSelector this)
{
	ASSERT(__SAFE_CAST(OptionsSelector, this), "OptionsSelector::doCurrentSelectionCallback: null this");

	Option* option = VirtualNode_getData(this->currentOption);

	if(option->callback && option->callbackScope)
	{
		option->callback(option->callbackScope);
	}
}
