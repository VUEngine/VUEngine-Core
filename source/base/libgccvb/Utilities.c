/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Utilities.h>
#include <HardwareManager.h>
#include <Clock.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
// 											DEFINITIONS
//---------------------------------------------------------------------------------------------------------

static const char numbers[17] = "0123456789ABCDEF";

int Utilities_intLength(int value)
{
	int length = 0;

	while (value > 0)
	{
		value /= 10;

		length++;
	}
	(!length) ? length++ : length;

	return length;
}

WORD Utilities_rotateBits(WORD invalue, int places, int direction)
{
	WORD outvalue;

    // Rotating left or right?
    if(__ROT_LEFT == direction)
	{
        // First a normal shift
        outvalue = invalue << (places & ((sizeof(int) << 3) - 1));
        outvalue |= invalue >> ((sizeof(int) << 3) - (places & ((sizeof(int) << 3) - 1)));
    }
    else
	{
        // First a normal shift
        outvalue = invalue >> (places & ((sizeof(int) << 3) - 1));
        // Then place the part that's shifted off the screen at the end
        outvalue |= invalue << ((sizeof(int) << 3) - (places & ((sizeof(int) << 3) - 1)));
    }
    return outvalue;
}

char* Utilities_itoa(u32 num, u8 base, u8 digits)
{
#define __CHAR_HOLDER_SIZE		11
	int i = 0;
	static char rev[__CHAR_HOLDER_SIZE];
	int flag = false;
//	static char sign='-';

	if((int)num < 0)
	{
		flag = true;
//		num*=(-1);
	}
	
	for(; i < __CHAR_HOLDER_SIZE - 1; i++)
	{
		rev[__CHAR_HOLDER_SIZE - 2 - i] = numbers[num % base];
		num /= base;
	}
	
	i = 0;
	while (rev[i] == '0')
	{
		i++;
	}
	if(i >= (__CHAR_HOLDER_SIZE - 1 - digits))
	{
		i = __CHAR_HOLDER_SIZE - 1 - digits;
	}

	rev[__CHAR_HOLDER_SIZE - 1] = 0;
	return rev + i;
}

/*
 * When run at startup gets a random number based on the changing CTA
 */
long Utilities_randomSeed()
{
	long random = 1;
	int	rand, prevnum = 0,	count = 1;
	extern unsigned int volatile* _xpstts;

	Clock clock = Game_getClock(Game_getInstance());
	rand = Clock_getTime(clock);

	// repeat through many times to make more random and to allow the CTA value to change multiple times
	while (count < __RANDOM_SEED_CYCLES)
	{
		rand |= * _xpstts;
		rand |= (HW_REGS[TLR] | (HW_REGS[THR] << 8));

		// prevent division by zero
		if(random == 0)
		{
		    random = 1;
        }

		if(rand == 0)
		{
			rand = 1;
        }

		// just randomly doing stuff to the number
		random += ((rand * count) + (count % random) + (prevnum / rand));

		if(rand == prevnum)
		{
			// if the CTA value doesnt change then count up
			count++;
		}
		else
		{
			// if the number does change then restart the counter
			count = 0;
		}

		// keep track of the last number
		prevnum = rand;
	}

	// returns the random seed
	return random;
}

/*
 * Returns a random number in the requested range from the random seed
 */
int Utilities_random(long seed, int randnums)
{
	return seed & randnums? abs((int)(seed % randnums)): 0;
}

/*
 * Check if 2 numbers have an equal sign
 */
int Utilities_equalSign(int a, int b)
{
	return ((a & (1 << sizeof(int))) ==  (b & (1 << sizeof(int))));
}

int Utilities_getDigitCount(int value)
{
	int size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while (value);

	return (size) ? size : 1;
}