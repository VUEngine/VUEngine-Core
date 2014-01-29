/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
#include <InGameEntity.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

enum ShapeTypes{
	kCircle = 0,
	kCuboid,
};


#define Shape_METHODS								\
		Object_METHODS								\
		__VIRTUAL_DEC(draw);						\
		__VIRTUAL_DEC(overlaps);					\
		__VIRTUAL_DEC(setup);						\
		__VIRTUAL_DEC(positione);					\
		__VIRTUAL_DEC(getAxisOfCollision);			\
		__VIRTUAL_DEC(testIfCollision);


#define Shape_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)							\
		
	
#define Shape_ATTRIBUTES							\
													\
	/* super's attributes */						\
	Object_ATTRIBUTES;								\
													\
	/* the entity to which the shape belongs */		\
	InGameEntity owner;								\
													\
	/* shape's deep over the z axis */				\
	int deep;										\
													\
	/* raise flag to make the shape active */		\
	int active: 1;									\
													\
	/* flag to know if the shapes below to an */	\
	/* entity which moves */						\
	int moves: 1;									\
													\
	/* whether it has been checked */				\
	/* for collision in current update */			\
	int checked:1;									\
													\
	/* flag to know if setup is needed */			\
	int ready:1;


// A Shape which represent a generic object inside a Stage
__CLASS(Shape);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
void Shape_constructor(Shape this, InGameEntity owner, int deep);

// class's destructor
void Shape_destructor(Shape this);

// retrieve owner
InGameEntity Shape_getOwner(Shape this);

// set active
void Shape_setActive(Shape this, int active);

// is active?
int Shape_isActive(Shape this);

// do I move?
int Shape_moves(Shape this);

// has been checked
int Shape_isChecked(Shape this);

// set check status
void Shape_checked(Shape this, int checked);

// has been configured?
int Shape_isReady(Shape this);

#endif /*SHAPE_H_*/
