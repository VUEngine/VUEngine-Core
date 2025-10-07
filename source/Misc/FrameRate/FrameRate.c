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
#include <Printer.h>
#include <Singleton.h>

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

#ifdef __DEBUG
	this->targetFPS >>= 1;
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::update()
{
	this->FPS++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

#ifdef __TOOLS
		if(!isDeleted(this->events) && !VUEngine::isInToolState())
#else
		if(!isDeleted(this->events))
#endif
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

		this->FPS = 0;
		this->unevenFPS = 0;
		this->gameFrameStarts = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameRate::print(int32 x, int32 y)
{
	Printer::text("FPS     |TORN  |AVR     ", x, y, NULL);
	Printer::int32(this->FPS, x + 4, y, NULL);
	Printer::int32(this->unevenFPS, x + 14, y, NULL);
	Printer::int32(((float)this->totalFPS / this->seconds) + 0.5f, x + 20, y, NULL);
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
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
