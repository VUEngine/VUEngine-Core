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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Circle.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Circle
__CLASS_DEFINITION(Circle, Shape);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Circle_constructor(Circle this, SpatialObject owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Circle, SpatialObject owner)
__CLASS_NEW_END(Circle, owner);

// class's constructor
static void Circle_constructor(Circle this, SpatialObject owner)
{
	ASSERT(this, "Circle::constructor: null this");

	__CONSTRUCT_BASE(owner);

	// TODO
	this->radius = 1;
}

// class's destructor
void Circle_destructor(Circle this)
{
	ASSERT(this, "Circle::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}
