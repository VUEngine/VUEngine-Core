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
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define OptionsSelector_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* pages */																						\
        VirtualList pages;																				\
        /* current page */																				\
        VirtualNode currentPage;																		\
        /* current option */																			\
        VirtualNode currentOption;																		\
        /* printing column */																			\
        u8 x;																							\
        /* printing row */																				\
        u8 y;																							\
        /* cols per page */																				\
        u8 cols;																						\
        /* rows per page */																				\
        u8 rows;																						\
        /* width of a column */																			\
        u8 columnWidth;																					\
        /* output type */																				\
        u8 type;																						\
        /* total options count */																		\
        int totalOptions;																				\
        /* current page index */																		\
        int currentPageIndex;																			\
        /* current option index */																		\
        int currentOptionIndex;																			\
        /* mark symbol */																				\
        char* mark;																						\

// define the OptionsSelector
__CLASS_DEFINITION(OptionsSelector, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void OptionsSelector_flushPages(OptionsSelector this);
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OptionsSelector, u8 cols, u8 rows, char* mark, u8 type)
__CLASS_NEW_END(OptionsSelector, cols, rows, mark, type);

// class's constructor
void OptionsSelector_constructor(OptionsSelector this, u8 cols, u8 rows, char* mark, u8 type)
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
	this->columnWidth = (__SCREEN_WIDTH >> 3) / this->cols;
}

