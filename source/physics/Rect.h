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

#ifndef RECT_H_
#define RECT_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Shape.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define Rect_METHODS								\
		Shape_METHODS								\


#define Rect_SET_VTABLE(ClassName)								\
		Shape_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Rect, draw);					\
		__VIRTUAL_SET(ClassName, Rect, overlaps);				\
		__VIRTUAL_SET(ClassName, Rect, setup);					\
		__VIRTUAL_SET(ClassName, Rect, positione);


typedef struct Rectangle{

	/* left upper corner */
	fix19_13 x0;
	fix19_13 y0;
			
	/* right down corner */
	fix19_13 x1;					
	fix19_13 y1;

}Rectangle;

#define Rect_ATTRIBUTES								\
													\
	/* super's attributes */						\
	Shape_ATTRIBUTES;								\
													\
	/* the rectangle */								\
	Rectangle rectangle;							\
													\
	/* the rectangle to check */					\
	Rectangle positionedRectangle;
	

// A Rect which represent a generic object inside a Stage
__CLASS(Rect);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Rect, __PARAMETERS(InGameEntity owner, int deep));

// class's destructor
void Rect_destructor(Rect this);

// check if overlaps with other shape
int Rect_overlaps(Rect this, Shape shape, int axisMovement);

// setup the rectangle
void Rect_setup(Rect this);

// prepare the shape to be checked
void Rect_positione(Rect this);

// retrieve rectangle
Rectangle Rect_getPositionedRectangle(Rect this);

// draw rect
void Rect_draw(Rect this);

#endif /*RECT_H_*/
