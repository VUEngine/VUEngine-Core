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

typedef struct AffineMatrix
{
	float pa;
	float pb;
	float pc;
	float pd;
	float dx;
	float dy;
	s16   paralax;
} AffineMatrix ;

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


fix19_13 Affine_applyAll(u16 param, fix19_13 paramTableRow, const Scale* scale, const Rotation* rotation, const TextureSource* textureSource, s16 width, s16 height)
{
	CACHE_ENABLE;

	AffineMatrix affineMatrix;

	ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	ASSERT(scale->y, "Affine::applyAll: 0 y scale");

	float scaleX = FIX7_9TOF(scale->x);
	float scaleY = FIX7_9TOF(scale->y);
	affineMatrix.pa = COSF(rotation->z) / scaleX;
	affineMatrix.pb = -SINF(rotation->z) / scaleX;
	affineMatrix.pc = SINF(rotation->z) / scaleY;
	affineMatrix.pd = COSF(rotation->z) / scaleY;
	affineMatrix.dx = (textureSource->mx + width / fabs(scaleX)) - (affineMatrix.pa * width + affineMatrix.pb * height);
	affineMatrix.dy = (textureSource->my + height / fabs(scaleY)) - (affineMatrix.pc * width + affineMatrix.pd * height);
	affineMatrix.paralax = 0;

	AffineEntry* affine = (AffineEntry*)__PARAM_DISPLACEMENT(param);

	FixedAffineMatrix fixedAffineMatrix;
	fixedAffineMatrix.pa = FTOFIX7_9(affineMatrix.pa);
	fixedAffineMatrix.pb = FTOFIX13_3(affineMatrix.pb);
	fixedAffineMatrix.pc = FTOFIX7_9(affineMatrix.pc);
	fixedAffineMatrix.pd = FTOFIX13_3(affineMatrix.pd);
	fixedAffineMatrix.dx = FTOFIX13_3(affineMatrix.dx);
	fixedAffineMatrix.dy = FTOFIX13_3(affineMatrix.dy);
	fixedAffineMatrix.paralax = affineMatrix.paralax;
	
	// add one row for cleaning up
	int totalRows = (height << 1) * scaleY + 1;
	int i = 0 <= paramTableRow? paramTableRow: 0;
	int counter = 0;

	for(; counter < __MAXIMUM_AFFINE_ROWS_PER_CALL && i < totalRows; i++, counter++)
	{
		affine[i].pb_y = FTOFIX13_3(i * affineMatrix.pb) + fixedAffineMatrix.dx;
		affine[i].paralax = fixedAffineMatrix.paralax;
		affine[i].pd_y = FTOFIX13_3(i * affineMatrix.pd) + fixedAffineMatrix.dy;
		affine[i].pa = fixedAffineMatrix.pa;
		affine[i].pc = fixedAffineMatrix.pc;
	}
	
	CACHE_DISABLE;
	
	return i < totalRows? i: -1;
}

