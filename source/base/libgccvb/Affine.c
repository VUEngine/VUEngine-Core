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

static int16 Affine::applyAll(uint32 param, int16 paramTableRow, fixed_t x, fixed_t y, fix13_3 mx, fix13_3 my, fixed_t halfWidth, fixed_t halfHeight, const Scale* scale, const Rotation* rotation)
{
	NM_ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	NM_ASSERT(scale->y, "Affine::applyAll: 0 y scale");

	fixed_t finalScaleX = __FIXED_MULT(__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->y))), __FIX7_9_TO_FIXED(scale->x));
	fixed_t finalScaleY = __FIXED_MULT(__FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->x))), __FIX7_9_TO_FIXED(scale->y));

	if(0 == finalScaleX)
	{
		finalScaleX = __F_TO_FIX7_9(0.01f);
	}

	if(0 == finalScaleY)
	{
		finalScaleY = __F_TO_FIX7_9(0.01f);
	}

	fixed_t highPrecisionPa = __FIXED_DIV(__FIX7_9_TO_FIXED(__COS(-__FIXED_TO_I(rotation->z))), finalScaleX);
	fixed_t highPrecisionPb = -__FIXED_DIV(__FIX7_9_TO_FIXED(__SIN(-__FIXED_TO_I(rotation->z))), finalScaleX);
	fixed_t highPrecisionPc = __FIXED_DIV(__FIX7_9_TO_FIXED(__SIN(-__FIXED_TO_I(rotation->z))), finalScaleY);
	fixed_t highPrecisionPd = __FIXED_DIV(__FIX7_9_TO_FIXED(__COS(-__FIXED_TO_I(rotation->z))), finalScaleY);

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = __FIXED_TO_FIX7_9(highPrecisionPa);
	fixedAffineMatrix.pc = __FIXED_TO_FIX7_9(highPrecisionPc);

	// bgX + bgWidth - pa * dispX - pb * dispY
	fixedAffineMatrix.dx =
		mx
		+
		__FIXED_TO_FIX13_3
		(
			halfWidth
			-
			(
				__FIXED_MULT(highPrecisionPa, x)
				+
				__FIXED_MULT(highPrecisionPb, y)
			)
		);

	// bgY + bgHeight - pc * dispX - pd * dispY
	fixedAffineMatrix.dy =
		my
		+
		__FIXED_TO_FIX13_3
		(
			halfHeight
			-
			(
				__FIXED_MULT(highPrecisionPc, x)
				+
				__FIXED_MULT(highPrecisionPd, y)
			)
		);

	fixedAffineMatrix.parallax = 0;

	AffineEntry* affine = (AffineEntry*)param;


	int16 i = 0 <= paramTableRow ? paramTableRow : 0;
	int32 lastRow = __FIXED_TO_I(__FIXED_MULT((halfHeight << 1), finalScaleY)) + 1;
	int32 counter = SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance());

	if(rotation->x)
	{
		fix19_13 topEdgeSizeZ = __FIX19_13_MULT(__FIXED_TO_FIX19_13(halfHeight), __FIX7_9_TO_FIX19_13(__SIN(__FIXED_TO_I(rotation->x))));
		fix19_13 bottomEdgeSizeZ = -topEdgeSizeZ;

		extern const Optical* _optical;

		fix19_13 scaleXDifference = __FIX19_13_MULT(__I_TO_FIX19_13(1), __FIXED_TO_FIX19_13(__FIXED_DIV(__PIXELS_TO_METERS(__FIX19_13_TO_I(topEdgeSizeZ)), _optical->scalingFactor)));
		fix19_13 scaleYDifference = __FIX7_9_TO_FIX19_13(__SIN(__FIXED_TO_I(rotation->x)));

		fix19_13 scaleXFactor = __FIXED_TO_FIX19_13(finalScaleX);
		fix19_13 scaleYFactor = __FIXED_TO_FIX19_13(finalScaleX);

		fix19_13 topEdgeScaleX = __FIX19_13_MULT(__I_TO_FIX19_13(1) - scaleXDifference, (scaleXFactor));
		fix19_13 bottomEdgeScaleX = __FIX19_13_MULT(__I_TO_FIX19_13(1) + scaleXDifference, (scaleXFactor));

		fix19_13 topEdgeScaleY = __FIX19_13_MULT(__I_TO_FIX19_13(1) - scaleYDifference, (scaleYFactor));
		fix19_13 bottomEdgeScaleY = __FIX19_13_MULT(__I_TO_FIX19_13(1) + scaleYDifference, (scaleYFactor));

		fix19_13 scaleXIncrement = __FIX19_13_DIV(bottomEdgeScaleX - topEdgeScaleX, __I_TO_FIX19_13(lastRow));
		fix19_13 scaleYIncrement = __FIX19_13_DIV(bottomEdgeScaleY - topEdgeScaleY, __I_TO_FIX19_13(lastRow));

		fix19_13 scaleX = topEdgeScaleX;
		fix19_13 scaleY = topEdgeScaleY;

		fixed_t parallaxIncrement = -__FIX19_13_TO_FIXED(__FIX19_13_DIV((bottomEdgeSizeZ - topEdgeSizeZ), __I_TO_FIX19_13(lastRow)));
		fixed_t parallax = -__FIXED_MULT(__I_TO_FIXED(lastRow / 2), parallaxIncrement) + __FIXED_MULT(parallaxIncrement, __I_TO_FIXED(i));

/*
PRINT_TEXT("    ", 1, 5);
PRINT_INT(rotation->x, 1, 5);
PRINT_TEXT("    ", 1, 7);
PRINT_INT(__FIX19_13_TO_F(topEdgeScaleX)*1000, 1, 7);
PRINT_TEXT("    ", 10, 7);
PRINT_INT(__FIX19_13_TO_F(topEdgeScaleY)*1000, 10, 7);
PRINT_TEXT("    ", 1, 8);
PRINT_INT(__FIX19_13_TO_F(bottomEdgeScaleX)*1000, 1, 8);
PRINT_TEXT("    ", 10, 8);
PRINT_INT(__FIX19_13_TO_F(bottomEdgeScaleY)*1000, 10, 8);

PRINT_TEXT("    ", 1, 9);
PRINT_INT(__FIX19_13_TO_F(topEdgeSizeZ)*1000, 1, 9);
PRINT_TEXT("    ", 10, 9);
PRINT_INT(__FIX19_13_TO_F(bottomEdgeSizeZ)*1000, 10, 9);

PRINT_TEXT("    ", 1, 10);
PRINT_INT(__FIX19_13_TO_F(scaleX)*1000, 1, 10);
PRINT_TEXT("    ", 10, 10);
PRINT_INT(__FIX19_13_TO_F(scaleY)*1000, 10, 10);

PRINT_TEXT("    ", 1, 16);
PRINT_INT(lastRow, 1, 16);
//PRINT_INT((_optical->maximumXViewDistancePower), 1, 14);
//PRINT_INT(__METERS_TO_PIXELS(_optical->scalingFactor), 1, 15);
*/

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

			highPrecisionPa = (__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-__FIXED_TO_I(rotation->z))), scaleX));
			highPrecisionPb = -(__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-__FIXED_TO_I(rotation->z))), scaleX));
			highPrecisionPc = (__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__SIN(-__FIXED_TO_I(rotation->z))), scaleY));
			highPrecisionPd = (__FIX19_13_DIV(__FIX7_9_TO_FIX19_13(__COS(-__FIXED_TO_I(rotation->z))), scaleY));

			fixedAffineMatrix.pa = __FIX19_13_TO_FIX7_9(highPrecisionPa);
			fixedAffineMatrix.pc = __FIX19_13_TO_FIX7_9(highPrecisionPc);

			// bgX + bgWidth - pa * dispX - pb * dispY
			fixedAffineMatrix.dx =
				mx
				+
				__FIX19_13_TO_FIX13_3
				(
					__FIXED_TO_FIX19_13(halfWidth)
					-
					(
						__FIX19_13_MULT(highPrecisionPa, __FIXED_TO_FIX19_13(x))
						+
						__FIX19_13_MULT(highPrecisionPb, __FIXED_TO_FIX19_13(y))
					)
				);

			// bgY + bgHeight - pc * dispX - pd * dispY
			fixedAffineMatrix.dy =
				my
				+
				__FIX19_13_TO_FIX13_3
				(
					__FIXED_TO_FIX19_13(halfHeight)
					-
					(
						__FIX19_13_MULT(highPrecisionPc, __FIXED_TO_FIX19_13(x))
						+
						__FIX19_13_MULT(highPrecisionPd, __FIXED_TO_FIX19_13(y))
					)
				);

			affine[i].pb_y = __FIX19_13_TO_FIX13_3(__FIXED_MULT(__I_TO_FIXED(i), highPrecisionPb)) + fixedAffineMatrix.dx;
			affine[i].parallax = __FIXED_TO_I(parallax);
			affine[i].pd_y = __FIX19_13_TO_FIX13_3(__FIX19_13_MULT(__I_TO_FIX19_13(i), highPrecisionPd)) + fixedAffineMatrix.dy;
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
			affine[i].pb_y = __FIXED_TO_FIX13_3(__FIXED_MULT(__I_TO_FIXED(i), highPrecisionPb)) + fixedAffineMatrix.dx;
			affine[i].parallax = fixedAffineMatrix.parallax;
			affine[i].pd_y = __FIXED_TO_FIX13_3(__FIXED_MULT(__I_TO_FIXED(i), highPrecisionPd)) + fixedAffineMatrix.dy;
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

