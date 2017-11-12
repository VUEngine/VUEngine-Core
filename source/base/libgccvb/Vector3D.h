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

#ifndef VECTOR_3D_H_
#define VECTOR_3D_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>
#include <MiscStructs.h>


//---------------------------------------------------------------------------------------------------------
//											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Vector3D Vector3D_get(Vector3D from, Vector3D to);
fix19_13 Vector3D_dotProduct(Vector3D vectorA, Vector3D vectorB);
Vector3D Vector3D_scalarProduct(Vector3D vector, fix19_13 scalar);
Vector3D Vector3D_normalize(Vector3D vector);
Vector3D Vector3D_getPlaneNormal(Vector3D vectorA, Vector3D vectorB, Vector3D vectorC);
fix19_13 Vector3D_length(Vector3D vector);
fix19_13 Vector3D_lengthProduct(Vector3D vectorA, Vector3D vectorB);

#endif
