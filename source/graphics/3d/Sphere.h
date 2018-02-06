
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

#ifndef SPHERE_H_
#define SPHERE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Sphere_METHODS(ClassName)																		\
		Wireframe_METHODS(ClassName)																	\

#define Sphere_SET_VTABLE(ClassName)																	\
		Wireframe_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Sphere, draw);															\

#define Sphere_ATTRIBUTES																				\
		Wireframe_ATTRIBUTES																			\
		/**
		 * @var Vector3D 	center
		 * @brief			Vertices
		 * @memberof		Sphere
		 */																								\
		Vector3D center;																				\
		/**
		 * @var fix10_6 	radius
		 * @brief			Radious
		 * @memberof		Sphere
		 */																								\
		fix10_6 radius;																					\

__CLASS(Sphere);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Sphere, Vector3D center, fix10_6 radius);

void Sphere_destructor(Sphere this);
void Sphere_draw(Sphere this, bool calculateParallax);
Vector3D Sphere_getCenter(Sphere this);
fix10_6 Sphere_getRadius(Sphere this);
void Sphere_setCenter(Sphere this, Vector3D center);
void Sphere_setRadius(Sphere this, fix10_6 radius);


#endif
