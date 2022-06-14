/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MATH_H_
#define MATH_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// extern declarations
extern int32 abs(int32);
extern float fabsf(float);

#define __USE_WORDS_SIZE_ABS_FUNCTION
#ifdef __USE_WORDS_SIZE_ABS_FUNCTION
#define __ABS(number)   (((number) + ((number) >> 31)) ^ ((number) >> 31))
#else
#ifdef __USE_VB_REGISTER_ABS_FUNCTION
#define __ABS(number)	customAbs(number)
#else
#define __ABS(number)   abs(number)
#endif
#endif

inline int32 customAbs(int32 number)
{
	int32 result = 0;

	asm("				\n\t"	\
		"ldsr %1, 31	\n\t"	\
		"stsr 31, %0	\n\t"	\
		: "=r" (result)			\
		: "r" (number)			\
		:
	);

	return result;
}

// usable only when m is a power of 2
#define __MODULO(n, m)									((n) & ((m) - 1))

// use for faster rounding on fix* values
#define __1I_FIX7_9 									0x0200
#define __05F_FIX7_9									0x0100
#define __1I_FIX7_9_EXT 								0x00000200
#define __05F_FIX7_9_EXT								0x00000100
#define __1I_FIX10_6									0x0040
#define __05F_FIX10_6									0x0020
#define __1I_FIX19_13									0x00002000
#define __05F_FIX19_13									0x00001000

// fixed point macros
#define fix7_9											int16
#define fix7_9_ext										int32
#define fix13_3											int16
#define fix13_3											int16
#define fix10_6											int16
#define fix10_6_ext										int32
#define fix19_13										int32
#define fix17_15										int32

// maximum values
#define __FIX10_6_MAXIMUM_VALUE_TO_I					((1 << (10 - 1)) - 1)
#define __FIX10_6_MAXIMUM_VALUE							(__I_TO_FIX10_6(__FIX10_6_MAXIMUM_VALUE_TO_I))

// round functions
#define __F_ROUND(n)									(int32)((n) + 0.5f)
#define __F_FLOOR(n)									(int32)((n) - 0.5f)
#define __F_CEIL(n)										(int32)((n) + 0.5f)

// convert a float to fixed point and back
#define __F_TO_FIX7_9(n)								(fix7_9)		((n) 	* 512.0f + 0.5f)
#define __F_TO_FIX7_9_EXT(n)							(fix7_9_ext)	((n) 	* 512.0f + 0.5f)
#define __F_TO_FIX13_3(n)								(fix13_3)		((n) 	* 8.0f + 0.5f)
#define __F_TO_FIX10_6(n)								(fix10_6)		((n) 	* 64.0f + 0.5f)
#define __F_TO_FIX10_6_EXT(n)							(fix10_6_ext)	((n) 	* 64.0f + 0.5f)
#define __F_TO_FIX19_13(n)								(fix19_13)		((n) 	* 8192.0f + 0.5f)
#define __F_TO_FIX17_15(n)								(fix17_15)		((n) 	* 32768.0f + 0.5f)

#define __FIX7_9_TO_F(n)								(float)((n) / 512.0f)
#define __FIX7_9_EXT_TO_F(n)							(int32)((n) / 512.0f)
#define __FIX13_3_TO_F(n)								(float)((n) / 8.0f)
#define __FIX10_6_TO_F(n)								(float)((n) / 64.0f)
#define __FIX10_6_EXT_TO_F(n)							(float)((n) / 64.0f)
#define __FIX19_13_TO_F(n)								(float)((n) / 8192.0f)
#define __FIX17_15_TO_F(n)								(float)((n) / 32768.0f)

// convert an int32 to fixed point and back
#define __I_TO_FIX7_9(n)								(fix7_9)		((n) << 9)
#define __I_TO_FIX7_9_EXT(n)							(fix7_9_ext)	((n) << 9)
#define __I_TO_FIX13_3(n)								(fix13_3)		((n) << 3)
#define __I_TO_FIX10_6(n)								(fix10_6)		((n) << 6)
#define __I_TO_FIX10_6_EXT(n)							(fix10_6_ext)	((n) << 6)
#define __I_TO_FIX19_13(n)								(fix19_13)		((n) << 13)
#define __I_TO_FIX17_15(n)								(fix17_15)		((n) << 15)

#define __FIX7_9_TO_I(n)								(int16)((n) >> 9)
#define __FIX7_9_EXT_TO_I(n)							(int32)((n) >> 9)
#define __FIX13_3_TO_I(n)								(int16)((n) >> 3)
#define __FIX10_6_TO_I(n)								(int32)((n) >> 6)
#define __FIX10_6_EXT_TO_I(n)							(int32)((n) >> 6)
#define __FIX19_13_TO_I(n)								(int32)((n) >> 13)
#define __FIX17_15_TO_I(n)								(int32)((n) >> 15)


