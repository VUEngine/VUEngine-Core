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
#include <VirtualList.h>
#include <Game.h>
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

	this->fps = 0;
	this->unevenFps = 0;
	this->gameFrameStarts = 0;
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
	this->fps = 0;
	this->unevenFps = 0;
	this->gameFrameStarts = 0;
	// Prevents reporting 51 FPS when swapping states
}

/**
 * Retrieve FPS
 */
uint16 FrameRate::getFps()
{
	return this->fps;
}

/**
 * Acknowledge that the game frame started
 *
 * @param gameFrameEnded	Boolean
 */
void FrameRate::gameFrameStarted(bool gameFrameEnded)
{
	this->gameFrameStarts++;

	if(!gameFrameEnded)
	{
		this->unevenFps++;
		FrameRate::fireEvent(this, kEventTornFrame);
	}

	if(__TARGET_FPS <= this->gameFrameStarts)
	{

#ifdef __PRINT_FRAMERATE
		if(!Game::isInSpecialMode(Game::getInstance()))
		{
			FrameRate::print(this, 21, 26);
		}

		this->fps = 0;
		this->unevenFps = 0;
		this->gameFrameStarts = 0;
#endif
	}
}

/**
 * Update
 */
void FrameRate::update()
{
	this->fps++;
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
	Printing::int32(printing, this->fps, col + 4, row, NULL);
	Printing::int32(printing, this->unevenFps, col + 7, row, NULL);
}
