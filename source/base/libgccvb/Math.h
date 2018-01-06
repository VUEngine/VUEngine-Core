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

#ifndef MATH_H_
#define MATH_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


//---------------------------------------------------------------------------------------------------------
//											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// extern declarations
extern int abs(int);
extern float fabsf(float);

//#define __ABS(number)   (((number) + ((number) >> 31)) ^ ((number) >> 31))
#define __ABS(number)   abs(number)

// usable only when m is a power of 2
#define __MODULO(n, m)			(n & (m - 1))


// fixed point macros
#define fix7_9					s16
#define fix13_3					s16
#define fix13_3					s16
#define fix10_6					s16
#define fix10_6_ext				s32
#define fix19_13				s32


// round functions
#define __F_ROUND(n)				(int)((n) + 0.5f)
#define __F_FLOOR(n)				(int)((n) - 0.5f)
#define __F_CEIL(n)					(int)((n) + 0.5f)

// convert a float to fixed point and back
#define __F_TO_FIX7_9(n)			(fix7_9)		((n) 	* 512.0f + 0.5f)
#define __F_TO_FIX13_3(n)			(fix13_3)		((n) 	* 8.0f + 0.5f)
#define __F_TO_FIX10_6(n)			(fix10_6)		((n) 	* 64.0f + 0.5f)
#define __F_TO_FIX10_6_EXT(n)		(fix10_6_ext)	((n) 	* 64.0f + 0.5f)
#define __F_TO_FIX19_13(n)			(fix19_13)		((n) 	* 8192.0f + 0.5f)

#define __FIX7_9_TO_F(n)			(float)((n) / 512.0f)
#define __FIX13_3_TO_F(n)			(float)((n) / 8.0f)
#define __FIX10_6_TO_F(n)			(float)((n) / 64.0f)
#define __FIX10_6_EXT_TO_F(n)		(float)((n) / 64.0f)
#define __FIX19_13_TO_F(n)			(float)((n) / 8192.0f)

// convert an int to fixed point and back
#define __I_TO_FIX7_9(n)			(fix7_9)		((n) << 9)
#define __I_TO_FIX13_3(n)			(fix13_3)		((n) << 3)
#define __I_TO_FIX10_6(n)			(fix10_6)		((n) << 6)
#define __I_TO_FIX10_6_EXT(n)		(fix10_6_ext)	((n) << 6)
#define __I_TO_FIX19_13(n)			(fix19_13)		((n) << 13)

#define __FIX7_9_TO_I(n)			(s16)((n) >> 9)
#define __FIX13_3_TO_I(n)			(s16)((n) >> 3)
#define __FIX10_6_TO_I(n)			(s32)((n) >> 6)
#define __FIX10_6_EXT_TO_I(n)		(s32)((n) >> 6)
#define __FIX19_13_TO_I(n)			(s32)((n) >> 13)

#define __FIX10_6_TO_FIX7_9(n)		(fix7_9)		((n) << 3)
#define __FIX10_6_TO_FIX13_3(n)		(fix13_3)		((n) >> 3)
#define __FIX10_6_TO_FIX10_6_EXT(n)	(fix10_6_ext)	((n) << 0)
#define __FIX10_6_EXT_TO_FIX10_6(n)	(fix10_6)		((n) << 0)
#define __FIX10_6_TO_FIX19_13(n)	(fix19_13)		((n) << 7)
#define __FIX13_3_TO_FIX7_9(n)		(fix7_9)		((n) << 6)
#define __FIX13_3_TO_FIX10_6(n)		(fix10_6)		((n) << 3)
#define __FIX7_9_TO_FIX13_3(n)		(fix13_3)		((n) >> 6)
#define __FIX7_9_TO_FIX10_6(n)		(fix10_6)		((n) >> 3)
#define __FIX19_13_TO_FIX10_6(n)	(fix10_6)		((n) >> 7)

// return the integral part
#define __FIX7_9_INT_PART(n)		(((fix7_9)n) 	& 0xFE00)
#define __FIX13_3_INT_PART(n)		(((fix13_3)n) 	& 0xFFF8)
#define __FIX10_6_INT_PART(n)		(((fix10_6)n) 	& 0xFFC0)
#define __FIX19_13_INT_PART(n)		(((fix19_13)n) 	& 0xFFFFE000)

// return fractional part of fixed
#define __FIX7_9_FRAC(n)			(((fix7_9)n) 	& 0x01FF)
#define __FIX13_3_FRAC(n)			(((fix13_3)n) 	& 0x0007)
#define __FIX10_6_FRAC(n)			(((fix10_6)n) 	& 0x003F)
#define __FIX19_13_FRAC(n)			(((fix19_13)n) 	& 0x00001FFF)

// fixed multiplication, what a mess of brackets
// TODO: how do we return an s32 from s16*s16 without forcing a promotion to s32?
#define __FIX7_9_MULT(a,b)			(fix7_9)	((((s32)(a)) * ((s32)(b))) >> 9)
#define __FIX13_3_MULT(a,b)			(fix13_3)	((((s32)(a)) * ((s32)(b))) >> 3)
#define __FIX10_6_MULT(a,b)			(fix10_6)	((((s32)(a)) * ((s32)(b))) >> 6)
#define __FIX10_6_EXT_MULT(a,b)					((((s32)(a)) * ((s32)(b))) >> 6)
#define __FIX19_13_MULT(a,b)		(fix19_13)	((((s64)(a)) * ((s64)(b))) >> 13)


// fixed division
#define __FIX7_9_DIV(a,b)			(fix7_9)	((((s32)(a)) << 9) / ((s32)(b)))
#define __FIX13_3_DIV(a,b)			(fix13_3)	((((s32)(a)) << 3) / ((s32)(b)))
#define __FIX10_6_DIV(a,b)			(fix10_6)	((((s32)(a)) << 6) / ((s32)(b)))
#define __FIX10_6_EXT_DIV(a,b)					((((s32)(a)) << 6) / ((s32)(b)))
#define __FIX19_13_DIV(a,b)			(fix19_13)	((((s64)(a)) << 13) / ((s64)(b)))

#define __COS(x) _sinLut[(128 - (x)) & 0x1FF]
#define __SIN(x) _sinLut[(x) & 0x1FF]

#define __COSF(x) __FIX7_9_TO_F(__COS(x))
#define __SINF(x) __FIX7_9_TO_F(__SIN(x))

#define __SIN_LUT_ENTRIES				(sizeof(_sinLut) / sizeof(s16))
#define __QUARTER_ROTATION_DEGREES		((signed)(__SIN_LUT_ENTRIES >> 2))
#define __HALF_ROTATION_DEGREES			((signed)(__SIN_LUT_ENTRIES >> 1))
#define __FULL_ROTATION_DEGREES			((signed)(__SIN_LUT_ENTRIES))

static const s16 _sinLut[] =
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

float Math_squareRoot(float number);
int Math_powerFast(int base, int power);
int Math_intInfinity();
fix10_6 Math_fix10_6Infinity();


#endif
