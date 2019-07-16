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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <debugConfig.h>
#include <Profiler.h>
#include <Game.h>
#include <GameState.h>
#include <UIContainer.h>
#include <SpriteManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Profiler::getInstance()
 * @memberof	Profiler
 * @public
 * @return		Profiler instance
 */


/**
 * Class constructor
 *
 * @private
 */
void Profiler::constructor()
{
	Base::constructor();

	this->initialized = false;
	this->profilingSprite = NULL;
	Profiler::reset(this);
}

/**
 * Class destructor
 */
void Profiler::destructor()
{
	// allow a new construct
	Base::destructor();
}

void Profiler::reset()
{
	this->paletteValues[0] = 0b01000000;
	this->paletteValues[1] = 0b01010000;
	this->paletteValues[2] = 0b01010100;
	this->paletteValues[3] = 0b10010100;
	this->paletteValues[4] = 0b10100100;
	this->paletteValues[5] = 0b10101000;
	this->paletteValues[6] = 0b11101000;
	this->paletteValues[7] = 0b11111000;
	this->paletteValues[8] = 0b11111100;

	this->currentPeletteIndex = 0;

	if(this->initialized)
	{
		_vipRegisters[__GPLT0 + __PROFILING_PALETTE] = 0b00000000;
	}
}

/**
 * Initialize manager
 */
void Profiler::initialize()
{
	/*
	Printing::resetCoordinates(Printing::getInstance());

	int i = 0;
	int j = 1;

	for(; i < __SCREEN_HEIGHT_IN_CHARS; i++)
	{
		PRINT_TEXT(__CHAR_PROFILING, j, i);
		PRINT_TEXT(__CHAR_PROFILING, j, i);
	}
	*/

	Stage stage = GameState::getStage(Game::getCurrentState(Game::getInstance()));

	if(!isDeleted(stage))
	{
		UIContainer uiContainer = Stage::getUIContainer(stage);

		if(!isDeleted(uiContainer))
		{
			extern EntitySpec PROFILING_IM;

			PositionedEntity entities[] = 
			{
				{&PROFILING_IM, {__SCREEN_WIDTH / 2 -64, 112, -128, 0}, 0, "Profile", NULL, NULL, true}, 
				{NULL, {0,0,0,0}, 0, NULL, NULL, NULL, false}
			};

			UIContainer::addEntities(uiContainer, entities);
			Entity profilingEntity = !isDeleted(uiContainer) ? Entity::safeCast(UIContainer::getChildByName(uiContainer, "Profile", true)) : NULL;
			this->profilingSprite = !isDeleted(uiContainer) ? VirtualList::front(Entity::getSprites(profilingEntity)) : NULL;

			NM_ASSERT(!isDeleted(this->profilingSprite), "ERROR");
			SpriteManager::showLayer(SpriteManager::getInstance(), Sprite::getWorldLayer(this->profilingSprite));

			this->initialized = true;
		}
	}
}

void Profiler::start()
{
/*	static s16 previousBlock = 0;
	s16 currentBlock = VIPManager::getCurrentBlockBeingDrawn(VIPManager::getInstance());

//	if(__PROFILER_TOTAL_AVAILABLE_PALETTES - 1 < ++this->currentPeletteIndex)
	{
		PRINT_TEXT("              ", 1, 1 + this->currentPeletteIndex);
		PRINT_INT(currentBlock, 1, 1 + this->currentPeletteIndex);
	}

	if(this->currentPeletteIndex)
	PRINT_INT(currentBlock - previousBlock, 7, 1 + this->currentPeletteIndex);


	if(__PROFILER_TOTAL_AVAILABLE_PALETTES <= this->currentPeletteIndex)
	{
		//this->currentPeletteIndex = 0;
		previousBlock = 0;
	}

	previousBlock = currentBlock;
*/

	if(!this->initialized || __PROFILER_TOTAL_AVAILABLE_PALETTES <= this->currentPeletteIndex)
	{
		return;
	}

	_vipRegisters[__GPLT0 + __PROFILING_PALETTE] = this->paletteValues[this->currentPeletteIndex++];
}

void Profiler::end()
{	
	Profiler::reset(this);
}
