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

#include <Affine.h>
#include <HardwareManager.h>
#include <BgmapTextureManager.h>
#include <SpriteManager.h>
#include <ParamTableManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern double fabs (double);


//---------------------------------------------------------------------------------------------------------
//											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

s16 Affine_applyAll(u32 param, s16 paramTableRow, fix19_13 x, fix19_13 y, fix13_3 mx, fix13_3 my, fix19_13 halfWidth, fix19_13 halfHeight, const Scale* scale, const Rotation* rotation)
{
	Scale finalScale =
	{
		__FIX7_9_MULT(__COS(rotation->y), scale->x),
		__FIX7_9_MULT(__COS(rotation->x), scale->y),
		__1I_FIX7_9,
	};

	ASSERT(finalScale.x, "Affine::applyAll: 0 x scale");
	ASSERT(finalScale.y, "Affine::applyAll: 0 y scale");

	fix19_13 highPrecisionPa = __FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-rotation->z)), __FIX7_9_TO_FIX19_13(finalScale.x));
	fix19_13 highPrecisionPb = -__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-rotation->z)), __FIX7_9_TO_FIX19_13(finalScale.x));
	fix19_13 highPrecisionPc = __FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-rotation->z)), __FIX7_9_TO_FIX19_13(finalScale.y));
	fix19_13 highPrecisionPd = __FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-rotation->z)), __FIX7_9_TO_FIX19_13(finalScale.y));

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = __FIX19_13_TO_FIX7_9(highPrecisionPa);
	fixedAffineMatrix.pc = __FIX19_13_TO_FIX7_9(highPrecisionPc);

	// bgX + bgWidth - pa * dispX - pb * dispY
	fixedAffineMatrix.dx =
		mx
		+
		__FIX19_13_TO_FIX13_3(__FIX19_13_DIV(halfWidth, __FIX7_9_TO_FIX19_13(abs(finalScale.x))))
		-
		__FIX19_13_TO_FIX13_3
		(
			__FIX19_13_MULT(highPrecisionPa, x)
			+
			__FIX19_13_MULT(highPrecisionPb, y)
		);

	// bgY + bgHeight - pc * dispX - pd * dispY
	fixedAffineMatrix.dy =
		my
		+
		__FIX19_13_TO_FIX13_3(__FIX19_13_DIV(halfHeight, __FIX7_9_TO_FIX19_13(abs(finalScale.y))))
		-
		(
			__FIX19_13_TO_FIX13_3(__FIX19_13_MULT(highPrecisionPc, x))
			+
			__FIX19_13_TO_FIX13_3(__FIX19_13_MULT(highPrecisionPd, y))
		);

	fixedAffineMatrix.parallax = 0;

	AffineEntry* affine = (AffineEntry*)param;

	s16 i = 0 <= paramTableRow ? paramTableRow : 0;
	int lastRow = __FIX19_13_TO_I(halfHeight << 1) + 1;
	int counter = SpriteManager_getMaximumParamTableRowsToComputePerCall(SpriteManager_getInstance());

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	for(;counter && i <= lastRow; i++, counter--)
	{
		affine[i].pb_y = __FIX19_13_TO_FIX13_3(__FIX19_13_MULT(__I_TO_FIX19_13(i), highPrecisionPb)) + fixedAffineMatrix.dx;
		affine[i].parallax = fixedAffineMatrix.parallax;
		affine[i].pd_y = __FIX19_13_TO_FIX13_3(__FIX19_13_MULT(__I_TO_FIX19_13(i), highPrecisionPd)) + fixedAffineMatrix.dy;
		affine[i].pa = fixedAffineMatrix.pa;
		affine[i].pc = fixedAffineMatrix.pc;
	}

	if(i <= lastRow)
	{
		return i;
	}

	affine[i].pb_y = 0;
	affine[i].parallax = 0;
	affine[i].pd_y = 0;
	affine[i].pa = 0;
	affine[i].pc = 0;

	return -1;
}
