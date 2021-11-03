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
	this->gameFrameTotalTime = 0;
	this->stopwatch = new Stopwatch();
}

/**
 * Class destructor
 *
 * @private
 */
void FrameRate::destructor()
{
	delete this->stopwatch;

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
	// Prevents reporting 51 FPS when swapping states
	this->gameFrameTotalTime = __MILLISECONDS_PER_SECOND / 2;
	Stopwatch::reset(this->stopwatch);
}

/**
 * Retrieve FPS
 */
uint16 FrameRate::getFps()
{
	return this->fps;
}

/**
 * Update
 */
#ifdef __PRINT_FRAMERATE
void FrameRate::update()
{
	float elapsedTime = Stopwatch::lap(this->stopwatch);
	this->gameFrameTotalTime += elapsedTime;

#ifdef __PRINT_FRAME_TIMES
	static uint32 elapsedTimes[60] = {1};
	elapsedTimes[this->fps] = elapsedTime;
#endif

	if(__GAME_FRAME_DURATION + 0.5f < elapsedTime)
	{
		this->unevenFps++;
	}

	this->fps++;

	if(__MILLISECONDS_PER_SECOND - __GAME_FRAME_DURATION < this->gameFrameTotalTime)
	{
#ifdef __PRINT_FRAME_TIMES
		int32 x = 1; 
		int32 y = 1;

		for(int32 fps = 0; fps < 51; fps++)
		{
			PRINT_TEXT("       ", x, y + fps);
			PRINT_INT(elapsedTimes[fps] * 1000, x, y + fps);
			elapsedTimes[fps] = 0;

			if(0 == ((y + fps) % 20))
			{
				x += 8;
				y -= 20;
			}
		}
#endif

		if(__MILLISECONDS_PER_SECOND <= (int32)this->gameFrameTotalTime) 
		{
			this->fps--;
		}

		if(!Game::isInSpecialMode(Game::getInstance()))
		{
			FrameRate::print(this, 21, 26);
		}

		this->fps = 0;
		this->unevenFps = 0;
		this->gameFrameTotalTime = 0;
	}
}

#else

void FrameRate::update()
{
	this->gameFrameTotalTime += (__MICROSECONDS_PER_MILLISECOND / __TARGET_FPS);
	this->fps++;

	if(__MILLISECONDS_PER_SECOND - __GAME_FRAME_DURATION < this->gameFrameTotalTime)
	{
		if(__MILLISECONDS_PER_SECOND <= (int32)this->gameFrameTotalTime) 
		{
			this->fps--;
		}

		this->fps = 0;
		this->unevenFps = 0;
		this->gameFrameTotalTime = 0;
	}
}

#endif

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
