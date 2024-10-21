/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <DebugConfig.h>
#include <Printing.h>
#include <VUEngine.h>

#include "FrameRate.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void FrameRate::reset()
{
	this->FPS = 0;
	this->unevenFPS = 0;
	this->gameFrameStarts = 0;
	this->seconds = 0;
	this->totalFPS = 0;
	this->totalUnevenFPS = 0;
}
//---------------------------------------------------------------------------------------------------------
void FrameRate::setTarget(uint8 targetFPS)
{
	FrameRate::reset(this);
	this->targetFPS = targetFPS;
}
//---------------------------------------------------------------------------------------------------------
void FrameRate::update()
{
	this->FPS++;
}
//---------------------------------------------------------------------------------------------------------
void FrameRate::gameFrameStarted(bool gameCycleEnded)
{
	if(!gameCycleEnded)
	{
		this->unevenFPS++;
	}

	this->gameFrameStarts++;

	if(this->targetFPS <= this->gameFrameStarts)
	{
		this->seconds++;
		this->totalFPS += this->FPS;
		this->totalUnevenFPS += this->unevenFPS;

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

#ifdef __UNLOCK_FPS
#define __PRINT_FRAMERATE
#define __PRINT_FRAMERATE_AT_X		27
#define __PRINT_FRAMERATE_AT_Y		0
#endif

#ifdef __PRINT_FRAMERATE
#ifdef __PRINT_FRAMERATE_AT_X
#ifdef __PRINT_FRAMERATE_AT_Y
		if(!VUEngine::isInToolState(VUEngine::getInstance()))
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
//---------------------------------------------------------------------------------------------------------
void FrameRate::print(int32 x, int32 y)
{
#ifdef __UNLOCK_FPS
	Printing printing = Printing::getInstance();
	Printing::int32(printing, this->FPS, x, y, NULL);
	Printing::int32(printing, this->totalFPS / this->seconds, x + 7, y, NULL);
#else
	Printing printing = Printing::getInstance();
	Printing::text(printing, "FPS   /   ", x, y, NULL);
	Printing::int32(printing, this->FPS, x + 4, y, NULL);
	Printing::int32(printing, this->unevenFPS, x + 7, y, NULL);

	Printing::text(printing, "AVR   /   ", x + 10, y, NULL);
	Printing::int32(printing, this->totalFPS / this->seconds, x + 4 + 10, y, NULL);
	Printing::int32(printing, this->unevenFPS / this->seconds, x + 7 + 10, y, NULL);
#endif
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void FrameRate::destructor()
{
	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
