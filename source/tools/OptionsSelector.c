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

#include <string.h>

#include <OptionsSelector.h>
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
#include <Utilities.h>

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
void OptionsSelector::constructor(uint16 cols, uint16 rows, char* font, char* leftMark, char* rightMark)
{
	Base::constructor();

	this->pages = NULL;
	this->currentPage = NULL;
	this->currentOption = NULL;
	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
	this->x = 0;
	this->y = 0;
	this->optionsLength = 0;
	this->alignment = kOptionsAlignLeft;
	this->spacing = 0;
	this->cols = ((0 < cols) && (cols <= __OPTIONS_SELECT_MAX_COLS)) ? cols : 1;
	this->rows = ((0 < rows) && (rows <= __OPTIONS_SELECT_MAX_ROWS)) ? rows : __OPTIONS_SELECT_MAX_ROWS;
	this->totalOptions = 0;
	this->leftMark = NULL == leftMark ? __CHAR_SELECTOR_LEFT : leftMark;
	this->rightMark = rightMark;
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

		for(; NULL != node; node = node->next)
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
void OptionsSelector::setMarkCharacters(char* leftMark, char* rightMark)
{
	this->leftMark = leftMark;
	this->rightMark = rightMark;
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
		OptionsSelector::printSelectorMark(this, " ", -this->optionsLength);
		OptionsSelector::printSelectorMark(this, " ", this->optionsLength);

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
				OptionsSelector::printOptions(this, this->x, this->y, this->alignment, this->spacing - 1);
			}
			else
			{
				// wrap around and select first option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;
				this->currentOptionIndex = 0;
			}
		}

		// print new selection mark
		OptionsSelector::printSelectorMark(this, this->leftMark, -this->optionsLength);
		OptionsSelector::printSelectorMark(this, this->rightMark, this->optionsLength);
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
		OptionsSelector::printSelectorMark(this, " ", -this->optionsLength);
		OptionsSelector::printSelectorMark(this, " ", this->optionsLength);

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
				OptionsSelector::printOptions(this, this->x, this->y, this->alignment, this->spacing - 1);
			}
			else
			{
				// wrap around and select last option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->tail;
				this->currentOptionIndex = this->totalOptions - 1;
			}
		}

		// print new selection mark
		OptionsSelector::printSelectorMark(this, this->leftMark, -this->optionsLength);
		OptionsSelector::printSelectorMark(this, this->rightMark, this->optionsLength);
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
void OptionsSelector::printOptions(uint8 x, uint8 y, uint32 alignment, uint8 spacing)
{
	Printing printing = Printing::getInstance();

	spacing++;

	if(this->currentPage && 0 < VirtualList::getSize(VirtualList::safeCast(VirtualNode::getData(this->currentPage))))
	{
		FontData* fontData = Printing::getFontByName(printing, this->font);

		this->x = (x < (__SCREEN_WIDTH_IN_CHARS)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT_IN_CHARS)) ? y : 0;
		this->alignment = alignment;
		this->spacing = spacing;

		ASSERT(this->currentPage, "printOptions: currentPage");
		VirtualNode node = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;

		int8 jStart = 0;

		switch(alignment)
		{
			case kOptionsAlignCenter:

				jStart -= this->columnWidth / 2 - 1;
				break;

			case kOptionsAlignRight:

				jStart -= this->columnWidth;
				break;
		}

		for(int32 i = 0; i < (this->rows * fontData->fontSpec->fontSize.y) && y + i < __SCREEN_HEIGHT_IN_CHARS; i++)
		{
			for(int32 j = 0; (this->columnWidth * this->cols) > j && x + j < __SCREEN_WIDTH_IN_CHARS; j++)
			{
				Printing::text(printing, " ", x + j + jStart, y + i * spacing, this->font);
			}
		}


		for(int32 counter = 0; NULL != node; node = node->next, counter++)
		{
			ASSERT(node, "printOptions: push null node");
			ASSERT(node->data, "printOptions: push null node data");

			int8 optionsLength = 0;
			int8 optionsLengthDivisor = 1;
			Option* option = VirtualNode::getData(node);

			switch(alignment)
			{
				case kOptionsAlignCenter:

					optionsLengthDivisor = 2;
					break;
			}

			if(NULL == option->value)
			{
				Printing::int32(printing, counter, x + fontData->fontSpec->fontSize.x, y, this->font);
			}
			else
			{
				switch(option->type)
				{
					case kString:

						optionsLength = strnlen((char*)option->value, this->columnWidth);
						Printing::text(printing, (char*)option->value, x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, y, this->font);
						break;

					case kFloat:

						optionsLength = Utilities::getDigitCount(*((int32*)option->value)) - 3;
						Printing::float(printing, *((float*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, y, 2, this->font);
						break;

					case kInt:

						optionsLength = Utilities::getDigitCount(*((int32*)option->value));
						Printing::int32(printing, *((int32*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, y, this->font);
						break;

					case kShortInt:

						optionsLength = Utilities::getDigitCount(*((int32*)option->value));
						Printing::int32(printing, *((int16*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, y, this->font);
						break;

					case kChar:

						optionsLength = Utilities::getDigitCount(*((int32*)option->value));
						Printing::int32(printing, *((int8*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, y, this->font);
						break;				
				}
			}

			if(__ABS(optionsLength) > __ABS(this->optionsLength))
			{
				this->optionsLength = optionsLength;
			}

			y += fontData->fontSpec->fontSize.y * spacing;
			
			if((y >= (this->rows * this->spacing * fontData->fontSpec->fontSize.y + this->y)) || (y >= (__SCREEN_HEIGHT_IN_CHARS)))
			{
				y = this->y;
				x += this->columnWidth;
			}
		}

		OptionsSelector::printSelectorMark(this, this->leftMark, -this->optionsLength);
		OptionsSelector::printSelectorMark(this, this->rightMark, this->optionsLength);
	}
}

/**
 * Print the selector mark
 *
 * @private
 * @param mark	The character to use
 */
void OptionsSelector::printSelectorMark(char* mark, int8 optionsLength)
{
	if(this->currentPage && NULL != mark)
	{
		FontData* fontData = Printing::getFontByName(Printing::getInstance(), this->font);

		ASSERT(this->currentPage, "printSelectorMark: current page");
		ASSERT(VirtualNode::getData(this->currentPage), "printSelectorMark: null current data");

		int32 indexOption = this->currentOptionIndex - this->currentPageIndex * VirtualList::getSize(VirtualList::safeCast(VirtualList::front(this->pages)));
		int32 optionColumn = (int32)(indexOption / this->rows);
		int32 optionRow = indexOption - optionColumn * this->rows;
		optionColumn = this->columnWidth * optionColumn;

		switch(this->alignment)
		{
			case kOptionsAlignCenter:

				if(0 < optionsLength)
				{
					int8 optionsLengthHelper = optionsLength;
					optionsLength = optionsLength / 2 + (0 == __MODULO(__ABS(optionsLengthHelper), 2) ? 1 : 2);
				}
				else
				{
					optionsLength /= 2;
				}

				break;
		}

		Printing::text(
			Printing::getInstance(),
			mark,
			this->x + optionColumn + optionsLength,
			this->y + (optionRow * this->spacing * fontData->fontSpec->fontSize.y),
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
