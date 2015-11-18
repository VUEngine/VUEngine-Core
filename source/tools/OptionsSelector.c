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
#include <VPUManager.h>
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
#include <ScrollBackground.h>
#include <GameState.h>
#include <Stage.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define OptionsSelector_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* pages */																	\
	VirtualList pages;															\
																				\
	/* current page */															\
	VirtualNode currentPage;													\
																				\
	/* current option */														\
	VirtualNode currentOption;													\
																				\
	/* current page index */													\
	int currentPageIndex;														\
																				\
	/* current option index */													\
	int currentOptionIndex;														\
																				\
	/* printing column */														\
	int x;																		\
																				\
	/* printing row */															\
	int y;																		\
																				\
	/* cols per page */															\
	int cols;																	\
																				\
	/* rows per page */															\
	int rows;																	\
																				\
	/* total options count */													\
	int totalOptions;															\
																				\
	/* mark symbol */															\
	char* mark;																	\
																				\
	/* output type */															\
	int type;																	\

// define the OptionsSelector
__CLASS_DEFINITION(OptionsSelector, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void OptionsSelector_flushPages(OptionsSelector this);
static void OptionsSelector_printSelectorMark(OptionsSelector this, char* mark);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OptionsSelector, int cols, int rows, char* mark, int type)
__CLASS_NEW_END(OptionsSelector, cols, rows, mark, type);

// class's constructor
void OptionsSelector_constructor(OptionsSelector this, int cols, int rows, char* mark, int type)
{
	ASSERT(this, "OptionsSelector::constructor: null this");

	__CONSTRUCT_BASE();

	this->pages = NULL;
	this->currentPage = NULL;
	this->currentOption = NULL;
	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
	this->x = 0;
	this->y = 0;
	this->cols = 0 < cols && cols <= __SCREEN_WIDTH >> 3 / 4 ? cols : 1;
	this->rows = 0 < rows && rows <= __SCREEN_WIDTH >> 3 ? rows : __SCREEN_HEIGHT >> 3;
	this->totalOptions = 0;
	this->mark = mark;
	this->type = type;
}

// class's destructor
void OptionsSelector_destructor(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::destructor: null this");

	OptionsSelector_flushPages(this);

	// allow a new construct
	__DESTROY_BASE;
}

static void OptionsSelector_flushPages(OptionsSelector this)
{
	ASSERT(this, "OptionsSelector::flushPages: null this");

	if(this->pages)
	{
		VirtualNode node = VirtualList_begin(this->pages);

		for(; node; node = VirtualNode_getNext(node))
		{
			ASSERT(VirtualNode_getData(node), "flushPages: null node data");
			__DELETE(VirtualNode_getData(node));
		}

		__DELETE(this->pages);
	}

	this->pages = NULL;
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

	VirtualNode node = VirtualList_begin(optionsNames);

	ASSERT(VirtualList_getSize(optionsNames), "OptionsSelector::setOptions: empty optionsNames");
	ASSERT(VirtualList_getSize(optionsNames), "OptionsSelector::setOptions: empty options");

	if(0 < VirtualList_getSize(optionsNames))
	{
		int page = 0;

		for(; page < numberOfPages && node; page++)
		{
			VirtualList options = __NEW(VirtualList);

			int counter = 0;
			for(; node && counter < optionsPerPage; counter++, node = VirtualNode_getNext(node))
			{
				VirtualList_pushBack(options, (const char*)VirtualNode_getData(node));
			}

			VirtualList_pushBack(this->pages, options);
		}

		this->currentPage = VirtualList_begin(this->pages);
		ASSERT(VirtualList_getSize(this->pages), "OptionsSelector::setOptions: empty pages");

		this->currentOption = this->currentPage ? VirtualList_begin(__GET_CAST(VirtualList, VirtualNode_getData(this->currentPage))) : NULL;
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

		this->currentOption = VirtualNode_getNext(this->currentOption);
		this->currentOptionIndex++;

		if(!this->currentOption)
		{
			this->currentPage = VirtualNode_getNext(this->currentPage);
			this->currentPageIndex++;

			if(!this->currentPage)
			{
				this->currentPage = VirtualList_begin(this->pages);
				this->currentPageIndex = 0;
				this->currentOptionIndex = 0;
			}

			this->currentOption = VirtualList_begin(__GET_CAST(VirtualList, VirtualNode_getData(this->currentPage)));
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
				this->currentPage = VirtualList_end(this->pages);
				this->currentPageIndex = VirtualList_getSize(this->pages) - 1;
				this->currentOptionIndex = this->totalOptions - 1;
			}

			this->currentOption = VirtualList_end(__GET_CAST(VirtualList, VirtualNode_getData(this->currentPage)));
			ASSERT(this->currentOption, "selectPrevious: current option data");

			OptionsSelector_showOptions(this, this->x, this->y);
		}

		OptionsSelector_printSelectorMark(this, this->mark);
	}
}

// retrieve selected options name
int OptionsSelector_getSelectedOption(OptionsSelector this)
{
	return this->currentOptionIndex;
}

// show options
void OptionsSelector_showOptions(OptionsSelector this, int x, int y)
{
	ASSERT(this, "OptionsSelector::showOptions: null this");

	if(this->currentPage && 0 < VirtualList_getSize(__GET_CAST(VirtualList, VirtualNode_getData(this->currentPage))))
	{
		this->x = 0 <= x && x <= __SCREEN_WIDTH >> 3 ? x : 0;
		this->y = 0 <= y && y <= __SCREEN_HEIGHT >> 3 ? y : 0;

		ASSERT(this->currentPage, "showOptions: currentPage");
		VirtualNode node = VirtualList_begin(__GET_CAST(VirtualList, VirtualNode_getData(this->currentPage)));

		int i = 0;
		for(; i + y < (__SCREEN_HEIGHT >> 3); i++)
		{
			int j = 0;
			for(; ((__SCREEN_WIDTH >> 3) >= (x + j)); j++)
			{
				Printing_text(Printing_getInstance(), " ", x + j, y + i, NULL);
			}
		}

		int counter = 0;
		
		for(; node; node = VirtualNode_getNext(node))
		{
			if(y <= __SCREEN_WIDTH >> 3)
			{
				ASSERT(node, "showOptions: push null node");
				ASSERT(VirtualNode_getData(node), "showOptions: push null node data");

				switch(this->type)
				{
					case kString:
						Printing_text(Printing_getInstance(), (char*)VirtualNode_getData(node), x + 1, y, NULL);
						break;

					case kInt:
						Printing_int(Printing_getInstance(), *((int*)VirtualNode_getData(node)), x + 1, y, NULL);
						break;

					case kFloat:
						Printing_float(Printing_getInstance(), *((float*)VirtualNode_getData(node)), x + 1, y, NULL);
						break;
						
					case kCount:
						Printing_int(Printing_getInstance(), counter++, x + 1, y, NULL);
						break;
				}
			}

			if(++y >= this->rows + this->y || y > (__SCREEN_HEIGHT >> 3))
			{
				y = this->y;
				x += (__SCREEN_WIDTH >> 3) / this->cols;
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

		int indexOption = this->currentOptionIndex - this->currentPageIndex * VirtualList_getSize(__GET_CAST(VirtualList, VirtualList_front(this->pages)));
		int optionColumn = (int)(indexOption / this->rows);
		int optionRow = indexOption - optionColumn * this->rows;
		optionColumn = (__SCREEN_WIDTH >> 3) / this->cols * optionColumn;
		Printing_text(Printing_getInstance(), mark, this->x + optionColumn, this->y + optionRow, NULL);
	}
}