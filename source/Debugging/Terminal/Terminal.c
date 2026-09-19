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

#include "Terminal.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __TERMINAL_OUTPUT_ADDRESS		(unsigned char*) 0x02000030;
#define __TERMINAL_BUFFER_SIZE			256 

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Terminal::print(const char* text)
{
	if(NULL == text)
	{
		return;
	}
	
#ifndef __SHIPPING
	unsigned char* const terminalOut = __TERMINAL_OUTPUT_ADDRESS;

    while (0 != *text)
	{
        *terminalOut = *text;
        text++;
    }
	
    *terminalOut = '\n';
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Terminal::info(const char* text)
{
	char buffer[__TERMINAL_BUFFER_SIZE];

	Terminal::print(Terminal::addPrefix(buffer, __TERMINAL_BUFFER_SIZE, "INFO: ", text));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Terminal::warning(const char* text)
{
	char buffer[__TERMINAL_BUFFER_SIZE];

	Terminal::print(Terminal::addPrefix(buffer, __TERMINAL_BUFFER_SIZE, "WARNING: ", text));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Terminal::error(const char* text)
{
	char buffer[__TERMINAL_BUFFER_SIZE];

	Terminal::print(Terminal::addPrefix(buffer, __TERMINAL_BUFFER_SIZE, "ERROR: ", text));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static char* Terminal::addPrefix(char* buffer, int16 bufferSize, const char* prefix, const char* text)
{
    if(NULL == prefix || NULL == text || 0 >= bufferSize)
    {
		return NULL;
    }

	char* bufferHelper = buffer;
	
    while(0 != *prefix && 1 < bufferSize--)
	{
		*bufferHelper++ = *prefix++;
	}
	
    while(0 != *text && 1 < bufferSize--)
	{
		*bufferHelper++ = *text++;		
	}
		
	*bufferHelper = '\0';
	
    return buffer;
}
