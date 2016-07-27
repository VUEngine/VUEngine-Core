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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Affine.h>
#include <HardwareManager.h>
#include <BgmapTextureManager.h>
#include <SpriteManager.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern double fabs (double);


//---------------------------------------------------------------------------------------------------------
// 											DEFINES
//---------------------------------------------------------------------------------------------------------

typedef struct AffineEntry
{
    fix13_3	pb_y;		// *y+Dx /= 8.0
    s16		paralax;
    fix13_3	pd_y;		// *y+Dy /= 8.0
    fix7_9	pa;			// /=512.0
    fix7_9	pc;			// /=512.0
    u16 spacer[3];		//unknown
} AffineEntry ;

typedef struct FixedAffineMatrix
{
	fix7_9 pa;
	fix13_3 pb;
	fix7_9 pc;
	fix13_3 pd;
	fix13_3 dx;
	fix13_3 dy;
	s16   paralax;
} FixedAffineMatrix ;


//---------------------------------------------------------------------------------------------------------
// 											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

fix19_13 Affine_applyAll(fix19_13 paramTableRow, const Scale* scale, const Rotation* rotation, const WORLD* worldPointer, int finalRow, int mxDisplacement, int myDisplacement)
{
	ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	ASSERT(scale->y, "Affine::applyAll: 0 y scale");

	fix19_13 halfWidth = ITOFIX19_13((worldPointer->w - (mxDisplacement? worldPointer->mx: 0)) >> 1);
	fix19_13 halfheight = ITOFIX19_13((worldPointer->h - (myDisplacement? worldPointer->my: 0)) >> 1);

    fix19_13 highPrecisionPa = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->x));
    fix19_13 highPrecisionPb = -FIX19_13_DIV(FIX7_9TOFIX19_13(SIN(rotation->z)), FIX7_9TOFIX19_13(scale->x));
    fix19_13 highPrecisionPc = FIX19_13_DIV(FIX7_9TOFIX19_13(SIN(rotation->z)), FIX7_9TOFIX19_13(scale->y));
    fix19_13 highPrecisionPd = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->y));

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = FIX19_13TOFIX7_9(highPrecisionPa);
	fixedAffineMatrix.pc = FIX19_13TOFIX7_9(highPrecisionPc);

	fixedAffineMatrix.dx =
	    ITOFIX13_3(worldPointer->mx)
	    +
	    FIX19_13TOFIX13_3(FIX19_13_DIV(halfWidth, FIX7_9TOFIX19_13(abs(scale->x))))
	    -
	    FIX19_13TOFIX13_3
	    (
	        FIX19_13_MULT(highPrecisionPa, halfWidth)
	        +
	        FIX19_13_MULT(highPrecisionPb, halfheight)
        );

	fixedAffineMatrix.dy =
	    ITOFIX13_3(worldPointer->my)
	    +
	    FIX19_13TOFIX13_3(FIX19_13_DIV(halfheight, FIX7_9TOFIX19_13(abs(scale->y))))
	    -
	    (
	        FIX19_13TOFIX13_3(FIX19_13_MULT(highPrecisionPc, halfWidth))
	        +
	        FIX19_13TOFIX13_3(FIX19_13_MULT(highPrecisionPd, halfheight))
    	);

	fixedAffineMatrix.paralax = 0;

	AffineEntry* affine = (AffineEntry*)__PARAM_DISPLACEMENT(worldPointer->param);
	int i = 0 <= paramTableRow? paramTableRow: 0;
	int counter = SpriteManager_getMaximumAffineRowsToComputePerCall(SpriteManager_getInstance());

	for(;0 <= counter && i < finalRow; i++, counter--)
	{
		affine[i].pb_y = FIX19_13TOFIX13_3(FIX19_13_MULT(ITOFIX19_13(i), highPrecisionPb)) + fixedAffineMatrix.dx;
		affine[i].paralax = fixedAffineMatrix.paralax;
		affine[i].pd_y = FIX19_13TOFIX13_3(FIX19_13_MULT(ITOFIX19_13(i), highPrecisionPd)) + fixedAffineMatrix.dy;
		affine[i].pa = fixedAffineMatrix.pa;
		affine[i].pc = fixedAffineMatrix.pc;
	}

	return i < finalRow? i: -1;
}

