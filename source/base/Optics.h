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

#ifndef OPTICS_H_
#define	OPTICS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MiscStructs.h>
#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// these improve performance in the real machine
#define __OPTICS_NORMALIZE(Vector)																		\
	Vector.x -= (_screenPosition->x + 0x00001000)& 0xFFFFE000;											\
	Vector.y -= (_screenPosition->y + 0x00001000)& 0xFFFFE000;											\
	Vector.z -= (_screenPosition->z + 0x00001000)& 0xFFFFE000;											\

#define __OPTICS_PROJECT_TO_2D(Vector3D, Vector2D)														\
		Vector2D.x = Vector3D.x + (FIX19_13_MULT(_optical->horizontalViewPointCenter - 					\
            Vector3D.x, Vector3D.z) >> _optical->maximumViewDistancePower);								\
		Vector2D.y = Vector3D.y - (FIX19_13_MULT(Vector3D.y - _optical->verticalViewPointCenter,		\
            Vector3D.z) >> _optical->maximumViewDistancePower);											\
		Vector2D.z = Vector3D.z;																		\
		Vector2D.x += 0x00001000;																		\
		Vector2D.y += 0x00001000;																		\
		Vector2D.x &= 0xFFFFE000;																		\
		Vector2D.y &= 0xFFFFE000;																		\


//---------------------------------------------------------------------------------------------------------
// 												3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

extern int Optics_calculateParallax(fix19_13 x, fix19_13 z);
extern  VBVec3D Optics_normalizePosition(const VBVec3D* const position3D);
extern u16 Optics_calculateRealSize(u16 magnitude, u16 mapMode, fix7_9 scale);
extern bool Optics_isVisible(VBVec3D position3D, u16 width, u16 height, int parallax, int pad);
extern int vbjInsideGame(VBVec3D position3D, int width, int height);
extern int Optics_lengthSquared3D(VBVec3D vect1, VBVec3D vect2);


#endif