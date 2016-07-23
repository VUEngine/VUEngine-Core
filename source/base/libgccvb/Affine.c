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
	AffineMatrix affineMatrix;

	ASSERT(scale->x, "Affine::applyAll: 0 x scale");
	ASSERT(scale->y, "Affine::applyAll: 0 y scale");

	float scaleX = FIX7_9TOF(scale->x);
	float scaleY = FIX7_9TOF(scale->y);
	float absoluteScaleX = FIX7_9TOF(abs(scale->x));
	float absoluteScaleY = FIX7_9TOF(abs(scale->y));
	affineMatrix.pa = COSF(rotation->z) / scaleX;
	affineMatrix.pb = -SINF(rotation->z) / scaleX;
	affineMatrix.pc = SINF(rotation->z) / scaleY;
	affineMatrix.pd = COSF(rotation->z) / scaleY;

	affineMatrix.dx = (textureSource->mx + width / absoluteScaleX) - (affineMatrix.pa * width + affineMatrix.pb * height);
	affineMatrix.dy = (textureSource->my + height / absoluteScaleY) - (affineMatrix.pc * width + affineMatrix.pd * height);
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
	int counter = SpriteManager_getMaximumAffineRowsToComputePerCall(SpriteManager_getInstance());

	for(; counter && i < totalRows; i++, counter--)
	{
		affine[i].pb_y = FTOFIX13_3(i * affineMatrix.pb) + fixedAffineMatrix.dx;
		affine[i].paralax = fixedAffineMatrix.paralax;
		affine[i].pd_y = FTOFIX13_3(i * affineMatrix.pd) + fixedAffineMatrix.dy;
		affine[i].pa = fixedAffineMatrix.pa;
		affine[i].pc = fixedAffineMatrix.pc;
	}

	return i < totalRows? i: -1;
}
