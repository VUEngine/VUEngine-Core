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

#ifndef OPTICS_H_
#define	OPTICS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// these improve performance in the real machine
#undef __OPTICS_NORMALIZE
#define __OPTICS_NORMALIZE(Vector)													\
	Vector.x -= (_screenPosition->x);												\
	Vector.y -= (_screenPosition->y);												\
	Vector.z -= (_screenPosition->z);

#undef __OPTICS_PROJECT_TO_2D
#define __OPTICS_PROJECT_TO_2D(Vector3D, Vector2D)									\
		Vector2D.x = Vector3D.x 													\
			+ (FIX19_13_MULT(_optical->horizontalViewPointCenter - 					\
					Vector3D.x, Vector3D.z) >> _optical->maximumViewDistancePower);	\
		Vector2D.y = Vector3D.y 													\
			- (FIX19_13_MULT(Vector3D.y - _optical->verticalViewPointCenter,		\
				Vector3D.z) >> _optical->maximumViewDistancePower);					\
		Vector2D.z = Vector3D.z;													\



//---------------------------------------------------------------------------------------------------------
// 												3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------

extern int Optics_calculateParallax(fix19_13 x, fix19_13 z);
extern VBVec3D Optics_normalizePosition(const VBVec3D* const position3D);
extern int Optics_calculateRealSize(int magnitude, u16 mapMode, fix7_9 scale);
extern int vbjInsideGame(VBVec3D position3D, int width, int height);
extern int Optics_lengthSquared3D(VBVec3D vect1, VBVec3D vect2);


#endif
