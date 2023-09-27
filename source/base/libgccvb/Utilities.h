/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_


//---------------------------------------------------------------------------------------------------------
//											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
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

extern uint32 _seed;

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
	static inline uint32 randomSeed();
	static inline int32 random(uint32 seed, int32 randnums);
	static inline char* itoa(uint32 num, uint32 base, int32 digits);
	static inline const char* toUppercase(const char* string);
	static inline const char* toLowercase(const char* string);
	static inline int32 equalSign(int32 a, int32 b);
	static inline int32 getDigitsCount(int32 value);
	static inline uint32 reverse(uint32 x, int32 bits);
	static inline float floor(float x);
    static inline int32 min(int32 x, int32 y);
    static inline int32 max(int32 x, int32 y);
}

// These real versions are due to Isaku Wada, 2002/01/09 added
static inline int32 Utilities::random(uint32 seed, int32 randnums)
{
#ifdef __ADD_USER_INPUT_AND_TIME_TO_RANDOM_SEED
	extern Clock _gameClock;
	extern KeypadManager _keypadManager;

	if(NULL != _gameClock && NULL != _keypadManager)
	{
		seed += Clock::getTime(_gameClock) + KeypadManager::getAccumulatedUserInput(_keypadManager);
	}
#endif

	return 0 != randnums ? __ABS((int32)(seed % randnums)) : 0;
}

/*
 * Taken from https://www.youtube.com/watch?v=RzEjqJHW-NU
 */
static inline uint32 Utilities::randomSeed()
{
	_seed >>= 1;
	_seed |= ((0x00000001 & (_seed ^ (_seed >> 1))) << ((sizeof(_seed) << 3) - 1));

	return _seed;
}

/*
 * Taken from Shokwav's N64 demo
 */
/*
static inline uint32 Utilities::randomSeed()
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
*/

static inline int32 Utilities::getDigitsCount(int32 value)
{
	int32 size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while(0 != value);

	return size;
}

static inline char* Utilities::itoa(uint32 num, uint32 base, int32 digits)
{
#define __CHAR_HOLDER_SIZE		11
	
	extern char _itoaArray[];
	extern char _itoaNumbers[];

	int32 i = 0;

	for(; i < __CHAR_HOLDER_SIZE - 1; i++)
	{
		_itoaArray[__CHAR_HOLDER_SIZE - 2 - i] = _itoaNumbers[num % base];
		num /= base;
	}

	i = 0;
	while(_itoaArray[i] == '0')
	{
		i++;
	}

	if(i >= (__CHAR_HOLDER_SIZE - 1 - (int32)digits))
	{
		i = __CHAR_HOLDER_SIZE - 1 - (int32)digits;
	}

	_itoaArray[__CHAR_HOLDER_SIZE - 1] = 0;
	return _itoaArray + i;
}

static inline const char* Utilities::toUppercase(const char* string)
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

static inline const char* Utilities::toLowercase(const char* string)
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

static inline int32 Utilities::haveEqualSign(int32 a, int32 b)
{
	return ((a & (1 << sizeof(int32))) ==	(b & (1 << sizeof(int32))));
}

static inline uint32 Utilities::reverse(uint32 x, int32 bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
    return x >> ((sizeof(uint32) << 3) - bits);
}

static inline float Utilities::floor(float x) 
{
	return (float)((long)(x * 2 + 0.5f) >> 1);
}

static inline int32 Utilities::min(int32 x, int32 y)
{
	return x < y ? x : y;
}

static inline int32 Utilities::max(int32 x, int32 y)
{
	return x > y ? x : y;
}


#endif
