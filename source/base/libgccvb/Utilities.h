/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//											 PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// macros for bitmask operations
#define SET_BIT(var,bit) 	(var |= (0x01 << bit))
#define CLEAR_BIT(var,bit) 	(var &= (~(0x01 << bit)))
#define TOGGLE_BIT(var,bit) (var ^= (0x01 << bit))
#define GET_BIT(var,bit) 	(0x01 & (var >> bit))


//---------------------------------------------------------------------------------------------------------
//											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

void Utilities_setClock(Clock clock);
long Utilities_randomSeed();
int Utilities_random(long seed, int randnums);
char* Utilities_itoa(u32 num, u32 base, u32 digits);
const char* Utilities_toUppercase(const char* string);
const char* Utilities_toLowercase(const char* string);
int Utilities_equalSign(int a, int b);
int Utilities_getDigitCount(int value);
int Utilities_intLength(int value);
u32 Utilities_reverse(u32 x, int bits);


#endif
