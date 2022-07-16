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
static class Utilities : ListenerObject
{
	/// @publicsection
	static void setClock(Clock clock);
	static void setKeypadManager(KeypadManager keypadManager);
	static void resetRandomSeed();
	static inline uint32 randomSeed();
	static inline int32 random(uint32 seed, int32 randnums);
	static char* itoa(uint32 num, uint32 base, uint32 digits);
	static const char* toUppercase(const char* string);
	static const char* toLowercase(const char* string);
	static int32 equalSign(int32 a, int32 b);
	static int32 getDigitCount(int32 value);
	static int32 intLength(int32 value);
	static uint32 reverse(uint32 x, int32 bits);
	static float floor(float x);
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

#endif
