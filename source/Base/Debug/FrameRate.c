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

#include <DebugConfig.h>
#include <Printing.h>
#include <VUEngine.h>

#include "FrameRate.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __PRINT_FRAMERATE_AT_X
#define __PRINT_FRAMERATE_AT_X		20
#endif

#ifndef __PRINT_FRAMERATE_AT_Y
#define __PRINT_FRAMERATE_AT_Y		0
#endif


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::reset()
{
	this->FPS = 0;
	this->unevenFPS = 0;
	this->gameFrameStarts = 0;
	this->seconds = 0;
	this->totalFPS = 0;
	this->totalUnevenFPS = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::setTarget(uint8 targetFPS)
{
	FrameRate::reset(this);
	this->targetFPS = targetFPS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::update()
{
	this->FPS++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::gameFrameStarted(bool gameCycleEnded, bool printFPS)
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

		if(!isDeleted(this->events))
		{
			FrameRate::fireEvent(this, kEventFramerateReady);
		}

		if(this->targetFPS > this->FPS)
		{
			if(!isDeleted(this->events))
			{
				FrameRate::fireEvent(this, kEventFramerateDipped);
			}
		}

		if(printFPS)
		{
#ifdef __TOOLS
			if(!VUEngine::isInToolState(VUEngine::getInstance()))
#endif
			{
				FrameRate::print(this, __PRINT_FRAMERATE_AT_X, __PRINT_FRAMERATE_AT_Y);
			}
		}

		this->FPS = 0;
		this->unevenFPS = 0;
		this->gameFrameStarts = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::print(int32 x, int32 y)
{
	Printing printing = Printing::getInstance();
	Printing::text(printing, "FPS     |TORN  |AVR     ", x, y, NULL);
	Printing::int32(printing, this->FPS, x + 4, y, NULL);
	Printing::int32(printing, this->unevenFPS, x + 14, y, NULL);
	Printing::int32(printing, ((float)this->totalFPS / this->seconds) + 0.5f, x + 20, y, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->FPS = 0;
	this->unevenFPS = 0;
	this->gameFrameStarts = 0;
	this->targetFPS = __TARGET_FPS;
	this->seconds = 0;
	this->totalFPS = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::destructor()
{
	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

