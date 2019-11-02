/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef UTILITIES_H_
#define UTILITIES_H_


//---------------------------------------------------------------------------------------------------------
//											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Clock.h>
#include <KeypadManager.h>


//---------------------------------------------------------------------------------------------------------
//											 PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// macros for bitmask operations
#define SET_BIT(var,bit) 	(var |= (0x01 << bit))
#define CLEAR_BIT(var,bit) 	(var &= (~(0x01 << bit)))
#define TOGGLE_BIT(var,bit) (var ^= (0x01 << bit))
#define GET_BIT(var,bit) 	(0x01 & (var >> bit))


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Utilities : Object
{
	/// @publicsection
	static void setClock(Clock clock);
	static void setKeypadManager(KeypadManager keypadManager);
	static void resetRandomSeed();
	static u32 randomSeed();
	static int random(u32 seed, int randnums);
	static char* itoa(u32 num, u32 base, u32 digits);
	static const char* toUppercase(const char* string);
	static const char* toLowercase(const char* string);
	static int equalSign(int a, int b);
	static int getDigitCount(int value);
	static int intLength(int value);
	static u32 reverse(u32 x, int bits);
}


#endif
