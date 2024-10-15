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

#include <Object.h>


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

	asm
	(
		"ldsr	%1, 31	\n\t"	\
		"stsr	31, %0	\n\t"
		: "=r" (result)
		: "r" (number)
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

// maximum values
#define __FIX7_9_MAXIMUM_VALUE_TO_I						((1 << (7 - 1)) - 1)
#define __FIX7_9_MAXIMUM_VALUE							(__I_TO_FIX7_9(__FIX7_9_MAXIMUM_VALUE_TO_I))
#define __FIX10_6_MAXIMUM_VALUE_TO_I					((1 << (10 - 1)) - 1)
#define __FIX10_6_MAXIMUM_VALUE							(__I_TO_FIX10_6(__FIX10_6_MAXIMUM_VALUE_TO_I))
#define __FIX19_13_MAXIMUM_VALUE_TO_I					((1 << (19 - 1)) - 1)
#define __FIX19_13_MAXIMUM_VALUE						(__I_TO_FIX19_13(__FIX19_13_MAXIMUM_VALUE_TO_I))

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
#define __FIX7_9_EXT_TO_F(n)							(float)((n) / 512.0f)
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

#define __FIX7_9_TO_I(n)								(int16)			((n) >> 9)
#define __FIX7_9_EXT_TO_I(n)							(int32)			((n) >> 9)
#define __FIX13_3_TO_I(n)								(int16)			((n) >> 3)
#define __FIX10_6_TO_I(n)								(int32)			((n) >> 6)
#define __FIX10_6_EXT_TO_I(n)							(int32)			((n) >> 6)
#define __FIX19_13_TO_I(n)								(int32)			((n) >> 13)
#define __FIX17_15_TO_I(n)								(int32)			((n) >> 15)


#define __FIX10_6_TO_FIX7_9(n)							(fix7_9)		((n) << 3)
#define __FIX10_6_TO_FIX7_9_EXT(n)						(fix7_9_ext)	((n) << 3)
#define __FIX10_6_TO_FIX10_6(n)							(fix10_6)		((n))
#define __FIX10_6_TO_FIX13_3(n)							(fix13_3)		((n) >> 3)
#define __FIX10_6_TO_FIX17_15(n)						(fix17_15)		(((uint32)n) << 9)
#define __FIX10_6_TO_FIX19_13(n)						(fix19_13)		((n) << 7)
#define __FIX10_6_TO_FIX10_6_EXT(n)						(fix10_6_ext)	((n))
#define __FIX10_6_EXT_TO_FIX10_6(n)						(fix10_6)		((n))
#define __FIX10_6_EXT_TO_FIX7_9(n)						(fix7_9)		((n) << 3)


#define __FIX7_9_TO_FIX7_9(n)							(fix7_9)		((n))
#define __FIX7_9_TO_FIX10_6(n)							(fix10_6)		((n) >> 3)
#define __FIX7_9_EXT_TO_FIX10_6(n)						(fix10_6)		((n) >> 3)
#define __FIX7_9_TO_FIX13_3(n)							(fix13_3)		((n) >> 6)
#define __FIX7_9_TO_FIX17_15(n)							(fix17_15)		((n) << 6)
#define __FIX7_9_TO_FIX19_13(n)							(fix19_13)		((n) << 4)
#define __FIX7_9_TO_FIX7_9_EXT(n)						(fix7_9_ext)	((n))
#define __FIX7_9_EXT_TO_FIX7_9(n)						(fix7_9)		((n))

#define __FIX13_3_TO_FIX7_9(n)							(fix7_9)		((n) << 6)
#define __FIX13_3_TO_FIX10_6(n)							(fix10_6)		((n) << 3)
#define __FIX13_3_TO_FIX19_13(n)						(fix19_13)		((n) << 10)

#define __FIX19_13_TO_FIX7_9(n)							(fix7_9)		((n) >> 4)
#define __FIX19_13_TO_FIX10_6(n)						(fix10_6)		((n) >> 7)
#define __FIX19_13_TO_FIX10_6_EXT(n)					(fix10_6_ext)	((n) >> 7)
#define __FIX19_13_TO_FIX13_3(n)						(fix13_3)		((n) >> 10)
#define __FIX19_13_TO_FIX17_15(n)						(fix17_15)		((n) << 2)
#define __FIX19_13_TO_FIX19_13(n)						(fix19_13)		((n))

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
#define __FIX7_9_EXT_MULT_ROUND(a,b)					(fix7_9_ext)	((((int32)(a)) * ((int32)(b))) / (1 << 9))
#define __FIX13_3_MULT(a,b)								(fix13_3)		((((int32)(a)) * ((int32)(b))) >> 3)
#define __FIX10_6_MULT(a,b)								(fix10_6)		((((int32)(a)) * ((int32)(b))) >> 6)
#define __FIX10_6_EXT_MULT(a,b)							(fix10_6_ext)	((((int32)(a)) * ((int32)(b))) >> 6)
#define __FIX10_6_EXT_MULT_ROUND(a,b)					(fix10_6_ext)	((((int32)(a)) * ((int32)(b))) / (1 << 6))
#define __FIX19_13_MULT(a,b)							(fix19_13)		((((int64)(a)) * ((int64)(b))) >> 13)
#define __FIX19_13_MULT_ROUND(a,b)						(fix19_13)		((((int64)(a)) * ((int64)(b))) / (1 << 13))
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
#define __SIN_LUT_ENTRIES								(512)
#define __SIN_LUT_ENTRIES_2_POWER						(9) // 512 = 2^9
#define __QUARTER_ROTATION_DEGREES						__I_TO_FIXED((signed)(__SIN_LUT_ENTRIES >> 2))
#define __HALF_ROTATION_DEGREES							((signed)__I_TO_FIXED(__SIN_LUT_ENTRIES >> 1))
#define __FULL_ROTATION_DEGREES							((signed)__I_TO_FIXED_EXT(__SIN_LUT_ENTRIES))

extern const int16 _sinLut[];


#if __FIXED_POINT_TYPE == 13

#define __FIXED_INFINITY								0x7FFFFFFF
#define __FIXED_EXT_INFINITY							0x7FFFFFFF
#define __FIXED_MAXIMUM_VALUE							__FIX19_13_MAXIMUM_VALUE

#define __FIXED_TO_I_BITS								(13)

#define __FIXED_INT_PART(n)								__FIX19_13_INT_PART(n)
#define __FIXED_FRAC(n)									__FIX19_13_FRAC(n)
#define __1I_FIXED										__1I_FIX19_13
#define __05F_FIXED										__05F_FIX19_13
#define __I_TO_FIXED(n)									__I_TO_FIX19_13(n)
#define __I_TO_FIXED_EXT(n)								__I_TO_FIX19_13(n)
#define __F_TO_FIXED(n)									__F_TO_FIX19_13(n)
#define __F_TO_FIXED_EXT(n)								__F_TO_FIX19_13(n)
#define __FIXED_TO_I(n)									__FIX19_13_TO_I(n)
#define __FIXED_TO_F(n)									__FIX19_13_TO_F(n)
#define __FIXED_EXT_TO_I(n)								__FIX19_13_TO_I(n)
#define __FIXED_EXT_TO_F(n)								__FIX19_13_TO_F(n)
#define __FIXED_TO_FIX7_9(n)							__FIX19_13_TO_FIX7_9(n)
#define __FIXED_TO_FIX13_3(n)							__FIX19_13_TO_FIX13_3(n)
#define __FIXED_TO_FIX10_6(n)							__FIX19_13_TO_FIX10_6(n)
#define __FIXED_TO_FIX10_6_EXT(n)						__FIX19_13_TO_FIX10_6_EXT(n)
#define __FIXED_TO_FIX17_15(n)							__FIX19_13_TO_FIX17_15(n)
#define __FIXED_TO_FIX19_13(n)							__FIX19_13_TO_FIX19_13(n)
#define __FIXED_TO_FIXED_EXT(n)							__FIX19_13_TO_FIX19_13(n)
#define __FIXED_EXT_TO_FIXED(n)							__FIX19_13_TO_FIX19_13(n)
#define __FIXED_EXT_TO_FIX7_9(n)						__FIX19_13_TO_FIX7_9(n)
#define __FIX7_9_TO_FIXED(n)							__FIX7_9_TO_FIX19_13(n)
#define __FIX13_3_TO_FIXED(n)							__FIX13_3_TO_FIX19_13(n)
#define __FIX10_6_TO_FIXED(n)							__FIX10_6_TO_FIX19_13(n)
#define __FIX10_6_EXT_TO_FIXED(n)						__FIX10_6_EXT_TO_FIX19_13(n)
#define __FIX19_13_TO_FIXED(n)							__FIX19_13_TO_FIX19_13(n)
#define __FIXED_MULT(a,b)								__FIX19_13_MULT(a,b)	
#define __FIXED_EXT_MULT(a,b)							__FIX19_13_MULT(a,b)	
#define __FIXED_EXT_MULT_ROUND(a,b)						__FIX19_13_MULT(a,b)
#define __FIXED_DIV(a,b)								__FIX19_13_DIV(a,b)	
#define __FIXED_EXT_DIV(a,b)							__FIX19_13_DIV(a,b)	

#endif

#if __FIXED_POINT_TYPE == 6

#define __FIXED_INFINITY								0x7FFF
#define __FIXED_EXT_INFINITY							0x7FFFFFFF
#define __FIXED_MAXIMUM_VALUE							__FIX10_6_MAXIMUM_VALUE

#define __FIXED_TO_I_BITS								(6)

#define __FIXED_INT_PART(n)								__FIX10_6_INT_PART(n)
#define __FIXED_FRAC(n)									__FIX10_6_FRAC(n)
#define __1I_FIXED										__1I_FIX10_6
#define __05F_FIXED										__05F_FIX10_6
#define __I_TO_FIXED(n)									__I_TO_FIX10_6(n)
#define __I_TO_FIXED_EXT(n)								__I_TO_FIX10_6_EXT(n)
#define __F_TO_FIXED(n)									__F_TO_FIX10_6(n)
#define __F_TO_FIXED_EXT(n)								__F_TO_FIX10_6_EXT(n)
#define __FIXED_TO_I(n)									__FIX10_6_TO_I(n)
#define __FIXED_TO_F(n)									__FIX10_6_TO_F(n)
#define __FIXED_EXT_TO_I(n)								__FIX10_6_TO_I(n)
#define __FIXED_EXT_TO_F(n)								__FIX10_6_EXT_TO_F(n)
#define __FIXED_TO_FIX7_9(n)							__FIX10_6_TO_FIX7_9(n)
#define __FIXED_TO_FIX7_9_EXT(n)						__FIX10_6_TO_FIX7_9_EXT(n)
#define __FIXED_TO_FIX13_3(n)							__FIX10_6_TO_FIX13_3(n)
#define __FIXED_TO_FIX10_6(n)							__FIX10_6_TO_FIX10_6(n)
#define __FIXED_TO_FIX10_6_EXT(n)						__FIX10_6_TO_FIX10_6_EXT(n)
#define __FIXED_TO_FIX17_15(n)							__FIX10_6_TO_FIX17_15(n)
#define __FIXED_TO_FIX19_13(n)							__FIX10_6_TO_FIX19_13(n)
#define __FIXED_TO_FIXED_EXT(n)							__FIX10_6_TO_FIX10_6_EXT(n)
#define __FIXED_EXT_TO_FIXED(n)							__FIX10_6_EXT_TO_FIX10_6(n)
#define __FIXED_EXT_TO_FIX7_9(n)						__FIX10_6_EXT_TO_FIX7_9(n)
#define __FIX7_9_TO_FIXED(n)							__FIX7_9_TO_FIX10_6(n)
#define __FIX7_9_EXT_TO_FIXED(n)						__FIX7_9_EXT_TO_FIX10_6(n)
#define __FIX13_3_TO_FIXED(n)							__FIX13_3_TO_FIX10_6(n)
#define __FIX10_6_TO_FIXED(n)							__FIX10_6_TO_FIX10_6(n)
#define __FIX10_6_EXT_TO_FIXED(n)						__FIX10_6_EXT_TO_FIX10_6(n)
#define __FIX19_13_TO_FIXED(n)							__FIX19_13_TO_FIX10_6(n)
#define __FIXED_MULT(a,b)								__FIX10_6_MULT(a,b)	
#define __FIXED_EXT_MULT(a,b)							__FIX10_6_EXT_MULT(a,b)	
#define __FIXED_EXT_MULT_ROUND(a,b)						__FIX10_6_EXT_MULT_ROUND(a,b)
#define __FIXED_DIV(a,b)								__FIX10_6_DIV(a,b)	
#define __FIXED_EXT_DIV(a,b)							__FIX10_6_EXT_DIV(a,b)	

#endif

#if __FIXED_POINT_TYPE == 9

#define __FIXED_INFINITY								0x7FFF
#define __FIXED_EXT_INFINITY							0x7FFFFFFF
#define __FIXED_MAXIMUM_VALUE							__FIX7_9_MAXIMUM_VALUE

#define __FIXED_TO_I_BITS								(9)

#define __FIXED_INT_PART(n)								__FIX7_9_INT_PART(n)
#define __FIXED_FRAC(n)									__FIX7_9_FRAC(n)
#define __1I_FIXED										__1I_FIX7_9
#define __05F_FIXED										__05F_FIX7_9
#define __I_TO_FIXED(n)									__I_TO_FIX7_9(n)
#define __I_TO_FIXED_EXT(n)								__I_TO_FIX7_9_EXT(n)
#define __F_TO_FIXED(n)									__F_TO_FIX7_9(n)
#define __F_TO_FIXED_EXT(n)								__F_TO_FIX7_9_EXT(n)
#define __FIXED_TO_I(n)									__FIX7_9_TO_I(n)
#define __FIXED_TO_F(n)									__FIX7_9_TO_F(n)
#define __FIXED_EXT_TO_I(n)								__FIX7_9_TO_I(n)
#define __FIXED_EXT_TO_F(n)								__FIX7_9_EXT_TO_F(n)
#define __FIXED_TO_FIX7_9(n)							__FIX7_9_TO_FIX7_9(n)
#define __FIXED_TO_FIX13_3(n)							__FIX7_9_TO_FIX13_3(n)
#define __FIXED_TO_FIX10_6(n)							__FIX7_9_TO_FIX10_6(n)
#define __FIXED_TO_FIX10_6_EXT(n)						__FIX7_9_TO_FIX10_6_EXT(n)
#define __FIXED_TO_FIX17_15(n)							__FIX7_9_TO_FIX17_15(n)
#define __FIXED_TO_FIX19_13(n)							__FIX7_9_TO_FIX19_13(n)
#define __FIXED_TO_FIXED_EXT(n)							__FIX7_9_TO_FIX7_9_EXT(n)
#define __FIXED_EXT_TO_FIXED(n)							__FIX7_9_EXT_TO_FIX7_9(n)
#define __FIXED_EXT_TO_FIX7_9(n)						__FIX7_9_EXT_TO_FIX7_9(n)
#define __FIX7_9_TO_FIXED(n)							__FIX7_9_TO_FIX7_9(n)
#define __FIX13_3_TO_FIXED(n)							__FIX13_3_TO_FIX7_9(n)
#define __FIX10_6_TO_FIXED(n)							__FIX10_6_TO_FIX7_9(n)
#define __FIX10_6_EXT_TO_FIXED(n)						__FIX10_6_EXT_TO_FIX7_9(n)
#define __FIX19_13_TO_FIXED(n)							__FIX19_13_TO_FIX7_9(n)
#define __FIXED_MULT(a,b)								__FIX7_9_MULT(a,b)	
#define __FIXED_EXT_MULT(a,b)							__FIX7_9_EXT_MULT(a,b)	
#define __FIXED_EXT_MULT_ROUND(a,b)						__FIX7_9_EXT_MULT_ROUND(a,b)
#define __FIXED_DIV(a,b)								__FIX7_9_DIV(a,b)	
#define __FIXED_EXT_DIV(a,b)							__FIX7_9_EXT_DIV(a,b)	

#endif

#define __FIXED_SQUARE(n)								__FIXED_EXT_MULT(n, n)
#define __F_SQUARE_ROOT_OF_2							(1.47f)

extern uint32 _seed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;
extern uint32 _gameRandomSeed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Math : Object
{
	/// @publicsection
	static inline float squareRoot(float number);
	static inline fixed_t Math::squareRootFixed(fixed_ext_t base);
	static int32 powerFast(int32 base, int32 power);
	static int32 intInfinity();
	static fixed_t fixedInfinity();
	static fixed_ext_t fixed_extInfinity();
	static int32 getAngle(fix7_9 x, fix7_9 y);
	static int32 aSin(fix7_9 sin);
	static void resetRandomSeed();
	static inline uint32 randomSeed();
	static inline int32 random(uint32 seed, int32 randnums);
	static inline int32 haveEqualSign(int32 a, int32 b);
	static inline int32 getDigitsCount(int32 value);
	static inline float floor(float x);
}

// retrieve the square root
// this code was taken from the Doom engine
static inline float Math::squareRoot(float radicand)
{
// Disable "warning: dereferencing type-punned pointer will break strict-aliasing rules"
// Doom's code causes a warning because of breaking of aliasing rules
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

	float x = radicand * 0.5F;
	float y = radicand;
	long i = *(long*)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y = y * (1.5F - ( x * y * y ));

	return radicand * y;
}

// retrieve the square root
// this code was taken from the Doom engine
static inline fixed_t Math::squareRootFixed(fixed_ext_t base)
{
// Disable "warning: dereferencing type-punned pointer will break strict-aliasing rules"
// Doom's code causes a warning because of breaking of aliasing rules
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

	float radicand = (float)base;

	float x = radicand * 0.5F;
	float y = radicand;
	long i = *(long*)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y = y * (1.5F - ( x * y * y ));

#if __FIXED_POINT_TYPE == 13
	return (fixed_t)(90.6f * radicand * y );
#else 
#if __FIXED_POINT_TYPE == 6
	return (fixed_t)(8 * radicand * y);
#else
	return (fixed_t)__F_TO_FIXED(radicand * y);
#endif
#endif
}

// These real versions are due to Isaku Wada, 2002/01/09 added
static inline int32 Math::random(uint32 seed, int32 randnums)
{
#ifdef __ADD_USER_INPUT_AND_TIME_TO_RANDOM_SEED
	seed += Clock::getMilliseconds(VUEngine::getClock(VUEngine::getInstance())) + KeypadManager::getAccumulatedUserInput(KeypadManager::getInstance());
#endif

	return 0 != randnums ? __ABS((int32)(seed % randnums)) : 0;
}

/*
 * Taken from https://www.youtube.com/watch?v=RzEjqJHW-NU
 */
static inline uint32 Math::randomSeed()
{
	_seed >>= 1;
	_seed |= ((0x00000001 & (_seed ^ (_seed >> 1))) << ((sizeof(_seed) << 3) - 1));

	return _seed;
}

/*
 * Taken from Shokwav's N64 demo
 */
/*
static inline uint32 Math::randomSeed()
{
	if(!_seed)
	{
		_seed = 7;
	}

	_seed ^= _seed << 13;
	_seed ^= _seed >> 17;
	_seed ^= _seed << 5;

	return _seed;
}
*/

static inline int32 Math::haveEqualSign(int32 a, int32 b)
{
	return ((a & (1 << sizeof(int32))) ==	(b & (1 << sizeof(int32))));
}

static inline int32 Math::getDigitsCount(int32 value)
{
	int32 size = 0;

	do
	{
		value /= 10;
		size++;
	}
	while(0 != value);

	return size;
}

static inline float Math::floor(float x) 
{
	return (float)((long)(x * 2 + 0.5f) >> 1);
}

static inline int32 Math::min(int32 x, int32 y)
{
	return x < y ? x : y;
}

static inline int32 Math::max(int32 x, int32 y)
{
	return x > y ? x : y;
}


#endif