// class's destructor
void OptionsSelector_destructor(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::destructor: null this");

	OptionsSelector_flushPages(this);

	// allow a new construct
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

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

// set column width
void OptionsSelector_setColumnWidth(OptionsSelector this, u8 width)
{
    // add space for selection mark
    width++;

    if((0 < width) && (width <= (__SCREEN_WIDTH >> 3)))
    {
	    this->columnWidth = width;
    }
}

// get total width
u8 OptionsSelector_getWidth(OptionsSelector this)
{
    return this->columnWidth * this->cols;
}

// set options
void OptionsSelector_setOptions(OptionsSelector this, VirtualList optionsNames)
{
	ASSERT(this, "OptionsSelector::setOptions: null this");
	ASSERT(optionsNames, "OptionsSelector::setOptions: null optionsNames");

	OptionsSelector_flushPages(this);

	this->pages = __NEW(VirtualList);

	this->totalOptions = VirtualList_getSize(optionsNames);

	int optionsPerPage = this->cols * this->rows;
	int numberOfPages = (int)(this->totalOptions / optionsPerPage);
	numberOfPages += 0 < this->totalOptions % optionsPerPage ? 1 : 0;

	VirtualNode node = optionsNames->head;

	ASSERT(VirtualList_getSize(optionsNames), "OptionsSelector::setOptions: empty optionsNames");
	ASSERT(VirtualList_getSize(optionsNames), "OptionsSelector::setOptions: empty options");

	if(0 < VirtualList_getSize(optionsNames))
	{
		int page = 0;

		for(; page < numberOfPages && node; page++)
		{
			VirtualList options = __NEW(VirtualList);

			int counter = 0;
			for(; node && counter < optionsPerPage; counter++, node = node->next)
			{
				VirtualList_pushBack(options, (const char*)node->data);
			}

			VirtualList_pushBack(this->pages, options);
		}

		this->currentPage = this->pages->head;
		ASSERT(VirtualList_getSize(this->pages), "OptionsSelector::setOptions: empty pages");

		this->currentOption = this->currentPage ? (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head : NULL;
	}

	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
}

// select next option
void OptionsSelector_selectNext(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::selectNext: null this");

	if(this->currentOption)
	{
		OptionsSelector_printSelectorMark(this, " ");

		this->currentOption = this->currentOption->next;
		this->currentOptionIndex++;

		if(!this->currentOption)
		{
			this->currentPage = this->currentPage->next;
			this->currentPageIndex++;

			if(!this->currentPage)
			{
				this->currentPage = this->pages->head;
				this->currentPageIndex = 0;
				this->currentOptionIndex = 0;
			}

			this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head;
			ASSERT(this->currentOption, "selectNext: null current option");

			OptionsSelector_showOptions(this, this->x, this->y);
		}

		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

// select previous option
void OptionsSelector_selectPrevious(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::selectPrevious: null this");

	if(this->currentOption)
	{
		OptionsSelector_printSelectorMark(this, " ");

		this->currentOption = VirtualNode_getPrevious(this->currentOption);
		this->currentOptionIndex--;

		if(!this->currentOption)
		{
			this->currentPage = VirtualNode_getPrevious(this->currentPage);
			this->currentPageIndex--;

			if(!this->currentPage)
			{
				this->currentPage = this->pages->tail;
				this->currentPageIndex = VirtualList_getSize(this->pages) - 1;
				this->currentOptionIndex = this->totalOptions - 1;
			}

			this->currentOption = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->tail;
			ASSERT(this->currentOption, "selectPrevious: current option data");

			OptionsSelector_showOptions(this, this->x, this->y);
		}

		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

// set selected option
void OptionsSelector_setSelectedOption(OptionsSelector this, int optionIndex)
{
	// check if desired option index is within bounds
	if(optionIndex >= 0 && optionIndex <= this->totalOptions)
	{
		if(optionIndex < this->currentOptionIndex)
		{
			// if desired option is smaller than current option, select previous until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector_selectNext(this);
			}
		}
		else if(optionIndex > this->currentOptionIndex)
		{
			// if desired option is larger than current option, select next until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector_selectPrevious(this);
			}
		}
	}
}

// retrieve selected options name
int OptionsSelector_getSelectedOption(OptionsSelector this)
{
	return this->currentOptionIndex;
}

// show options
void OptionsSelector_showOptions(OptionsSelector this, u8 x, u8 y)
{
	ASSERT(this, "OptionsSelector::showOptions: null this");

	if(this->currentPage && 0 < VirtualList_getSize(__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage))))
	{
		this->x = (x < (__SCREEN_WIDTH >> 3)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT >> 3)) ? y : 0;

		ASSERT(this->currentPage, "showOptions: currentPage");
		VirtualNode node = (__SAFE_CAST(VirtualList, VirtualNode_getData(this->currentPage)))->head;

		int i = 0;
		for(; i + y < (__SCREEN_HEIGHT >> 3); i++)
		{
			int j = 0;
			for(; ((this->columnWidth * this->cols) >= (x + j)); j++)
			{
				Printing_text(Printing_getInstance(), " ", x + j, y + i, NULL);
			}
		}

		int counter = 0;

		for(; node; node = node->next)
		{
            ASSERT(node, "showOptions: push null node");
            ASSERT(node->data, "showOptions: push null node data");

            switch(this->type)
            {
                case kString:
                    Printing_text(Printing_getInstance(), (char*)node->data, x + 1, y, NULL);
                    break;

                case kInt:
                    Printing_int(Printing_getInstance(), *((int*)node->data), x + 1, y, NULL);
                    break;

                case kFloat:
                    Printing_float(Printing_getInstance(), *((float*)node->data), x + 1, y, NULL);
                    break;

                case kCount:
                    Printing_int(Printing_getInstance(), counter++, x + 1, y, NULL);
                    break;
            }

			if((++y >= (this->rows + this->y)) || (y >= (__SCREEN_HEIGHT >> 3)))
			{
				y = this->y;
				x += this->columnWidth;
			}
		}

		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark)
{
	ASSERT(this, "OptionsSelector::printSelectorMark: null this");

	if(this->currentPage)
	{
		ASSERT(this->currentPage, "printSelectorMark: current page");
		ASSERT(VirtualNode_getData(this->currentPage), "printSelectorMark: null current data");

		int indexOption = this->currentOptionIndex - this->currentPageIndex * VirtualList_getSize(__SAFE_CAST(VirtualList, VirtualList_front(this->pages)));
		int optionColumn = (int)(indexOption / this->rows);
		int optionRow = indexOption - optionColumn * this->rows;
		optionColumn = this->columnWidth * optionColumn;
		Printing_text(Printing_getInstance(), mark, this->x + optionColumn, this->y + optionRow, NULL);
	}
}
