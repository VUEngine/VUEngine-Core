/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
#include <Ball.h>
#include <Box.h>
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

friend class VirtualList;
friend class VirtualNode;
friend class Printing;


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param cols	Number of columns
 * @param rows	Number of rows
 * @param font	Font to use for printing selector
 */
void OptionsSelector::constructor(uint16 cols, uint16 rows, char* font)
{
	Base::constructor();

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
 */
void OptionsSelector::destructor()
{
	OptionsSelector::flushPages(this);

	// allow a new construct
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Flush internal list of pages and options
 */
void OptionsSelector::flushPages()
{
	if(this->pages)
	{
		VirtualNode node = this->pages->head;

		for(; node; node = node->next)
		{
			ASSERT(node->data, "flushPages: null node data");

			VirtualNode optionsNode = (VirtualList::safeCast(node->data))->head;

			for(; optionsNode; optionsNode = optionsNode->next)
			{
				delete optionsNode->data;
			}

			delete node->data;
		}

		delete this->pages;
	}

	this->pages = NULL;
}

/**
 * Set character to use as selection mark
 *
 * @param mark	Selection mark character
 */
void OptionsSelector::setMarkCharacter(char* mark)
{
	this->mark = mark;
}

/**
 * Set column width
 *
 * @param width Width (in font chars)
 */
void OptionsSelector::setColumnWidth(uint8 width)
{
	FontData* fontData = Printing::getFontByName(Printing::getInstance(), this->font);

	// add space for selection mark, consider font width
	width = ((width + 1) * fontData->fontSpec->fontSize.x);

	if((0 < width) && (width <= (__SCREEN_WIDTH_IN_CHARS)))
	{
		this->columnWidth = width;
	}
}

/**
 * Get total width of options selector (in chars)
 *
 * @return		Total width of options selector (in chars)
 */
uint8 OptionsSelector::getWidth()
{
	return this->columnWidth * this->cols;
}

/**
 * Set options
 *
 * @param options	List of options
 */
void OptionsSelector::setOptions(VirtualList options)
{
	ASSERT(options, "OptionsSelector::setOptions: null options");

	OptionsSelector::flushPages(this);

	this->pages = new VirtualList();

	this->totalOptions = VirtualList::getSize(options);

	int32 optionsPerPage = this->cols * this->rows;
	int32 numberOfPages = (int32)(this->totalOptions / optionsPerPage);
	numberOfPages += (0 < (this->totalOptions % optionsPerPage)) ? 1 : 0;

	VirtualNode node = options->head;

	ASSERT(VirtualList::getSize(options), "OptionsSelector::setOptions: empty options");

	if(0 < VirtualList::getSize(options))
	{
		int32 page = 0;

		for(; page < numberOfPages && node; page++)
		{
			VirtualList pageOptions = new VirtualList();

			int32 counter = 0;
			for(; node && counter < optionsPerPage; counter++, node = node->next)
			{
				VirtualList::pushBack(pageOptions, node->data);
			}

			VirtualList::pushBack(this->pages, pageOptions);
		}

		this->currentPage = this->pages->head;
		ASSERT(VirtualList::getSize(this->pages), "OptionsSelector::setOptions: empty pages");

		this->currentOption = this->currentPage ? (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head : NULL;
	}

	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
}

/**
 * Select next option
 */
void OptionsSelector::selectNext()
{
	if(this->currentOption)
	{
		// remove previous selection mark
		OptionsSelector::printSelectorMark(this, " ");

		// get next option
		this->currentOption = this->currentOption->next;
		this->currentOptionIndex++;

		// if there's no next option on the current page
		if(!this->currentOption)
		{
			// if there's more than 1 page, go to next page
			if(VirtualList::getSize(this->pages) > 1)
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
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;
				ASSERT(this->currentOption, "selectNext: null current option");

				// render new page
				OptionsSelector::printOptions(this, this->x, this->y);
			}
			else
			{
				// wrap around and select first option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;
				this->currentOptionIndex = 0;
			}
		}

		// print new selection mark
		OptionsSelector::printSelectorMark(this, this->mark);
	}
}

/**
 * Select previous option
 */
void OptionsSelector::selectPrevious()
{
	if(this->currentOption)
	{
		// remove previous selection mark
		OptionsSelector::printSelectorMark(this, " ");

		// get previous option
		this->currentOption = VirtualNode::getPrevious(this->currentOption);
		this->currentOptionIndex--;

		// if there's no previous option on the current page
		if(!this->currentOption)
		{
			// if there's more than 1 page
			if(VirtualList::getSize(this->pages) > 1)
			{
				// select previous page
				this->currentPage = VirtualNode::getPrevious(this->currentPage);
				this->currentPageIndex--;

				// if previous page does not exist, go to last page
				if(!this->currentPage)
				{
					this->currentPage = this->pages->tail;
					this->currentPageIndex = VirtualList::getSize(this->pages) - 1;
					this->currentOptionIndex = this->totalOptions - 1;
				}

				// get new option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->tail;
				ASSERT(this->currentOption, "selectPrevious: current option data");

				// render new page
				OptionsSelector::printOptions(this, this->x, this->y);
			}
			else
			{
				// wrap around and select last option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->tail;
				this->currentOptionIndex = this->totalOptions - 1;
			}
		}

		// print new selection mark
		OptionsSelector::printSelectorMark(this, this->mark);
	}
}

/**
 * Set selected option
 *
 * @param optionIndex	Index of desired option
 * @return				Boolean that indicated whether a new option was selected
 */
bool OptionsSelector::setSelectedOption(int32 optionIndex)
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
				OptionsSelector::selectNext(this);
				changed = true;
			}
		}
		else if(optionIndex > this->currentOptionIndex)
		{
			// if desired option index is larger than the current one, select next until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector::selectPrevious(this);
				changed = true;
			}
		}
	}

	return changed;
}

