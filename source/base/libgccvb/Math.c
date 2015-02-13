/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
// 											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

// retrieve the square root
// this code was taken from the Doom engine
float Math_squareRoot(float number)
{
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}

int Math_powerFast(int base, int power)
{
	int i=0;
	int j=0;
	int sum = base;
	int result = 0;
	int limit = base;

	power = 0 > power ? -power : power;

	for (i = 1; i < power; i++)
	{
		for (j = 0; j < limit; j++)
		{
			result += sum;
		}
		limit = base -1;
		sum = result;
	}

	return 0 == power ? 1 : 1 == power ? base : result;
}

int Math_multiply(int a, int b)
{
	return (0 < b) ? a + Math_multiply(a, b - 1) : 0;
}

int Math_doPower(int sum, int base, int power)
{
	return (1 < power) ? Math_doPower(Math_multiply(sum, base), base, power - 1) : sum;
}

int Math_power(int base, int power)
{
	return 0 == power ? 1 : 1 == power ? base : Math_doPower(base, base, 0 > power ? -power : power);
}