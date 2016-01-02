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

#ifndef DEBUG_CONFIG_H_
#define DEBUG_CONFIG_H_


//---------------------------------------------------------------------------------------------------------
// 										CONFIG MACROS OVERRIDE
//---------------------------------------------------------------------------------------------------------

#undef __PRINT_FRAMERATE

//#undef __TIMER_RESOLUTION
//#define __TIMER_RESOLUTION			1
//#undef __MILLISECONDS_IN_SECOND
//#define __MILLISECONDS_IN_SECOND	100

/*
// these improve performance in the real machine
#undef __OPTICS_NORMALIZE
#define __OPTICS_NORMALIZE(Vector)													\
	Vector.x -= (_screenPosition->x + 0x00001000)& 0xFFFFE000;						\
	Vector.y -= (_screenPosition->y + 0x00001000)& 0xFFFFE000;						\
	Vector.z -= (_screenPosition->z + 0x00001000)& 0xFFFFE000;

#undef __OPTICS_PROJECT_TO_2D
#define __OPTICS_PROJECT_TO_2D(Vector3D, Vector2D)									\
		Vector2D.x = Vector3D.x 													\
			+ (FIX19_13_MULT(_optical->horizontalViewPointCenter - 					\
					Vector3D.x, Vector3D.z) >> _optical->maximumViewDistancePower);	\
		Vector2D.y = Vector3D.y 													\
			- (FIX19_13_MULT(Vector3D.y - _optical->verticalViewPointCenter,		\
				Vector3D.z) >> _optical->maximumViewDistancePower);					\
		Vector2D.z = Vector3D.z;													\
		Vector2D.x += 0x00001000;													\
		Vector2D.y += 0x00001000;													\
		Vector2D.x &= 0xFFFFE000;													\
		Vector2D.y &= 0xFFFFE000;					
*/


#endif