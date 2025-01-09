/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <Actor.h>
#include <AnimationCoordinatorFactory.h>
#include <Actor.h>
#include <Ball.h>
#include <BgmapTextureManager.h>
#include <Body.h>
#include <Box.h>
#include <Camera.h>
#include <CameraEffectManager.h>
#include <CameraMovementManager.h>
#include <CharSet.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <Container.h>
#include <DebugState.h>
#include <DirectDraw.h>
#include <Actor.h>
#include <ColliderManager.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <InverseBox.h>
#include <LineField.h>
#include <MBgmapSprite.h>
#include <Mem.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <Particle.h>
#include <ParticleSystem.h>
#include <BodyManager.h>
#include <Printing.h>
#include <Collider.h>
#include <SoundManager.h>
#include <Sphere.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <SRAMManager.h>
#include <Stage.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <State.h>
#include <StateMachine.h>
#include <Sound.h>
#include <Telegram.h>
#include <Texture.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>
#include <Wireframe.h>

#include "OptionsSelector.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;
friend class Printing;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::constructor(uint16 cols, uint16 rows, char* font, char* leftMark, char* rightMark)
{
	// Always explicitly call the base's constructor 
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::destructor()
{
	OptionsSelector::flushPages(this);

	// Allow a new construct

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::setColumnWidth(uint8 width)
{
	FontData* fontData = Printing::getFontByName(this->font);

	// Add space for selection mark, consider font width
	width = ((width + 1) * fontData->fontSpec->fontSize.x);

	if((0 < width) && (width <= (__SCREEN_WIDTH_IN_CHARS)))
	{
		this->columnWidth = width;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::setMarkCharacters(char* leftMark, char* rightMark)
{
	this->leftMark = leftMark;
	this->rightMark = rightMark;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::setOptions(VirtualList options)
{
	ASSERT(options, "OptionsSelector::setOptions: null options");

	OptionsSelector::flushPages(this);

	this->pages = new VirtualList();

	this->totalOptions = VirtualList::getCount(options);

	int32 optionsPerPage = this->cols * this->rows;
	int32 numberOfPages = (int32)(this->totalOptions / optionsPerPage);
	numberOfPages += (0 < (this->totalOptions % optionsPerPage)) ? 1 : 0;

	VirtualNode node = options->head;

	ASSERT(VirtualList::getCount(options), "OptionsSelector::setOptions: empty options");

	if(0 < VirtualList::getCount(options))
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
		ASSERT(VirtualList::getCount(this->pages), "OptionsSelector::setOptions: empty pages");

		this->currentOption = this->currentPage ? (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head : NULL;
	}

	this->currentPageIndex = 0;
	this->currentOptionIndex = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool OptionsSelector::setSelectedOption(int32 optionIndex)
{
	bool changed = false;

	// Check if desired option index is within bounds
	if(optionIndex >= 0 && optionIndex <= this->totalOptions)
	{
		if(optionIndex < this->currentOptionIndex)
		{
			// If desired option index is smaller than the current one, select previous until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector::selectNext(this);
				changed = true;
			}
		}
		else if(optionIndex > this->currentOptionIndex)
		{
			// If desired option index is larger than the current one, select next until desired option is set
			while(this->currentOptionIndex != optionIndex)
			{
				OptionsSelector::selectPrevious(this);
				changed = true;
			}
		}
	}

	return changed;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::selectNext()
{
	if(this->currentOption)
	{
		// Remove previous selection mark
		OptionsSelector::printSelectorMark(this, " ", -this->optionsLength);
		OptionsSelector::printSelectorMark(this, " ", this->optionsLength);

		// Get next option
		this->currentOption = this->currentOption->next;
		this->currentOptionIndex++;

		// If there's no next option on the current page
		if(!this->currentOption)
		{
			// If there's more than 1 page, go to next page
			if(VirtualList::getCount(this->pages) > 1)
			{
				// Select next page
				this->currentPage = this->currentPage->next;
				this->currentPageIndex++;

				// If next page does not exist
				if(!this->currentPage)
				{
					this->currentPage = this->pages->head;
					this->currentPageIndex = 0;
					this->currentOptionIndex = 0;
				}

				// Get new option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;
				ASSERT(this->currentOption, "selectNext: null current option");

				// Render new page
				OptionsSelector::print(this, this->x, this->y, this->alignment, this->spacing - 1);
			}
			else
			{
				// Wrap around and select first option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->head;
				this->currentOptionIndex = 0;
			}
		}

		// Print new selection mark
		OptionsSelector::printSelectorMark(this, this->leftMark, -this->optionsLength);
		OptionsSelector::printSelectorMark(this, this->rightMark, this->optionsLength);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::selectPrevious()
{
	if(this->currentOption)
	{
		// Remove previous selection mark
		OptionsSelector::printSelectorMark(this, " ", -this->optionsLength);
		OptionsSelector::printSelectorMark(this, " ", this->optionsLength);

		// Get previous option
		this->currentOption = VirtualNode::getPrevious(this->currentOption);
		this->currentOptionIndex--;

		// If there's no previous option on the current page
		if(!this->currentOption)
		{
			// If there's more than 1 page
			if(VirtualList::getCount(this->pages) > 1)
			{
				// Select previous page
				this->currentPage = VirtualNode::getPrevious(this->currentPage);
				this->currentPageIndex--;

				// If previous page does not exist, go to last page
				if(!this->currentPage)
				{
					this->currentPage = this->pages->tail;
					this->currentPageIndex = VirtualList::getCount(this->pages) - 1;
					this->currentOptionIndex = this->totalOptions - 1;
				}

				// Get new option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->tail;
				ASSERT(this->currentOption, "selectPrevious: current option data");

				// Render new page
				OptionsSelector::print(this, this->x, this->y, this->alignment, this->spacing - 1);
			}
			else
			{
				// Wrap around and select last option
				this->currentOption = (VirtualList::safeCast(VirtualNode::getData(this->currentPage)))->tail;
				this->currentOptionIndex = this->totalOptions - 1;
			}
		}

		// Print new selection mark
		OptionsSelector::printSelectorMark(this, this->leftMark, -this->optionsLength);
		OptionsSelector::printSelectorMark(this, this->rightMark, this->optionsLength);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 OptionsSelector::getSelectedOption()
{
	return this->currentOptionIndex;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 OptionsSelector::getNumberOfOptions()
{
	return this->totalOptions;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::print(uint8 x, uint8 y, uint32 alignment, uint8 spacing)
{
	spacing++;

	if(this->currentPage && 0 < VirtualList::getCount(VirtualList::safeCast(VirtualNode::getData(this->currentPage))))
	{
		FontData* fontData = Printing::getFontByName(this->font);

		this->x = (x < (__SCREEN_WIDTH_IN_CHARS)) ? x : 0;
		this->y = (y < (__SCREEN_HEIGHT_IN_CHARS)) ? y : 0;
		this->alignment = alignment;
		this->spacing = spacing;

		ASSERT(this->currentPage, "print: currentPage");
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
				Printing::text(" ", x + j + jStart, y + i * spacing, this->font);
			}
		}

		for(int32 counter = 0; NULL != node; node = node->next, counter++)
		{
			ASSERT(node, "print: push null node");
			ASSERT(node->data, "print: push null node data");

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
				Printing::int32(counter, x + fontData->fontSpec->fontSize.x, y, this->font);
			}
			else
			{
				switch(option->type)
				{
					case kString:

						optionsLength = alignment == kOptionsAlignLeft ? 0 : strnlen((char*)option->value, this->columnWidth);
						Printing::text
						(
							(char*)option->value, x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor,
							y, this->font
						);
						break;

					case kFloat:

						optionsLength = alignment == kOptionsAlignLeft ? 0 : Math::getDigitsCount(*((int32*)option->value)) - 3;
						Printing::float
						(
							*((float*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, 
							y, 2, this->font
						);
						break;

					case kInt:

						optionsLength = alignment == kOptionsAlignLeft ? 0 : Math::getDigitsCount(*((int32*)option->value));
						Printing::int32
						(
							*((int32*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, 
							y, this->font
						);
						break;

					case kShortInt:

						optionsLength = alignment == kOptionsAlignLeft ? 0 : Math::getDigitsCount(*((int32*)option->value));
						Printing::int32
						(
							*((int16*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, 
							y, this->font
						);
						break;

					case kChar:

						optionsLength = alignment == kOptionsAlignLeft ? 0 : Math::getDigitsCount(*((int32*)option->value));
						Printing::int32
						(
							*((int8*)option->value), x + fontData->fontSpec->fontSize.x - optionsLength / optionsLengthDivisor, 
							y, this->font
						);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::printSelectorMark(char* mark, int8 optionsLength)
{
	if(this->currentPage && NULL != mark)
	{
		FontData* fontData = Printing::getFontByName(this->font);

		ASSERT(this->currentPage, "printSelectorMark: current page");
		ASSERT(VirtualNode::getData(this->currentPage), "printSelectorMark: null current data");

		int32 indexOption = 
			this->currentOptionIndex - 
			this->currentPageIndex * VirtualList::getCount(VirtualList::safeCast(VirtualList::front(this->pages)));
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

		Printing::text
		(
			mark,
			this->x + optionColumn + optionsLength,
			this->y + (optionRow * this->spacing * fontData->fontSpec->fontSize.y),
			this->font
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void OptionsSelector::doCurrentSelectionCallback()
{
	Option* option = VirtualNode::getData(this->currentOption);

	if(option->callback && option->scope)
	{
		option->callback(option->scope);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
