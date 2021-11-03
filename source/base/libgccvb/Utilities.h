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
	static uint32 randomSeed();
	static int32 random(uint32 seed, int32 randnums);
	static char* itoa(uint32 num, uint32 base, uint32 digits);
	static const char* toUppercase(const char* string);
	static const char* toLowercase(const char* string);
	static int32 equalSign(int32 a, int32 b);
	static int32 getDigitCount(int32 value);
	static int32 intLength(int32 value);
	static uint32 reverse(uint32 x, int32 bits);
	static float Utilities::floor(float x);
}


#endif