static int16 Affine::rotate(uint32 param, int16 paramTableRow, fixed_t x, fixed_t y, fix13_3 mx, fix13_3 my, fixed_t halfWidth, fixed_t halfHeight, const Rotation* rotation)
{
	fixed_t highPrecisionPa = __FIX7_9_TO_FIXED(__COS(-__FIXED_TO_I(rotation->z)));
	fixed_t highPrecisionPb = -__FIX7_9_TO_FIXED(__SIN(-__FIXED_TO_I(rotation->z)));
	fixed_t highPrecisionPc = __FIX7_9_TO_FIXED(__SIN(-__FIXED_TO_I(rotation->z)));
	fixed_t highPrecisionPd = __FIX7_9_TO_FIXED(__COS(-__FIXED_TO_I(rotation->z)));

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = __FIXED_TO_FIX7_9(highPrecisionPa);
	fixedAffineMatrix.pc = __FIXED_TO_FIX7_9(highPrecisionPc);

	// bgX + bgWidth - pa * dispX - pb * dispY
	fixedAffineMatrix.dx =
		mx
		+
		__FIXED_TO_FIX13_3
		(
			halfWidth
			-
			(
				__FIXED_MULT(highPrecisionPa, x)
				+
				__FIXED_MULT(highPrecisionPb, y)
			)
		);

	// bgY + bgHeight - pc * dispX - pd * dispY
	fixedAffineMatrix.dy =
		my
		+
		__FIXED_TO_FIX13_3
		(
			halfHeight
			-
			(
				__FIXED_MULT(highPrecisionPc, x)
				+
				__FIXED_MULT(highPrecisionPd, y)
			)
		);

	fixedAffineMatrix.parallax = 0;

	AffineEntry* affine = (AffineEntry*)param;


	int16 i = 0 <= paramTableRow ? paramTableRow : 0;
	int32 lastRow = __FIXED_TO_I((halfHeight << 1)) + 1;

/*
	int16 i = 0 <= paramTableRow ? paramTableRow : 0;
	int32 lastRow = __FIXED_TO_I(__FIXED_MULT((halfHeight << 1), finalScaleY)) + 1;
	int32 counter = SpriteManager::getMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance());
*/
//	for(;counter && i <= lastRow; i++, counter--)
	for(;i <= lastRow; i++, )
	{
		affine[i].pb_y = __FIXED_TO_FIX13_3(__FIXED_MULT(__I_TO_FIXED(i), highPrecisionPb)) + fixedAffineMatrix.dx;
		affine[i].parallax = fixedAffineMatrix.parallax;
		affine[i].pd_y = __FIXED_TO_FIX13_3(__FIXED_MULT(__I_TO_FIXED(i), highPrecisionPd)) + fixedAffineMatrix.dy;
		affine[i].pa = fixedAffineMatrix.pa;
		affine[i].pc = fixedAffineMatrix.pc;
	}
/*
	if(i <= lastRow)
	{
		return i;
	}
*/
	affine[i].pb_y = 0;
	affine[i].parallax = 0;
	affine[i].pd_y = 0;
	affine[i].pa = 0;
	affine[i].pc = 0;

	return -1;
}
