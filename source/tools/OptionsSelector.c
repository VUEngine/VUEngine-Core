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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
#include <InanimatedInGameEntity.h>
#include <Container.h>
#include <Entity.h>
#include <InGameEntity.h>
#include <GameState.h>
#include <Stage.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class           OptionsSelector
 * @extends         Object
 * @brief           Tool to render a menu
 *
 * @var VirtualList pages
 * @brief           List of pages, each being a VirtualLists of options
 * @memberof        OptionsSelector
 *
 * @var VirtualNode currentPage
 * @brief           Current page node
 * @memberof        OptionsSelector
 *
 * @var VirtualNode currentOption
 * @brief           Current option node
 * @memberof        OptionsSelector
 *
 * @var u8          x
 * @brief           Printing column
 * @memberof        OptionsSelector
 *
 * @var u8          y
 * @brief           Printing row
 * @memberof        OptionsSelector
 *
 * @var u8          cols
 * @brief           Number of columns per page
 * @memberof        OptionsSelector
 *
 * @var u8          rows
 * @brief           Number of rows per page
 * @memberof        OptionsSelector
 *
 * @var u8          columnWidth
 * @brief           Width of a column (in chars)
 * @memberof        OptionsSelector
 *
 * @var u8          type
 * @brief           Output type
 * @memberof        OptionsSelector
 *
 * @var int         totalOptions
 * @brief           Total number of options
 * @memberof        OptionsSelector
 *
 * @var int         currentPageIndex
 * @brief           Current page index
 * @memberof        OptionsSelector
 *
 * @var int         currentOptionIndex
 * @brief           Current option index
 * @memberof        OptionsSelector
 *
 * @var char*       mark
 * @brief           Selection mark character
 * @memberof        OptionsSelector
 */

__CLASS_DEFINITION(OptionsSelector, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(Printing);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void OptionsSelector_flushPages(OptionsSelector this);
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark);


//---------------------------------------------------------------------------------------------------------
// 												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OptionsSelector, u8 cols, u8 rows, char* mark, u8 type, char* font)
__CLASS_NEW_END(OptionsSelector, cols, rows, mark, type, font);

/**
 * Class constructor
 *
 * @memberof    OptionsSelector
 * @private
 *
 * @param this  Function scope
 * @param cols  Number of columns
 * @param rows  Number of rows
 * @param mark  Character to use for selector mark
 * @param type  Options type
 */
void OptionsSelector_constructor(OptionsSelector this, u8 cols, u8 rows, char* mark, u8 type, char* font)
{
	ASSERT(this, "OptionsSelector::constructor: null this");

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
	this->mark = mark;
	this->type = type;
	this->font = font;
	this->columnWidth = (__SCREEN_WIDTH >> 3) / this->cols;
}

/**
 * Class destructor
 *
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 */
void OptionsSelector_destructor(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::destructor: null this");

	OptionsSelector_flushPages(this);

	// allow a new construct
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Flush internal list of pages and options
 *
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 */
static void OptionsSelector_flushPages(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::flushPages: null this");

	if(this->pages)
	{
		VirtualNode node = this->pages->head;

		for(; node; node = node->next)
		{
			ASSERT(node->data, "flushPages: null node data");
			__DELETE(node->data);
		}

		__DELETE(this->pages);
	}

	this->pages = NULL;
}

/**
 * Set column width
 *
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 * @param width Width (in font chars)
 */
void OptionsSelector_setColumnWidth(OptionsSelector this, u8 width)
{
    FontData* fontData = Printing_getFontByName(Printing_getInstance(), this->font);

    // add space for selection mark, consider font width
    width = ((width + 1) * fontData->fontDefinition->fontSize.x);

	if((0 < width) && (width <= (__SCREEN_WIDTH >> 3)))
	{
		this->columnWidth = width;
	}
}

/**
 * Get total width of options selector (in chars)
 *
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 *
 * @return      Total width of options selector (in chars)
 */
u8 OptionsSelector_getWidth(OptionsSelector this)
{
	return this->columnWidth * this->cols;
}

/**
 * Set options
 *
 * @memberof        OptionsSelector
 * @public
 *
 * @param this      Function scope
 * @param options   List of options
 */
void OptionsSelector_setOptions(OptionsSelector this, VirtualList options)
{
	ASSERT(this, "OptionsSelector::setOptions: null this");
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
				VirtualList_pushBack(pageOptions, (const char*)node->data);
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
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 */
void OptionsSelector_selectNext(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::selectNext: null this");

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
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 */
void OptionsSelector_selectPrevious(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::selectPrevious: null this");

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
 * @memberof            OptionsSelector
 * @public
 *
 * @param this          Function scope
 * @param optionIndex   Index of desired option
 *
 * @return              Boolean that indicated whether a new option was selected
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
 * @memberof            OptionsSelector
 * @public
 *
 * @param this          Function scope
 * @param optionIndex   Index of desired option
 *
 * @return              Index of selected option
 */
int OptionsSelector_getSelectedOption(OptionsSelector this)
{
	return this->currentOptionIndex;
}

/**
 * Print the list of options
 *
 * @memberof    OptionsSelector
 * @public
 *
 * @param this  Function scope
 * @param x     X coordinate to start printing at (in chars)
 * @param y     Y coordinate to start printing at (in chars)
 */
void OptionsSelector_printOptions(OptionsSelector this, u8 x, u8 y)
{
	ASSERT(this, "OptionsSelector::printOptions: null this");

	if(this->currentPage && 0 < VirtualList_getSize(__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage))))
	{
	    FontData* fontData = Printing_getFontByName(Printing_getInstance(), this->font);

		this->x = (x < (__SCREEN_WIDTH >> 3)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT >> 3)) ? y : 0;

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

		int counter = 0;

		for(; node; node = node->next)
		{
			ASSERT(node, "printOptions: push null node");
			ASSERT(node->data, "printOptions: push null node data");

			switch(this->type)
			{
				case kString:
					Printing_text(Printing_getInstance(), (char*)node->data, x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;

				case kInt:
					Printing_int(Printing_getInstance(), *((int*)node->data), x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;

				case kFloat:
					Printing_float(Printing_getInstance(), *((float*)node->data), x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;

				case kCount:
					Printing_int(Printing_getInstance(), counter++, x + fontData->fontDefinition->fontSize.x, y, this->font);
					break;
			}

            y += fontData->fontDefinition->fontSize.y;
			if((y >= (this->rows * fontData->fontDefinition->fontSize.y + this->y)) || (y >= (__SCREEN_HEIGHT >> 3)))
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
 * @memberof    OptionsSelector
 * @private
 *
 * @param this  Function scope
 * @param mark  The character to use
 */
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark)
{
	ASSERT(this, "OptionsSelector::printSelectorMark: null this");

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
