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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// these improve performance in the real machine
#undef __OPTICS_NORMALIZE
#define __OPTICS_NORMALIZE(vector3D)																	\
		extern const Vector3D* _screenPosition;															\
		vector3D.x -= (_screenPosition->x);																\
		vector3D.y -= (_screenPosition->y);																\
		vector3D.z -= (_screenPosition->z);

#undef __OPTICS_PROJECT_TO_2D
#define __OPTICS_PROJECT_TO_2D(vector3D, vector2D)														\
		vector2D.x = vector3D.x 																		\
			+ (__FIX19_13_MULT(_optical->horizontalViewPointCenter - 									\
					vector3D.x, vector3D.z) >> _optical->maximumViewDistancePower);						\
		vector2D.y = vector3D.y 																		\
			- (__FIX19_13_MULT(vector3D.y - _optical->verticalViewPointCenter,							\
				vector3D.z) >> _optical->maximumViewDistancePower);										\
		vector2D.z = vector3D.z;																		\

#undef __OPTICS_3D_LENGHT_SQUARED
#define __OPTICS_3D_LENGHT_SQUARED(vector1, vector2)													\
		__FIX19_13_TO_I(__FIX19_13_MULT((vector1.x - vector2.x), (vector1.x - vector2.x)) +				\
        			__FIX19_13_MULT((vector1.y - vector2.y), (vector1.y - vector2.y)) +					\
        			__FIX19_13_MULT((vector1.z - vector2.z), (vector1.z - vector2.z)))					\



//---------------------------------------------------------------------------------------------------------
//												EXTERNALS
//---------------------------------------------------------------------------------------------------------

extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//											3D HELPER FUNCTIONS
//---------------------------------------------------------------------------------------------------------


extern int Optics_calculateParallax(fix19_13 x, fix19_13 z);


#endif
