/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


Clock _gameClock = NULL;
KeypadManager _keypadManager = NULL;

//---------------------------------------------------------------------------------------------------------
//											DEFINITIONS
//---------------------------------------------------------------------------------------------------------

static u32 _seed = 7; /* Seed value */
static char numbers[17] = "0123456789ABCDEF";

static void Utilities::setClock(Clock clock)
{
	_gameClock = clock;
}

static void Utilities::setKeypadManager(KeypadManager keypadManager)
{
	_keypadManager = keypadManager;
}


static int Utilities::intLength(int value)
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

static char* Utilities::itoa(u32 num, u32 base, u32 digits)
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

static const char* Utilities::toUppercase(const char* string)
{
	int i = 0;
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
	int i = 0;
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
 * Taken from Shokwav's N64 demo
 */
static u32 Utilities::randomSeed()
{
	if(!_seed)
	{
		_seed = 7;
	}

	_seed ^= _seed << 13;
	_seed ^= _seed >> 17;
	_seed ^= _seed << 5;

	return _seed;
}

// These real versions are due to Isaku Wada, 2002/01/09 added
static int Utilities::random(u32 seed, int randnums)
{
#ifdef __ADD_USER_INPUT_AND_TIME_TO_RANDOM_SEED
	seed += Clock::getTime(_gameClock) + KeypadManager::getAccumulatedUserInput(_keypadManager);
#endif

	return seed && randnums ? __ABS((int)(seed % randnums)) : 0;
}

/*
 * Returns a random number in the requested range from the random seed
 */

/*
 * Check if 2 numbers have an equal sign
 */
static int Utilities::equalSign(int a, int b)
{
	return ((a & (1 << sizeof(int))) ==	(b & (1 << sizeof(int))));
}

static int Utilities::getDigitCount(int value)
{
	int size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while(value);

	return size;
}

static u32 Utilities::reverse(u32 x, int bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
    return x >> ((sizeof(u32) << 3) - bits);
}

static float Utilities::floor(float x) 
{
    float xAux = x < 0 ? x *- 1 : x;
    unsigned int zeros = 0;
    
	float n = 1;
    
	for(; xAux > n * 10; n *= 10, zeros++);

    for(xAux -=n; -1 != (int)zeros; xAux -= n)
	{
        if(xAux < 0)
        {
            xAux += n;
            n /= 10;
            --zeros;
        }
	}

    xAux += n;

    return x < 0 ? (xAux == 0? x : x - ( 1 - xAux) ) : (x - xAux);
}