/**
 * Retrieve selected options index
 *
 * @return				Index of selected option
 */
int32 OptionsSelector::getSelectedOption()
{
	return this->currentOptionIndex;
}

/**
 * Print the list of options
 *
 * @param x	 X coordinate to start printing at (in chars)
 * @param y	 Y coordinate to start printing at (in chars)
 */
void OptionsSelector::printOptions(uint8 x, uint8 y)
{
	Printing printing = Printing::getInstance();

	if(this->currentPage && 0 < VirtualList::getSize(VirtualList::safeCast(VirtualNode::getData(this->currentPage))))
	{
		FontData* fontData = Printing::getFontByName(printing, this->font);

		this->x = (x < (__SCREEN_WIDTH_IN_CHARS)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT_IN_CHARS)) ? y : 0;

		ASSERT(this->currentPage, "printOptions: currentPage");
		VirtualNode node = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;

		int32 i = 0;
		for(; i < (this->rows * fontData->fontSpec->fontSize.y) && y + i < __SCREEN_HEIGHT_IN_CHARS; i++)
		{
			int32 j = 0;
			for(; (this->columnWidth * this->cols) > j && x + j < __SCREEN_WIDTH_IN_CHARS; j++)
			{
				Printing::text(printing, " ", x + j, y + i, this->font);
			}
		}

		int32 counter = 0;

		for(; node; node = node->next, counter++)
		{
			ASSERT(node, "printOptions: push null node");
			ASSERT(node->data, "printOptions: push null node data");

			Option* option = VirtualNode::getData(node);

			if(NULL == option->value)
			{
				Printing::int32(printing, counter, x + fontData->fontSpec->fontSize.x, y, this->font);
			}
			else
			{
				switch(option->type)
				{
					case kString:
						Printing::text(printing, (char*)option->value, x + fontData->fontSpec->fontSize.x, y, this->font);
						break;

					case kFloat:
						Printing::float(printing, *((float*)option->value), x + fontData->fontSpec->fontSize.x, y, 2, this->font);
						break;

					case kInt:
						Printing::int32(printing, *((int32*)option->value), x + fontData->fontSpec->fontSize.x, y, this->font);
						break;

					case kShortInt:
						Printing::int32(printing, *((int16*)option->value), x + fontData->fontSpec->fontSize.x, y, this->font);
						break;

					case kChar:
						Printing::int32(printing, *((int8*)option->value), x + fontData->fontSpec->fontSize.x, y, this->font);
						break;				}
			}

			y += fontData->fontSpec->fontSize.y;
			if((y >= (this->rows * fontData->fontSpec->fontSize.y + this->y)) || (y >= (__SCREEN_HEIGHT_IN_CHARS)))
			{
				y = this->y;
				x += this->columnWidth;
			}
		}

		OptionsSelector::printSelectorMark(this, this->mark);
	}
}

/**
 * Print the selector mark
 *
 * @private
 * @param mark	The character to use
 */
void OptionsSelector::printSelectorMark(char* mark)
{
	if(this->currentPage)
	{
		FontData* fontData = Printing::getFontByName(Printing::getInstance(), this->font);

		ASSERT(this->currentPage, "printSelectorMark: current page");
		ASSERT(VirtualNode::getData(this->currentPage), "printSelectorMark: null current data");

		int32 indexOption = this->currentOptionIndex - this->currentPageIndex * VirtualList::getSize(VirtualList::safeCast(VirtualList::front(this->pages)));
		int32 optionColumn = (int32)(indexOption / this->rows);
		int32 optionRow = indexOption - optionColumn * this->rows;
		optionColumn = this->columnWidth * optionColumn;

		Printing::text(
			Printing::getInstance(),
			mark,
			this->x + optionColumn,
			this->y + (optionRow * fontData->fontSpec->fontSize.y),
			this->font
		);
	}
}

/**
 * Execute the callback of the currently selected option
 */
void OptionsSelector::doCurrentSelectionCallback()
{
	Option* option = VirtualNode::getData(this->currentOption);

	if(option->callback && option->callbackScope)
	{
		option->callback(option->callbackScope);
	}
}

/**
 * Retrieve the total number of options
 *
 * @return		The number of options
 */
int32 OptionsSelector::getNumberOfOptions()
{
	return this->totalOptions;
}
