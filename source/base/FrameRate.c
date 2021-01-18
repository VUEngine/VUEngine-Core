/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
u16 FrameRate::getFps()
{
	return this->fps;
}

/**
 * Update
 */
void FrameRate::update()
{
	this->fps++;

	float elapsedTime = Stopwatch::lap(this->stopwatch);
	this->gameFrameTotalTime += elapsedTime;

	if(__GAME_FRAME_DURATION < elapsedTime)
	{
		this->unevenFps++;
	}

	if(__MILLISECONDS_PER_SECOND - __GAME_FRAME_DURATION < this->gameFrameTotalTime)
	{
		if(__MILLISECONDS_PER_SECOND <= (int)this->gameFrameTotalTime) 
		{
			this->fps--;
		}

#ifdef __PRINT_FRAMERATE
		if(!Game::isInSpecialMode(Game::getInstance()))
		{
			FrameRate::print(this, 21, 26);
		}
#endif

		this->fps = 0;
		this->unevenFps = 0;
		this->gameFrameTotalTime = 0;
	}
}

/**
 * Print FPS
 *
 * @param col	Column to start printing at
 * @param row	Row to start printing at
 */
void FrameRate::print(int col, int row)
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "FPS   /   ", col, row, NULL);
	Printing::int(printing, this->fps, col + 4, row, NULL);
	Printing::int(printing, this->unevenFps, col + 7, row, NULL);
}
