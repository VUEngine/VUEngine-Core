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
#define SET_BIT(var,bit) 		(var |= (0x01 << bit))
#define CLEAR_BIT(var,bit) 		(var &= (~(0x01 << bit)))
#define TOGGLE_BIT(var,bit)		 (var ^= (0x01 << bit))
#define GET_BIT(var,bit) 		(0x01 & (var >> bit))
#define __ITOA_ARRAY_SIZE		11

extern char _itoaArray[];
extern const char _itoaNumbers[];


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Utilities : Object
{
	/// @publicsection

	static inline char* itoa(uint32 num, uint32 base, int32 digits);
	static inline const char* toUppercase(const char* string);
	static inline const char* toLowercase(const char* string);
	static inline uint32 reverse(uint32 x, int32 bits);
}

static inline char* Utilities::itoa(uint32 num, uint32 base, int32 digits)
{
	int32 i = __ITOA_ARRAY_SIZE - 1;

	for(; 0 < i;)
	{
		i--;
		_itoaArray[i] = _itoaNumbers[num % base];
		num /= base;

		if(0 == num)
		{
			break;
		}
	}

	while(__ITOA_ARRAY_SIZE - 1 - digits < i)
	{
		_itoaArray[--i] = '0';
	}

	_itoaArray[__ITOA_ARRAY_SIZE - 1] = '\0';

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

static inline uint32 Utilities::reverse(uint32 x, int32 bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
    return x >> ((sizeof(uint32) << 3) - bits);
}


#endif
