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
 
#ifndef BODY_H_
#define BODY_H_

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
#include <Mass.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define Body_METHODS								\
		Object_METHODS								\


#define Body_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)							\
	

#define Body_ATTRIBUTES								\
													\
	/* super's attributes */						\
	Object_ATTRIBUTES;								\
													\
	/* radious */									\
	Mass mass;										\
													\
	/* radious */									\
	InGameEntity owner;								\
													\
	/* direction */									\
	Direction direction;							\
													\
	/* radious */									\
	int radious;


// A Body which represent a generic object inside a Stage
__CLASS(Body);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// class's allocator
__CLASS_NEW_DECLARE(Body, __PARAMETERS(InGameEntity owner));

// class's destructor
void Body_destructor(Body this);

// set game entity
void Body_setOwner(Body this, InGameEntity owner);

// get game entity
void Body_getOwner(Body this, InGameEntity owner);

// update
void Body_update(Body this);


#endif /*BODY_H_*/
