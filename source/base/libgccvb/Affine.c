/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Affine.h>


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

typedef struct AffineMatrix
{
	fix7_9 pa;
	fix13_3 pb;
	fix7_9 pc;
	fix13_3 pd;
	fix13_3 dx;
	fix13_3 dy;
	s16   paralax;
} AffineMatrix ;


//---------------------------------------------------------------------------------------------------------
// 											FUNCTIONS
//---------------------------------------------------------------------------------------------------------


fix19_13 Affine_applyAll(u16 param, fix19_13 paramTableRow, const Scale* scale, const Rotation* rotation, const TextureSource* textureSource, s16 width, s16 height)
{
	CACHE_ENABLE;

	AffineMatrix affineMatrix;

	ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	ASSERT(scale->y, "Affine::applyAll: 0 y scale");
	affineMatrix.pa  = FIX7_9_DIV(COS(rotation->z), scale->x);
	affineMatrix.pb  = -FIX7_9TOFIX13_3(FIX7_9_DIV(SIN(rotation->z), scale->x));
	affineMatrix.pc  = FIX7_9_DIV(SIN(rotation->z), scale->y);
	affineMatrix.pd  = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->y));
	affineMatrix.dx = FTOFIX13_3((textureSource->mx + width) - (FIX7_9TOF(FIX7_9_DIV(COS(rotation->z), scale->x)) * width + FIX13_3TOF(affineMatrix.pb) * height));
	affineMatrix.dy = FTOFIX13_3((textureSource->my + height) - (FIX7_9TOF(affineMatrix.pc) * width + FIX19_13TOF(affineMatrix.pd) * height));
	affineMatrix.paralax = 0;
	AffineEntry* affine = (AffineEntry*)__PARAM_DISPLACEMENT(param);

	AffineEntry affineEntrySource = 
	{
			affineMatrix.dx,
			affineMatrix.paralax,
			affineMatrix.dy,
			affineMatrix.pa,
			affineMatrix.pc
	};

	// add one row for cleaning up
	fix19_13 totalRows = FIX19_13_MULT(ITOFIX19_13(height << 1), FIX7_9TOFIX19_13(scale->y)) + ITOFIX19_13(1);
	
	if (0 > totalRows)
	{
		totalRows *= -1;
	}
	
	fix19_13 i = 0 <= paramTableRow? paramTableRow: 0;
	int affineEntry = FIX19_13TOI(i);
	int counter = 0;
	
	// prepare ahead of time to reduce computations
	affineMatrix.pb = FIX13_3TOTOFIX19_13(affineMatrix.pb);
	
	for (; counter < __MAXIMUM_AFFINE_ROWS_PER_CALL && i < totalRows; i += 0b10000000000000, affineEntry ++, counter++) 
	{
		affineEntrySource.pb_y = FIX19_13TOFIX13_3(FIX19_13_MULT(i, affineMatrix.pb)) + affineMatrix.dx;
		affineEntrySource.pd_y = FIX19_13TOFIX13_3(FIX19_13_MULT(i, affineMatrix.pd)) + affineMatrix.dy;
		affine[affineEntry] = affineEntrySource;
	}
	CACHE_DISABLE;
	
	return i < totalRows? i: -1;
}

