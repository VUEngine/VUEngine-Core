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


const int16 _sinLut[] =
{
   0,    6,   12,   18,   25,   31,   37,   43, //1
  50,   56,   62,   68,   75,   81,   87,   93, //2
  99,  106,  112,  118,  124,  130,  136,  142, //3
 148,  154,  160,  166,  172,  178,  184,  190, //4
 195,  201,  207,  213,  218,  224,  230,  235, //5
 241,  246,  252,  257,  263,  268,  273,  279, //6
 284,  289,  294,  299,  304,  310,  314,  319, //7
 324,  329,  334,  339,  343,  348,  353,  357, //8
 362,  366,  370,  375,  379,  383,  387,  391, //9
 395,  399,  403,  407,  411,  414,  418,  422, //10
 425,  429,  432,  435,  439,  442,  445,  448, //11
 451,  454,  457,  460,  462,  465,  468,  470, //12
 473,  475,  477,  479,  482,  484,  486,  488, //13
 489,  491,  493,  495,  496,  498,  499,  500, //14
 502,  503,  504,  505,  506,  507,  508,  508, //15
 509,  510,  510,  511,  511,  511,  511,  511, //16
 512,  511,  511,  511,  511,  511,  510,  510, //17
 509,  508,  508,  507,  506,  505,  504,  503, //18
 502,  500,  499,  498,  496,  495,  493,  491, //19
 489,  488,  486,  484,  482,  479,  477,  475, //20
 473,  470,  468,  465,  462,  460,  457,  454, //21
 451,  448,  445,  442,  439,  435,  432,  429, //22
 425,  422,  418,  414,  411,  407,  403,  399, //23
 395,  391,  387,  383,  379,  375,  370,  366, //24
 362,  357,  353,  348,  343,  339,  334,  329, //25
 324,  319,  314,  310,  304,  299,  294,  289, //26
 284,  279,  273,  268,  263,  257,  252,  246, //27
 241,  235,  230,  224,  218,  213,  207,  201, //28
 195,  190,  184,  178,  172,  166,  160,  154, //29
 148,  142,  136,  130,  124,  118,  112,  106, //30
  99,   93,   87,   81,   75,   68,   62,   56, //31
  50,   43,   37,   31,   25,   18,   12,    6, //32
   0,   -6,  -12,  -18,  -25,  -31,  -37,  -43, //33
 -50,  -56,  -62,  -68,  -75,  -81,  -87,  -93, //34
 -99, -106, -112, -118, -124, -130, -136, -142, //35
-148, -154, -160, -166, -172, -178, -184, -190, //36
-195, -201, -207, -213, -218, -224, -230, -235, //37
-241, -246, -252, -257, -263, -268, -273, -279, //38
-284, -289, -294, -299, -305, -310, -315, -319, //39
-324, -329, -334, -339, -343, -348, -353, -357, //40
-362, -366, -370, -375, -379, -383, -387, -391, //41
-395, -399, -403, -407, -411, -414, -418, -422, //42
-425, -429, -432, -435, -439, -442, -445, -448, //43
-451, -454, -457, -460, -462, -465, -468, -470, //44
-473, -475, -477, -479, -482, -484, -486, -488, //45
-489, -491, -493, -495, -496, -498, -499, -500, //46
-502, -503, -504, -505, -506, -507, -508, -508, //47
-509, -510, -510, -511, -511, -511, -511, -511, //48
-512, -511, -511, -511, -511, -511, -510, -510, //49
-509, -508, -508, -507, -506, -505, -504, -503, //50
-502, -500, -499, -498, -496, -495, -493, -491, //51
-489, -488, -486, -484, -482, -479, -477, -475, //52
-473, -470, -468, -465, -462, -460, -457, -454, //53
-451, -448, -445, -442, -439, -435, -432, -429, //54
-425, -422, -418, -414, -411, -407, -403, -399, //55
-395, -391, -387, -383, -379, -375, -370, -366, //56
-362, -357, -353, -348, -343, -339, -334, -329, //57
-324, -319, -314, -310, -304, -299, -294, -289, //58
-284, -279, -273, -268, -263, -257, -252, -246, //59
-241, -235, -230, -224, -218, -213, -207, -201, //60
-195, -190, -184, -178, -172, -166, -160, -154, //61
-148, -142, -136, -130, -124, -118, -112, -106, //62
 -99,  -93,  -87,  -81,  -75,  -68,  -62,  -56, //63
 -50,  -43,  -37,  -31,  -25,  -18,  -12,   -6  //64
};


//---------------------------------------------------------------------------------------------------------
//											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

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

static int32 Math::getAngle(fix7_9 cos, fix7_9 sin)
{
	int32 entry = 0;
	int32 lastEntry = 0;
	static int32 entriesPerQuadrant = (int32)(sizeof(_sinLut) / sizeof(int16)) >> 2;
	static int32 totalEntries = (int32)(sizeof(_sinLut) / sizeof(int16));

	// Determine the quadrant
	if(0 == cos)
	{
		// First quadrant
		if(0 < sin)
		{
			return entriesPerQuadrant * 1;
		}
		// Fourth quadrant
		else if(0 > sin)
		{
			return entriesPerQuadrant * 3;
		}

		return 0;
	}
	else if(0 == sin)
	{
		// First quadrant
		if(0 < cos)
		{
			return entriesPerQuadrant * 0;
		}
		// Fourth quadrant
		else if(0 > cos)
		{
			return entriesPerQuadrant * 2;
		}

		return 0;
	}
	else if(0 < cos)
	{
		// First quadrant
		if(0 < sin)
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
	else if(0 < sin)
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
