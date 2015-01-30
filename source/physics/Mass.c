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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Body.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Mass
__CLASS_DEFINITION(Mass, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Mass, fix19_13 weight)
__CLASS_NEW_END(Mass, weight);

// class's constructor
void Mass_constructor(Mass this, fix19_13 weight)
{
	ASSERT(this, "Mass::constructor: null this");

	__CONSTRUCT_BASE();

	this->weight = weight;
}

// class's destructor
void Mass_destructor(Mass this)
{
	ASSERT(this, "Mass::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// retrieve weight
fix19_13 Mass_getWeight(Mass this)
{
	ASSERT(this, "Mass::getWeight: null this");

	return this->weight;
}