/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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

	while(value > 0)
	{
		value /= 10;

		length++;
	}
	(!length) ? length++ : length;

	return length;
}

char* Utilities_itoa(u32 num, u8 base, u8 digits)
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
	if(i >= (__CHAR_HOLDER_SIZE - 1 - digits))
	{
		i = __CHAR_HOLDER_SIZE - 1 - digits;
	}

	rev[__CHAR_HOLDER_SIZE - 1] = 0;
	return rev + i;
}

/*
 * When run at startup gets a random number based on the changing __CTA
 */
long Utilities_randomSeed()
{
	long random = 1;
	int	rand, prevnum = 0,	count = 1;

	static Clock clock;

	if(!clock)
	{
		clock = Game_getClock(Game_getInstance());
	}

	rand = Clock_getTime(clock);

	// repeat through many times to make more random and to allow the __CTA value to change multiple times
	while(count < __RANDOM_SEED_CYCLES)
	{
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
			// if the __CTA value doesnt change then count up
			count++;
		}
		else
		{
			// if the number does change then restart the counter
			count = 0;
		}

		// keep track of the last number
		prevnum = rand;
		rand = Clock_getTime(clock);
	}

	// returns the random seed
	return random;
}

/*
 * Returns a random number in the requested range from the random seed
 */

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
	while(value);

	return (size) ? size : 1;
}

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)

// Period parameters
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   // constant vector a
#define UPPER_MASK 0x80000000UL // most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // least significant r bits

static unsigned long mt[N]; // the array for the state vector
static int mti=N+1; // mti==N+1 means mt[N] is not initialized

// initializes mt[N] with a seed
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for(mti=1; mti<N; mti++)
    {
        mt[mti] =
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
        // In the previous versions, MSBs of the seed affect
        // only MSBs of the array mt[].
        // 2002/01/09 modified by Makoto Matsumoto
        mt[mti] &= 0xffffffffUL;
        // for >32 bit machines
    }
}

// initialize by an array with array-length
// init_key is the array for initializing keys
// key_length is its length
// slight change for C++, 2004/2/26
void Utilities_initRandomSeedsByArray(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for(; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; // non linear
        mt[i] &= 0xffffffffUL; // for WORDSIZE > 32 machines
        i++; j++;
        if(i>=N) { mt[0] = mt[N-1]; i=1; }
        if(j>=key_length) j=0;
    }
    for(k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; // non linear
        mt[i] &= 0xffffffffUL; // for WORDSIZE > 32 machines
        i++;
        if(i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
}

// generates a random number on [0,0xffffffff]-interval
unsigned long Utilities_generateRandomInt32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    // mag01[x] = x * MATRIX_A  for x=0,1

    if(mti >= N) { // generate N words at one time
        int kk;

        if(mti == N+1)   // if init_genrand() has not been called,
            init_genrand(5489UL); // a default initial seed is used

        for(kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for(;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y? y: 1;
}
*/

// These real versions are due to Isaku Wada, 2002/01/09 added
int Utilities_random(long seed, int randnums)
{
//    unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
//    Utilities_initRandomSeedsByArray(init, length);

//    return Utilities_generateRandomInt32() % randnums;
	return seed & randnums? abs((int)(seed % randnums)): 0;
}
