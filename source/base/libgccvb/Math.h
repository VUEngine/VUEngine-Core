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

#ifndef MATH_H_
#define MATH_H_


//---------------------------------------------------------------------------------------------------------
// 											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// extern declarations
extern int abs(int);
extern float fabsf(float);

#define __ABS(number)   ((number + (number >> 31)) ^ (number >> 31))

// fixed point macros
#define fix7_9					s16
#define fix13_3					s16
#define fix13_3					s16
#define fix19_13				s32
#define fix15_17				s32

// round functions
#define FIX19_13_ROUNDTOI(n)	((FIX19_13TOI(n)) + (((n) & 0x000001FF) >> 8))
#define F_ROUND(n)				(int)((n) + 0.5f)
#define F_FLOOR(n)				(int)((n) - 0.5f)
#define F_CEIL(n)				(int)((n) + 0.5f)

// convert a float to fixed point and back
#define FTOFIX7_9(n)			(fix7_9)((n) * 512.0f + 0.5f)
#define FTOFIX13_3(n)			(fix13_3)((n) * 8.0f + 0.5f)
#define FTOFIX19_13(n)			(fix19_13)((n) * 8192.0f + 0.5f)
#define FTOFIX15_17(n)			(fix15_17)((n) * 131072.0f + 0.5f)

#define FIX7_9TOF(n)			(float)((n) / 512.0f)
#define FIX13_3TOF(n)			(float)((n) / 8.0f)
#define FIX19_13TOF(n)			(float)((n) / 8192.0f)
#define FIX15_17TOF(n)			(float)((n) / 131072.0f)

// convert an int to fixed point and back
#define ITOFIX7_9(n)			(fix7_9) ((n)<<9)
#define ITOFIX13_3(n)			(fix13_3)((n)<<3)
#define ITOFIX19_13(n)			(fix19_13)((n)<<13)
#define ITOFIX15_17(n)			(fix15_17)((n)<<17)

#define FIX7_9TOI(n)			(s16)((n)>>9)
#define FIX13_3TOI(n)			(s16)((n)>>3)
#define FIX19_13TOI(n)			(s32)((n)>>13)
#define FIX15_17TOI(n)			(s32)((n)>>17)

#define FIX19_13TOFIX7_9(n)		(fix7_9)((n)>>4)
#define FIX19_13TOFIX13_3(n)	(fix13_3)((n)>>10)
#define FIX19_13TOFIX15_17(n)	(fix15_17)((n)<<4)
#define FIX13_3TOFIX7_9(n)		(fix7_9) ((n)<<6)
#define FIX13_3TOTOFIX19_13(n)	(fix19_13) ((n)<<10)
#define FIX7_9TOFIX13_3(n)		(fix13_3)((n)>>6)
#define FIX7_9TOFIX19_13(n)		(fix19_13)((n)<<4)
#define FIX15_17TOFIX19_13(n)	(fix15_17)((n)>>4)

// return fractional part of fixed
#define FIX7_9_FRAC(n)			((n)&0x01FF)
#define FIX13_3_FRAC(n)			((n)&0x0007)
//#define FIX23_9_FRAC(n)		((n)&0x01FF)

// fixed multiplication, what a mess of brackets
// TODO: how do we return an s32 from s16*s16 without forcing a promotion to s32?
#define FIX7_9_MULT(a,b)	(fix7_9) ((((s32)(a))*((s32)(b)))>>9)
#define FIX13_3_MULT(a,b)	(fix13_3)((((s32)(a))*((s32)(b)))>>3)
#define FIX19_13_MULT(a,b)	(fix19_13)((((s64)(a))*((s64)(b)))>>13)
#define FIX15_17_MULT(a,b)	(fix15_17)((((s64)(a))*((s64)(b)))>>17)

// fixed division
#define FIX7_9_DIV(a,b)		(fix7_9) ((((s32)(a))<<9)/((s32)(b)))
#define FIX13_3_DIV(a,b)	(fix13_3)((((s32)(a))<<3)/((s32)(b)))
#define FIX19_13_DIV(a,b)	(fix19_13)((((s64)(a))<<13)/((s64)(b)))
#define FIX15_17_DIV(a,b)	(fix15_17)((((s64)(a))<<17)/((s64)(b)))

static const s16 SINLUT[] =
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
 509,  510,  510,  511,  511,  511,  511,  512, //16
 511,  511,  511,  511,  511,  511,  510,  510, //17
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
-509, -510, -510, -511, -511, -511, -511, -512, //48
-511, -511, -511, -511, -511, -511, -510, -510, //49
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

#define COS(x) SINLUT[((x)+128)&0x1FF]
#define SIN(x) SINLUT[(x)&0x1FF]

#define COSF(x) FIX7_9TOF(SINLUT[((x)+128)&0x1FF])
#define SINF(x) FIX7_9TOF(SINLUT[(x)&0x1FF])

float Math_squareRoot(float number);
int Math_powerFast(int base, int power);


#endif