#define __FIX10_6_TO_FIX7_9(n)							(fix7_9)		((n) << 3)
#define __FIX10_6_TO_FIX13_3(n)							(fix13_3)		((n) >> 3)
#define __FIX10_6_TO_FIX10_6_EXT(n)						(fix10_6_ext)	((n) << 0)
#define __FIX10_6_EXT_TO_FIX10_6(n)						(fix10_6)		((n) << 0)
#define __FIX10_6_TO_FIX19_13(n)						(fix19_13)		((n) << 7)
#define __FIX7_9_TO_FIX19_13(n)							(fix19_13)		((n) << 4)
#define __FIX13_3_TO_FIX7_9(n)							(fix7_9)		((n) << 6)
#define __FIX13_3_TO_FIX10_6(n)							(fix10_6)		((n) << 3)
#define __FIX7_9_TO_FIX13_3(n)							(fix13_3)		((n) >> 6)
#define __FIX7_9_TO_FIX10_6(n)							(fix10_6)		((n) >> 3)
#define __FIX19_13_TO_FIX10_6(n)						(fix10_6)		((n) >> 7)
#define __FIX10_6_TO_FIX17_15(n)						(fix17_15)		(((uint32)n) << 9)

// return the integral part
#define __FIX7_9_INT_PART(n)							(((fix7_9)n) 	& 0xFE00)
#define __FIX13_3_INT_PART(n)							(((fix13_3)n) 	& 0xFFF8)
#define __FIX10_6_INT_PART(n)							(((fix10_6)n) 	& 0xFFC0)
#define __FIX19_13_INT_PART(n)							(((fix19_13)n) 	& 0xFFFFE000)

// return fractional part of fixed
#define __FIX7_9_FRAC(n)								(((fix7_9)n) 	& 0x01FF)
#define __FIX13_3_FRAC(n)								(((fix13_3)n) 	& 0x0007)
#define __FIX10_6_FRAC(n)								(((fix10_6)n) 	& 0x003F)
#define __FIX19_13_FRAC(n)								(((fix19_13)n) 	& 0x00001FFF)

// fixed multiplication, what a mess of brackets
// TODO: how do we return an int32 from int16*int16 without forcing a promotion to int32?
#define __FIX7_9_MULT(a,b)								(fix7_9)		((((int32)(a)) * ((int32)(b))) >> 9)
#define __FIX7_9_EXT_MULT(a,b)							(fix7_9_ext)	((((int32)(a)) * ((int32)(b))) >> 9)
#define __FIX13_3_MULT(a,b)								(fix13_3)		((((int32)(a)) * ((int32)(b))) >> 3)
#define __FIX10_6_MULT(a,b)								(fix10_6)		((((int32)(a)) * ((int32)(b))) >> 6)
#define __FIX10_6_EXT_MULT(a,b)							(fix10_6_ext)	((((int32)(a)) * ((int32)(b))) >> 6)
#define __FIX10_6_EXT_MULT_ROUND(a,b)					(fix10_6_ext)	((((int32)(a)) * ((int32)(b))) / (1 << 6))
#define __FIX19_13_MULT(a,b)							(fix19_13)		((((int64)(a)) * ((int64)(b))) >> 13)
#define __FIX17_15_MULT(a,b)							(fix17_15)		((((int64)(a)) * ((int64)(b))) >> 15)


// fixed division
#define __FIX7_9_DIV(a,b)								(fix7_9)		((((int32)(a)) << 9) / ((int32)(b)))
#define __FIX7_9_EXT_DIV(a,b)							(fix7_9_ext)	((((int32)(a)) << 9) / ((int32)(b)))
#define __FIX13_3_DIV(a,b)								(fix13_3)		((((int32)(a)) << 3) / ((int32)(b)))
#define __FIX10_6_DIV(a,b)								(fix10_6)		((((int32)(a)) << 6) / ((int32)(b)))
#define __FIX10_6_EXT_DIV(a,b)							(fix10_6_ext)	((((int32)(a)) << 6) / ((int32)(b)))
#define __FIX19_13_DIV(a,b)								(fix19_13)		((((int64)(a)) << 13) / ((int64)(b)))
#define __FIX17_15_DIV(a,b)								(fix17_15)		((((int64)(a)) << 15) / ((int64)(b)))

#define __COS(x)										_sinLut[(128 - (x)) & 0x1FF]
#define __SIN(x)										_sinLut[(x) & 0x1FF]

#define __COSF(x)										__FIX7_9_TO_F(__COS(x))
#define __SINF(x)										__FIX7_9_TO_F(__SIN(x))

//#define __SIN_LUT_ENTRIES								(sizeof(_sinLut) / sizeof(int16))
#define __SIN_LUT_ENTRIES								(64 * 8)
#define __QUARTER_ROTATION_DEGREES						__I_TO_FIX10_6((signed)(__SIN_LUT_ENTRIES >> 2))
#define __HALF_ROTATION_DEGREES							((signed)__I_TO_FIX10_6(__SIN_LUT_ENTRIES >> 1))
#define __FULL_ROTATION_DEGREES							((signed)__I_TO_FIX10_6_EXT(__SIN_LUT_ENTRIES))

extern const int16 _sinLut[];


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Math : Object
{
	/// @publicsection
	static inline float squareRoot(float number);
	static int32 powerFast(int32 base, int32 power);
	static int32 intInfinity();
	static fix10_6 fix10_6Infinity();
	static fix10_6_ext fix10_6_extInfinity();
	static int32 getAngle(fix7_9 x, fix7_9 y);
}


// retrieve the square root
// this code was taken from the Doom engine
static inline float Math::squareRoot(float number)
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


#endif
