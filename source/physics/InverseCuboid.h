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

#ifndef INVERSE_CUBOID_H_
#define INVERSE_CUBOID_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Cuboid.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define InverseCuboid_METHODS																			\
		Cuboid_METHODS																					\

#define InverseCuboid_SET_VTABLE(ClassName)																\
		Cuboid_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, InverseCuboid, overlaps);												\

#define InverseCuboid_ATTRIBUTES																		\
																										\
	/* super's attributes */																			\
	Cuboid_ATTRIBUTES;																					\

__CLASS(InverseCuboid);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InverseCuboid, SpatialObject owner);

void InverseCuboid_destructor(InverseCuboid this);
u8 InverseCuboid_overlaps(InverseCuboid this, Shape shape);


#endif