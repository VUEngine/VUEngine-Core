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
	int lastRow = __FIX10_6_TO_I(__FIX10_6_MULT((halfHeight << 1), finalScaleY)) + 1;
	int counter = SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance());

	if(rotation->x)
	{
		fix19_13 auxWidth = __FIX10_6_TO_FIX19_13(__FIX10_6_MULT(halfWidth, __I_TO_FIX10_6(2)));
		fix19_13 auxHeight = __FIX10_6_TO_FIX19_13(__FIX10_6_MULT(halfHeight, __I_TO_FIX10_6(2)));

		fix19_13 auxHalfWidth = __FIX10_6_TO_FIX19_13(halfWidth);
		fix19_13 auxHalfHeight = __FIX10_6_TO_FIX19_13(halfHeight);

		fix19_13 edgeSizeX = __FIX19_13_MULT(auxHalfHeight, __FIX7_9_TO_FIX19_13(__COS(rotation->x)));
		fix19_13 edgeSizeY = __FIX19_13_MULT(auxHalfHeight, __FIX7_9_TO_FIX19_13(__COS(rotation->x)));
		fix19_13 edgeSizeZ = __FIX19_13_MULT(auxHalfHeight, __FIX7_9_TO_FIX19_13(__SIN(rotation->x)));

		fix19_13 proportionX = __FIX19_13_DIV(edgeSizeX, auxHalfWidth);
		fix19_13 proportionY = __FIX19_13_DIV(edgeSizeY, auxHalfHeight);

		extern const Optical* _optical;

		fix19_13 scaleXIncrement = __FIX19_13_DIV(proportionX, auxWidth >> (_optical->maximumXViewDistancePower >> 1));
		fix19_13 scaleYIncrement = __FIX19_13_DIV(proportionY, auxHeight >> (_optical->maximumXViewDistancePower >> 1));

		fix19_13 scaleX = __FIX10_6_TO_FIX19_13(finalScaleX) - __FIX19_13_MULT(edgeSizeX, scaleXIncrement) + __FIX19_13_MULT(scaleXIncrement, __I_TO_FIX19_13(i));
		fix19_13 scaleY = __FIX10_6_TO_FIX19_13(finalScaleY) - __FIX19_13_MULT(edgeSizeY, scaleYIncrement) + __FIX19_13_MULT(scaleYIncrement, __I_TO_FIX19_13(i));

		lastRow = __FIX19_13_MULT(edgeSizeY, __I_TO_FIX19_13(2));

		fix10_6 parallaxIncrement = -__FIX10_6_DIV(edgeSizeZ, auxHeight);
		//fix10_6 parallax = __FIX10_6_MULT(halfHeight, parallaxIncrement) - __FIX10_6_DIV(parallaxIncrement, __I_TO_FIX10_6(i));
		fix10_6 parallax =0;

		for(;counter && i <= lastRow; i++, counter--)
		{
			if(0 == scaleX) 
			{
				if(!scaleXIncrement)
				{
					affine[i].pb_y = 0;
					affine[i].parallax = 0;
					affine[i].pd_y = 0;
					affine[i].pa = 0;
					affine[i].pc = 0;
					
					continue;
				}

				scaleX += scaleXIncrement;
			}

			if(0 == scaleY) 
			{
				if(!scaleYIncrement)
				{
					affine[i].pb_y = 0;
					affine[i].parallax = 0;
					affine[i].pd_y = 0;
					affine[i].pa = 0;
					affine[i].pc = 0;
					
					continue;
				}

				scaleY += scaleYIncrement;
			}

			fix10_6 highPrecisionPa = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-rotation->z)), scaleX));
			fix10_6 highPrecisionPb = -__FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-rotation->z)),scaleX));
			fix10_6 highPrecisionPc = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-rotation->z)), scaleY));
			fix10_6 highPrecisionPd = __FIX19_13_TO_FIX10_6(__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-rotation->z)),scaleY));

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

			affine[i].pb_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPb)) + fixedAffineMatrix.dx;
			affine[i].parallax = __FIX10_6_TO_I(parallax);
			affine[i].pd_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPd)) + fixedAffineMatrix.dy;
			affine[i].pa = fixedAffineMatrix.pa;
			affine[i].pc = fixedAffineMatrix.pc;

			parallax += parallaxIncrement;
			scaleX += scaleXIncrement;
			scaleY += scaleYIncrement;
		}
	}
	else
	{
		for(;counter && i <= lastRow; i++, counter--)
		{
			affine[i].pb_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPb)) + fixedAffineMatrix.dx;
			affine[i].parallax = fixedAffineMatrix.parallax;
			affine[i].pd_y = __FIX10_6_TO_FIX13_3(__FIX10_6_MULT(__I_TO_FIX10_6(i), highPrecisionPd)) + fixedAffineMatrix.dy;
			affine[i].pa = fixedAffineMatrix.pa;
			affine[i].pc = fixedAffineMatrix.pc;
		}
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
