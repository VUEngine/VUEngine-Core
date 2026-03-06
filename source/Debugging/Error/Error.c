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

#include <DebugConfig.h>
#include <Hardware.h>
#include <Printer.h>
#include <Sprite.h>
#include <Timer.h>
#include <DisplayUnit.h>

#include "Error.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool _triggeringException = false;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Error::triggerException(char* message __attribute__((unused)), char* detail __attribute__((unused)))
{
#ifndef __SHIPPING
	static bool processingException = false;

	if(processingException)
	{
		return;
	}

	processingException = true;

	int32 lp = _vuengineLinkPointer;
	int32 sp = _vuengineStackPointer;

	int32 x = 0 <= __EXCEPTION_COLUMN && __EXCEPTION_COLUMN <= 24 ? __EXCEPTION_COLUMN : 0;
	int32 y = 0 <= __EXCEPTION_LINE && __EXCEPTION_LINE <= 28 ? __EXCEPTION_LINE : 0;

	// Disable timer
	Timer::disable();

	// Turn on the display
	DisplayUnit::allowInterrupts(false);
	DisplayUnit::startDisplaying();
	DisplayUnit::startDrawing();

	// Disable interrupts
	Hardware::disableInterrupts();

	// Make sure there are fonts to show the exception
	Printer::setDebugMode();

	//print error message to screen
	if(0 < y)
	{
		Printer::text("                                             ", x, y - 1, NULL);
	}

	Printer::text
	(
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08 EXCEPTION "
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" , x, y++, NULL
	);

	Printer::text("                                                " , x, y++, NULL);
	Printer::text(" LP:                                  " , x, y, NULL);
	Printer::hex(lp, x + 8, y, 8, NULL);
	Printer::text(" SP: 		                         " , x, ++y, NULL);
	Printer::hex(sp, x + 8, y, 8, NULL);

	if(message)
	{
		Printer::text("                                                " , x, ++y + 1, NULL);
		Printer::text(" Message:                                       " , x, ++y, NULL);

		int32 stringMaxLength = (__SCREEN_WIDTH_IN_CHARS) - 2;
		int32 rowsAvailable  = (__SCREEN_HEIGHT_IN_CHARS) - y;
		int32 stringLength = strnlen(message, stringMaxLength * rowsAvailable) + 1;
		int32 lines = stringLength / stringMaxLength + (stringLength % stringMaxLength ? 1 : 0);
		int32 line = 0;

		for(; line < lines; line++, message += stringMaxLength)
		{
			char messageLine[stringLength];
			strncpy(messageLine, message, stringLength);

			// TODO: fix me, termination character not working
			messageLine[stringLength - 1] = (char)0;
			Printer::text("                                                " , x, ++y, NULL);
			Printer::text(messageLine, x + 1, y, NULL);
		}

		if(detail)
		{
			Printer::text(detail, x + 1, ++y, NULL);
		}

		if(y < (__SCREEN_HEIGHT_IN_CHARS) - 1)
		{
			Printer::text("                                             ", x, y + 3, NULL);
		}
	}

#ifdef __SHOW_STACK_OVERFLOW_ALERT
	Hardware::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 10, 0, true);
#endif

	// Prevent VIP's interrupts
	Hardware::disableInterrupts();

	DisplayUnit::debug();

	// Trap the game here
	while(true);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
