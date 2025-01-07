/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with frameRate source code.
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

static void FrameRate::registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	FrameRate frameRate = FrameRate::getInstance();

	FrameRate::addEventListener(frameRate, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	FrameRate frameRate = FrameRate::getInstance();

	FrameRate::removeEventListener(frameRate, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::reset()
{
	FrameRate frameRate = FrameRate::getInstance();

	frameRate->FPS = 0;
	frameRate->unevenFPS = 0;
	frameRate->gameFrameStarts = 0;
	frameRate->seconds = 0;
	frameRate->totalFPS = 0;
	frameRate->totalUnevenFPS = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::setTarget(uint8 targetFPS)
{
	FrameRate frameRate = FrameRate::getInstance();

	FrameRate::reset();
	frameRate->targetFPS = targetFPS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::update()
{
	FrameRate frameRate = FrameRate::getInstance();

	frameRate->FPS++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::gameFrameStarted(bool gameCycleEnded, bool printFPS)
{
	FrameRate frameRate = FrameRate::getInstance();

	if(!gameCycleEnded)
	{
		frameRate->unevenFPS++;
	}

	frameRate->gameFrameStarts++;

	if(frameRate->targetFPS <= frameRate->gameFrameStarts)
	{
		frameRate->seconds++;
		frameRate->totalFPS += frameRate->FPS;
		frameRate->totalUnevenFPS += frameRate->unevenFPS;

		if(!isDeleted(frameRate->events))
		{
			FrameRate::fireEvent(frameRate, kEventFramerateReady);
		}

		if(frameRate->targetFPS > frameRate->FPS)
		{
			if(!isDeleted(frameRate->events))
			{
				FrameRate::fireEvent(frameRate, kEventFramerateDipped);
			}
		}

		if(printFPS)
		{
#ifdef __TOOLS
			if(!VUEngine::isInToolState())
#endif
			{
				FrameRate::print(__PRINT_FRAMERATE_AT_X, __PRINT_FRAMERATE_AT_Y);
			}
		}

		frameRate->FPS = 0;
		frameRate->unevenFPS = 0;
		frameRate->gameFrameStarts = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void FrameRate::print(int32 x, int32 y)
{
	FrameRate frameRate = FrameRate::getInstance();

	Printing::text("FPS     |TORN  |AVR     ", x, y, NULL);
	Printing::int32(frameRate->FPS, x + 4, y, NULL);
	Printing::int32(frameRate->unevenFPS, x + 14, y, NULL);
	Printing::int32(((float)frameRate->totalFPS / frameRate->seconds) + 0.5f, x + 20, y, NULL);
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
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
