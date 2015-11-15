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

#define Shape_METHODS															\
		Object_METHODS															\
		__VIRTUAL_DEC(overlaps);												\
		__VIRTUAL_DEC(setup);													\
		__VIRTUAL_DEC(positione);												\
		__VIRTUAL_DEC(getAxisOfCollision);										\
		__VIRTUAL_DEC(testIfCollision);											\
		__VIRTUAL_DEC(deleteDirectDrawData);									\
		__VIRTUAL_DEC(draw);													\
		__VIRTUAL_DEC(print);

#define Shape_SET_VTABLE(ClassName)												\
		Object_SET_VTABLE(ClassName)											\

#define Shape_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* the entity to which the shape belongs */									\
	SpatialObject owner;														\
																				\
	/* flag to know if the shapes below to an entity which moves */				\
	bool moves: true;															\
																				\
	/* whether it has been checked for collision in current update */			\
	bool checked: true;															\
																				\
	/* flag to know if setup is needed */										\
	bool ready: true;															\
																				\
	/* flag to check agains other shapes */										\
	bool checkForCollisions: true;

// A Shape which represent a generic object inside a Stage
__CLASS(Shape);

enum ShapeTypes
{
	kNoShape = 0,
	kCircle,
	kCuboid,
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