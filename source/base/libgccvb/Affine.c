/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

static s16 Affine::applyAll(u32 param, s16 paramTableRow, fix10_6 x, fix10_6 y, fix13_3 mx, fix13_3 my, fix10_6 halfWidth, fix10_6 halfHeight, const Scale* scale, const Rotation* rotation)
{
	fix10_6 finalScaleX = __FIX10_6_MULT(__FIX7_9_TO_FIX10_6(__COS(rotation->y)), __FIX7_9_TO_FIX10_6(scale->x));
	fix10_6 finalScaleY = __FIX10_6_MULT(__FIX7_9_TO_FIX10_6(__COS(rotation->x)), __FIX7_9_TO_FIX10_6(scale->y));

	ASSERT(finalScaleX, "Affine::applyAll: 0 x scale");
	ASSERT(finalScaleY, "Affine::applyAll: 0 y scale");

	fix10_6 highPrecisionPa = __FIX10_6_DIV(__FIX7_9_TO_FIX10_6(__COS(-rotation->z)), finalScaleX);
	fix10_6 highPrecisionPb = -__FIX10_6_DIV(__FIX7_9_TO_FIX10_6(__SIN(-rotation->z)), finalScaleX);
	fix10_6 highPrecisionPc = __FIX10_6_DIV(__FIX7_9_TO_FIX10_6(__SIN(-rotation->z)), finalScaleY);
	fix10_6 highPrecisionPd = __FIX10_6_DIV(__FIX7_9_TO_FIX10_6(__COS(-rotation->z)), finalScaleY);

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = __FIX10_6_TO_FIX7_9(highPrecisionPa);
	fixedAffineMatrix.pc = __FIX10_6_TO_FIX7_9(highPrecisionPc);

	// bgX + bgWidth - pa * dispX - pb * dispY
	fixedAffineMatrix.dx =
		mx
		+
		__FIX10_6_TO_FIX13_3
		(
			halfWidth
			-
			(
				__FIX10_6_MULT(highPrecisionPa, x)
				+
				__FIX10_6_MULT(highPrecisionPb, y)
			)
		);

	// bgY + bgHeight - pc * dispX - pd * dispY
	fixedAffineMatrix.dy =
		my
		+
		__FIX10_6_TO_FIX13_3
		(
			halfHeight
			-
			(
				__FIX10_6_MULT(highPrecisionPc, x)
				+
				__FIX10_6_MULT(highPrecisionPd, y)
			)
		);

	fixedAffineMatrix.parallax = 0;

	AffineEntry* affine = (AffineEntry*)param;

	s16 i = 0 <= paramTableRow ? paramTableRow : 0;
	int lastRow = __FIX10_6_MULT(__FIX10_6_TO_I(halfHeight << 1), __FIX7_9_TO_FIX10_6(scale->y)) + 1;
	int counter = SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance());

	for(;counter && i <= lastRow; i++, counter--)
	{
		affine[i].pb_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPb)) + fixedAffineMatrix.dx;
		affine[i].parallax = fixedAffineMatrix.parallax;
		affine[i].pd_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPd)) + fixedAffineMatrix.dy;
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
