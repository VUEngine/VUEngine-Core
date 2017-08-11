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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Utilities.h>
#include <HardwareManager.h>
#include <Game.h>

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Clock _gameClock = NULL;



//---------------------------------------------------------------------------------------------------------
//											DEFINITIONS
//---------------------------------------------------------------------------------------------------------

static const char numbers[17] = "0123456789ABCDEF";


void Utilities_setClock(Clock clock)
{
	_gameClock = clock;
}

int Utilities_intLength(int value)
{
	int length = 0;

	while(value > 0)
	{
		value /= 10;

		length++;
	}
	(!length) ? length++ : length;

	return length;
}

char* Utilities_itoa(u32 num, u32 base, u32 digits)
{
#define __CHAR_HOLDER_SIZE		11
	int i = 0;
	static char rev[__CHAR_HOLDER_SIZE] __attribute__((section(".bss")));
//	int flag = false;
//	static char sign='-';

/*	if((int)num < 0)
	{
		flag = true;
//		num*=(-1);
	}
*/

	for(; i < __CHAR_HOLDER_SIZE - 1; i++)
	{
		rev[__CHAR_HOLDER_SIZE - 2 - i] = numbers[num % base];
		num /= base;
	}

	i = 0;
	while(rev[i] == '0')
	{
		i++;
	}
	if(i >= (__CHAR_HOLDER_SIZE - 1 - (int)digits))
	{
		i = __CHAR_HOLDER_SIZE - 1 - (int)digits;
	}

	rev[__CHAR_HOLDER_SIZE - 1] = 0;
	return rev + i;
}

const char* Utilities_toUppercase(const char* string)
{
	int i = 0;
	char* result = NULL;
	while(string[i])
	{
		result[i] = (string[i] >= 'a' && string[i] <= 'z')
			? string[i] - 32
			: string[i];

		i++;
	}

	return result;
}

const char* Utilities_toLowercase(const char* string)
{
	int i = 0;
	char* result = NULL;
	while(string[i])
	{
		result[i] = (string[i] >= 'A' && string[i] <= 'Z')
			? string[i] + 32
			: string[i];

		i++;
	}

	return result;
}

/*
 * Taken from Shokwav's N64 demo
 */
long Utilities_randomSeed()
{
	ASSERT(_gameClock, "Utilities::randomSeed: null _gameClock");

	static u32 seed = 7; /* Seed value */

	seed |= Clock_getTime(_gameClock);

	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return (seed);
}

// These real versions are due to Isaku Wada, 2002/01/09 added
int Utilities_random(long seed, int randnums)
{
	return seed & randnums ? __ABS((int)(seed % randnums)) : 0;
}

/*
 * Returns a random number in the requested range from the random seed
 */

/*
 * Check if 2 numbers have an equal sign
 */
int Utilities_equalSign(int a, int b)
{
	return ((a & (1 << sizeof(int))) ==	(b & (1 << sizeof(int))));
}

int Utilities_getDigitCount(int value)
{
	int size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while(value);

	return (size) ? size : 1;
}

u32 Utilities_reverse(u32 x, int bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
    return x >> ((sizeof(u32) << 3) - bits);
}