/*
fix19_13 Affine_applyAllLowPrecision(u16 param, fix19_13 paramTableRow, const Scale* scale, const Rotation* rotation, const TextureSource* textureSource, s16 width, s16 height)
{
	CACHE_ENABLE;

	AffineMatrix affineMatrix;

//	if(512 == abs(rotation->z)) rotation->z= 0;
	ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	ASSERT(scale->y, "Affine::applyAll: 0 y scale");

	affineMatrix.pa = FIX7_9_DIV(COS(rotation->z), scale->x);
	affineMatrix.pb = -FIX7_9TOFIX13_3(FIX7_9_DIV(SIN(rotation->z), scale->x));
	affineMatrix.pc = FIX7_9_DIV(SIN(rotation->z), scale->y);
	affineMatrix.pd = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->y));
	affineMatrix.dx = FTOFIX13_3((textureSource->mx + width) - (FIX7_9TOF(affineMatrix.pa) * width + FIX13_3TOF(affineMatrix.pb) * height));
	affineMatrix.dy = FTOFIX13_3((textureSource->my + height) - (FIX7_9TOF(affineMatrix.pc) * width + FIX19_13TOF(affineMatrix.pd) * height));
	affineMatrix.paralax = 0;

	fix19_13 prePa = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->x));
	fix19_13 prePb = -FIX19_13_DIV(FIX7_9TOFIX19_13(SIN(rotation->z)), FIX7_9TOFIX19_13(scale->x));
	fix19_13 prePc = FIX19_13_DIV(FIX7_9TOFIX19_13(SIN(rotation->z)), FIX7_9TOFIX19_13(scale->y));
	fix19_13 prePd = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(rotation->z)), FIX7_9TOFIX19_13(scale->y));
	affineMatrix.pa = FIX19_13TOFIX7_9(prePa);
	affineMatrix.pb = FIX19_13TOFIX13_3(prePb);
	affineMatrix.pc = FIX19_13TOFIX7_9(prePc);
	affineMatrix.pd = prePd;
	affineMatrix.dx = ITOFIX13_3(textureSource->mx + width) - FTOFIX13_3(FIX19_13TOF(prePa)* width + FIX19_13TOF(prePb) * height);
	affineMatrix.dy = ITOFIX13_3(textureSource->my + height) - FTOFIX13_3(FIX19_13TOF(prePc)* width + FIX19_13TOF(prePd)* height);
	affineMatrix.paralax = 0;
	if(!affineMatrix.pa) affineMatrix.pa = 1;
	int angle = 128;
	affineMatrix.pa  = FIX7_9_DIV(COS(angle), scale->x);
	affineMatrix.pb  = -FIX7_9TOFIX13_3(FIX7_9_DIV(SIN(angle), scale->x));
	affineMatrix.pc  = FIX7_9_DIV(SIN(angle), scale->y);
	affineMatrix.pd  = FIX19_13_DIV(FIX7_9TOFIX19_13(COS(angle)), FIX7_9TOFIX19_13(scale->y));
	affineMatrix.dx = FTOFIX13_3((textureSource->mx + width) - 0*(FIX7_9TOF(FIX7_9_DIV(COS(angle), scale->x)) * width + FIX13_3TOF(affineMatrix.pb) * height));
	affineMatrix.dy = FTOFIX13_3((textureSource->my + height) - 0*(FIX7_9TOF(affineMatrix.pc) * width + FIX19_13TOF(affineMatrix.pd) * height));

	float scaleX = FIX7_9TOF(scale->x);
	float scaleY = FIX7_9TOF(scale->y);
	affineMatrix.pa = COSF(rotation->z) / scaleX;
	affineMatrix.pb = -SINF(rotation->z) / scaleX;
	affineMatrix.pc = SINF(rotation->z) / scaleY;
	affineMatrix.pd = COSF(rotation->z) / scaleY;
	affineMatrix.dx = (textureSource->mx + width) - (affineMatrix.pa * width + affineMatrix.pb * height);
	affineMatrix.dy = (textureSource->my + height) - (affineMatrix.pc * width + affineMatrix.pd * height);
	affineMatrix.paralax = 0;
	affineMatrix.pa = FTOFIX7_9(affineMatrix.pa);
	//affineMatrix.pb = FTOFIX19_13(affineMatrix.pb);
	affineMatrix.pc = FTOFIX7_9(affineMatrix.pc);
	//affineMatrix.pd = FTOFIX19_13(affineMatrix.pd);
	affineMatrix.dx = FTOFIX13_3(affineMatrix.dx);
	affineMatrix.dy = FTOFIX13_3(affineMatrix.dy);

	AffineEntry* affine = (AffineEntry*)__PARAM_DISPLACEMENT(param);
	AffineEntry affineEntrySource = 
	{
			affineMatrix.dx,
			affineMatrix.paralax,
			affineMatrix.dy,
			FTOFIX7_9(affineMatrix.pa),
			FTOFIX7_9(affineMatrix.pc)
	};
	// add one row for cleaning up
	fix19_13 totalRows = FIX19_13_MULT(ITOFIX19_13(height << 1), FIX7_9TOFIX19_13(scale->y)) + ITOFIX19_13(1);
	
	if(0 > totalRows)
	{
		totalRows *= -1;
	}
	
	fix19_13 i = 0 <= paramTableRow? paramTableRow: 0;
	int affineEntry = FIX19_13TOI(i);
	int counter = 0;
	
	// prepare ahead of time to reduce computations
	//affineMatrix.pb = FIX13_3TOTOFIX19_13(affineMatrix.pb);
	totalRows = height << 2;
	i = 0;
	affineEntry = 0;
//	for(; counter < __MAXIMUM_AFFINE_ROWS_PER_CALL && i < totalRows; i += 0b10000000000000, affineEntry ++, counter++)
//	for(; i < totalRows; i += 0b10000000000000, affineEntry ++, counter++)
		for(; i < totalRows; i++, affineEntry ++, counter++)
	{
			affine[affineEntry].pb_y = FTOFIX13_3(i * affineMatrix.pb + affineMatrix.dx);
			affine[affineEntry].paralax = affineMatrix.paralax;
			affine[affineEntry].pd_y = FTOFIX13_3(i * affineMatrix.pd + affineMatrix.dy);
			affine[affineEntry].pa = FTOFIX7_9(affineMatrix.pa);
			affine[affineEntry].pc = FTOFIX7_9(affineMatrix.pc);
	}
	CACHE_DISABLE;
	
	return i < totalRows? i: -1;
}
*/