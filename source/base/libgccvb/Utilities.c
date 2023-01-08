/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Utilities.h>
#include <HardwareManager.h>


Clock _gameClock = NULL;
KeypadManager _keypadManager = NULL;

//---------------------------------------------------------------------------------------------------------
//											DEFINITIONS
//---------------------------------------------------------------------------------------------------------

uint32 _seed = 7; /* Seed value */
static char numbers[17] = "0123456789ABCDEF";

static void Utilities::setClock(Clock clock)
{
	_gameClock = clock;
}

static void Utilities::setKeypadManager(KeypadManager keypadManager)
{
	_keypadManager = keypadManager;
}

static int32 Utilities::getDigitCount(int32 value)
{
	int32 size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while(value);

	return size;
}

static int32 Utilities::intLength(int32 value)
{
	int32 length = 0;

	while(value > 0)
	{
		value /= 10;

		length++;
	}

	(0 == length) ? length++ : length;

	return length;
}

static char* Utilities::itoa(uint32 num, uint32 base, uint32 digits)
{
#define __CHAR_HOLDER_SIZE		11
	int32 i = 0;
	static char rev[__CHAR_HOLDER_SIZE] __attribute__((section(".bss")));
//	int32 flag = false;
//	static char sign='-';

/*	if((int32)num < 0)
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
	if(i >= (__CHAR_HOLDER_SIZE - 1 - (int32)digits))
	{
		i = __CHAR_HOLDER_SIZE - 1 - (int32)digits;
	}

	rev[__CHAR_HOLDER_SIZE - 1] = 0;
	return rev + i;
}

static const char* Utilities::toUppercase(const char* string)
{
	int32 i = 0;
	char* result = NULL;
	while(string[i])
	{
		result[i] = (string[i] >= 'a' && string[i] <= 'z') ? string[i] - 32 : string[i];
		i++;
	}
	result[i] = 0;

	return result;
}

static const char* Utilities::toLowercase(const char* string)
{
	int32 i = 0;
	char* result = NULL;
	while(string[i])
	{
		result[i] = (string[i] >= 'A' && string[i] <= 'Z') ? string[i] + 32 : string[i];
		i++;
	}
	result[i] = 0;

	return result;
}

static void Utilities::resetRandomSeed()
{
	_seed = 7; /* Seed value */
}

/*
 * Returns a random number in the requested range from the random seed
 */

/*
 * Check if 2 numbers have an equal sign
 */
static int32 Utilities::equalSign(int32 a, int32 b)
{
	return ((a & (1 << sizeof(int32))) ==	(b & (1 << sizeof(int32))));
}

static uint32 Utilities::reverse(uint32 x, int32 bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
    return x >> ((sizeof(uint32) << 3) - bits);
}

static float Utilities::floor(float x) 
{
	return (float)((long)(x * 2 + 0.5f) >> 1);
}

static int32 Utilities::min(int32 x, int32 y)
{
	return x < y ? x : y;
}

static int32 Utilities::max(int32 x, int32 y)
{
	return x > y ? x : y;
}
