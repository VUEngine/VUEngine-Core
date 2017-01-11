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

#ifndef SHAPE_H_
#define SHAPE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Shape_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, int, overlaps, Shape shape);											\
		__VIRTUAL_DEC(ClassName, void, setup);															\
		__VIRTUAL_DEC(ClassName, void, position);														\
		__VIRTUAL_DEC(ClassName, int, getAxisOfCollision, SpatialObject collidingSpatialObject, VBVec3D displacement, VBVec3D previousPosition);												\
		__VIRTUAL_DEC(ClassName, int, testIfCollision, SpatialObject collidingSpatialObject, VBVec3D displacement);																	\
		__VIRTUAL_DEC(ClassName, void, deleteDirectDrawData);											\
		__VIRTUAL_DEC(ClassName, void, draw);															\
		__VIRTUAL_DEC(ClassName, void, print, int x, int y);                                            \

#define Shape_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\

#define Shape_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* the entity to which the shape belongs */														\
        SpatialObject owner;																			\
        /* flag to know if the shapes below to an entity which moves */									\
        u8 moves;																				        \
        /* whether it has been checked for collision in current update */								\
        u8 checked;																				        \
        /* flag to know if setup is needed */															\
        u8 ready;																				        \
        /* flag to check against other shapes */														\
        u8 checkForCollisions;																	        \

__CLASS(Shape);

enum ShapeTypes
{
	kNoShape = 0,
	kCircle,
	kCuboid,
	kInverseCuboid,
};


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Shape_constructor(Shape this, SpatialObject owner);
void Shape_destructor(Shape this);
SpatialObject Shape_getOwner(Shape this);
void Shape_setActive(Shape this, bool active);
bool Shape_isActive(Shape this);
bool Shape_moves(Shape this);
bool Shape_isChecked(Shape this);
void Shape_checked(Shape this, bool checked);
bool Shape_isReady(Shape this);
void Shape_setReady(Shape this, bool ready);
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions);
bool Shape_checkForCollisions(Shape this);
void Shape_draw(Shape this);
void Shape_deleteDirectDrawData(Shape this);
void Shape_print(Shape this, int x, int y);


#endif
