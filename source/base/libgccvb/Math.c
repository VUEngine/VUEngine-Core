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

#include <Math.h>
#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern float sqrtf (float);


//---------------------------------------------------------------------------------------------------------
//											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

// retrieve the square root
// this code was taken from the Doom engine
float Math_squareRoot(float number)
{
    // Doom's code causes a warning because of breaking of aliansing rules
	if(0 >= (* ( long * ) &number))
    {
    	return 0;
    }

	long i;
	float x, y;
	const float f = 1.5F;

	x = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = * ( float * ) &i;
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

	for(i = 1; i < power; i++)
	{
		for(j = 0; j < limit; j++)
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

static int Math_doPower(int sum, int base, int power)
{
	return (1 < power) ? Math_doPower(Math_multiply(sum, base), base, power - 1) : sum;
}

int Math_power(int base, int power)
{
	return 0 == power ? 1 : 1 == power ? base : Math_doPower(base, base, 0 > power ? -power : power);
}

int Math_intInfinity()
{
	return 0x3FFFFFFF;
}

fix19_13 Math_fix19_13Infinity()
{
	return 0x3FFFFFFF;
}
