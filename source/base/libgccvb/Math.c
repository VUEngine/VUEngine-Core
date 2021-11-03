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
static float Math::squareRoot(float number)
{
// Disable "warning: dereferencing type-punned pointer will break strict-aliasing rules"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

	if(0 >= (* ( long * ) &number))
    {
    	return 0;
    }

    // Doom's code causes a warning because of breaking of aliasing rules
	long i;
	float x, y;
#define F 	1.5F

	x = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = * ( float * ) &i;
	y  = y * ( F - ( x * y * y ) );

	return number * y;
}

static int32 Math::powerFast(int32 base, int32 power)
{
	int32 i=0;
	int32 j=0;
	int32 sum = base;
	int32 result = 0;
	int32 limit = base;

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

static int32 Math::multiply(int32 a, int32 b)
{
	return (0 < b) ? a + Math::multiply(a, b - 1) : 0;
}

static int32 Math::doPower(int32 sum, int32 base, int32 power)
{
	return (1 < power) ? Math::doPower(Math::multiply(sum, base), base, power - 1) : sum;
}

static int32 Math::power(int32 base, int32 power)
{
	return 0 == power ? 1 : 1 == power ? base : Math::doPower(base, base, 0 > power ? -power : power);
}

static int32 Math::intInfinity()
{
	return 0x3FFFFFFF;
}

static fix10_6 Math::fix10_6Infinity()
{
	return 0x3FFF;
}

static fix10_6_ext Math::fix10_6_extInfinity()
{
	return 0x3FFFFFFF;
}

static int32 Math::getAngle(fix7_9 x, fix7_9 y)
{
	int32 entry = 0;
	int32 lastEntry = 0;
	static int32 entriesPerQuadrant = (int32)(sizeof(_sinLut) / sizeof(int16)) >> 2;
	static int32 totalEntries = (int32)(sizeof(_sinLut) / sizeof(int16));

	// Determine the quadrant
	if(0 == x)
	{
		// First quadrant
		if(0 < y)
		{
			return entriesPerQuadrant * 1;
		}
		// Fourth quadrant
		else if(0 > y)
		{
			return entriesPerQuadrant * 3;
		}

		return 0;
	}
	else if(0 == y)
	{
		// First quadrant
		if(0 < x)
		{
			return entriesPerQuadrant * 0;
		}
		// Fourth quadrant
		else if(0 > x)
		{
			return entriesPerQuadrant * 2;
		}

		return 0;
	}
	else if(0 < x)
	{
		// First quadrant
		if(0 < y)
		{
			entry = 0;
			lastEntry = entriesPerQuadrant;
		}
		// Fourth quadrant
		else
		{
			entry = entriesPerQuadrant * 3;
			lastEntry = totalEntries;
		}
	}
	// Second quadrant
	else if(0 < y)
	{
		entry = entriesPerQuadrant;
		lastEntry = entry + entriesPerQuadrant;
	}
	// Third quadrant
	else
	{
		entry = entriesPerQuadrant * 2;
		lastEntry = totalEntries - entriesPerQuadrant;
	}

	fix7_9 difference = 1024;
	fix7_9 sin = y;
	int32 angle = 0;

	for(; entry < lastEntry; entry++)
	{
		if(__ABS(sin - _sinLut[entry]) <= difference)
		{
			difference = __ABS(sin - _sinLut[entry]);
			angle = entry;
		}
		else if(__ABS(sin - _sinLut[entry]) > difference)
		{
			break;
		}
	}

	return angle;
}
