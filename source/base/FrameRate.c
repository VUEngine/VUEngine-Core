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

#include <FrameRate.h>

#include <Printing.h>
#include <VUEngine.h>

#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			FrameRate::getInstance()
 * @memberof	FrameRate
 * @public
 * @return		FrameRate instance
 */


/**
 * Class constructor
 *
 * @private
 */
void FrameRate::constructor()
{
	Base::constructor();

	this->FPS = 0;
	this->unevenFPS = 0;
	this->gameFrameStarts = 0;
	this->targetFPS = __TARGET_FPS;
	this->seconds = 0;
	this->totalFPS = 0;
}

/**
 * Class destructor
 *
 * @private
 */
void FrameRate::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Reset internal values
 */
void FrameRate::reset()
{
	this->FPS = 0;
	this->unevenFPS = 0;
	this->gameFrameStarts = 0;
	this->targetFPS = __TARGET_FPS;
	this->seconds = 0;
	this->totalFPS = 0;
}

/**
 * Retrieve FPS
 */
uint16 FrameRate::getFPS()
{
	return this->FPS;
}

void FrameRate::setTarget(uint8 targetFPS)
{
	FrameRate::reset(this);
	this->targetFPS = targetFPS;
}

/**
 * Acknowledge that the game frame started
 *
 * @param gameCycleEnded	Boolean
 */
void FrameRate::gameFrameStarted(bool gameCycleEnded)
{
	if(!gameCycleEnded)
	{
		this->unevenFPS++;
		this->totalUnevenFPS += this->unevenFPS;
	}

	this->gameFrameStarts++;

	if(this->targetFPS <= this->gameFrameStarts)
	{
		this->seconds++;
		this->totalFPS += this->FPS;

		if(this->targetFPS > this->FPS)
		{
#ifdef __PRINT_FRAMERATE_DIP
#ifdef __PRINT_FRAMERATE_AT_X
#ifdef __PRINT_FRAMERATE_AT_Y
			FrameRate::print(this, __PRINT_FRAMERATE_AT_X, __PRINT_FRAMERATE_AT_Y);
#endif
#endif
#endif
			if(!isDeleted(this->events))
			{
				FrameRate::fireEvent(this, kEventFrameRateDipped);
			}
		}

#ifdef __PRINT_FRAMERATE
#ifdef __PRINT_FRAMERATE_AT_X
#ifdef __PRINT_FRAMERATE_AT_Y
		if(!VUEngine::isInSpecialMode(VUEngine::getInstance()))
		{
			FrameRate::print(this, __PRINT_FRAMERATE_AT_X, __PRINT_FRAMERATE_AT_Y);
		}
#endif
#endif
#endif
		this->FPS = 0;
		this->unevenFPS = 0;
		this->gameFrameStarts = 0;
	}
}

/**
 * Update
 */
void FrameRate::update()
{
	this->FPS++;
}

/**
 * Print FPS
 *
 * @param col	Column to start printing at
 * @param row	Row to start printing at
 */
void FrameRate::print(int32 col, int32 row)
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "FPS   /   ", col, row, NULL);
	Printing::int32(printing, this->FPS, col + 4, row, NULL);
	Printing::int32(printing, this->unevenFPS, col + 7, row, NULL);

	Printing::text(printing, "AVR   /   ", col + 10, row, NULL);
	Printing::int32(printing, this->totalFPS / this->seconds, col + 4l + 10, row, NULL);
	Printing::int32(printing, this->unevenFPS / this->seconds, col + 7l + 10, row, NULL);
}
