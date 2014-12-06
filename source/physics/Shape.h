/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef SHAPE_H_
#define SHAPE_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <Entity.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

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
	Entity owner;																\
																				\
	/* flag to know if the shapes below to an */								\
	/* entity which moves */													\
	u8 moves: 1;																\
																				\
	/* whether it has been checked */											\
	/* for collision in current update */										\
	u8 checked :1;																\
																				\
	/* flag to know if setup is needed */										\
	u8 ready :1;																\
																				\
	/* flag to check agains other shapes */										\
	u8 checkForCollisions :1;

// A Shape which represent a generic object inside a Stage
__CLASS(Shape);

enum ShapeTypes{
	kCircle = 0,
	kCuboid,
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
void Shape_constructor(Shape this, Entity owner);

// class's destructor
void Shape_destructor(Shape this);

// retrieve owner
Entity Shape_getOwner(Shape this);

// set active
void Shape_setActive(Shape this, u8 active);

// is active?
u8 Shape_isActive(Shape this);

// do I move?
u8 Shape_moves(Shape this);

// has been checked
u8 Shape_isChecked(Shape this);

// set check status
void Shape_checked(Shape this, u8 checked);

// has been configured?
u8 Shape_isReady(Shape this);

// set configured flag
void Shape_setReady(Shape this, u8 ready);

// set flag
void Shape_setCheckForCollisions(Shape this, u8 checkForCollisions);

// get flag
u8 Shape_checkForCollisions(Shape this);

// draw debug data
void Shape_draw(Shape this);

// flush direct draw data
void Shape_deleteDirectDrawData(Shape this);

// print debug data
void Shape_print(Shape this, int x, int y);

#endif /*SHAPE_H_*/
