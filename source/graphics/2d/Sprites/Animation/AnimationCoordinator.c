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

#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


__CLASS_DEFINITION(AnimationCoordinator, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

//class constructor
void AnimationCoordinator_constructor(AnimationCoordinator this, CharSet charSet)
{
	ASSERT(this, "AnimationCoordinator::constructor: null this");

	__CONSTRUCT_BASE();

	this->animationControllers = __NEW(VirtualList);
	this->charSet = charSet;
}

// class destructor
void AnimationCoordinator_destructor(AnimationCoordinator this)
{
	ASSERT(this, "AnimationCoordinator::destructor: null this");

	if(this->animationControllers) 
	{
		__DELETE(this->animationControllers);
		this->animationControllers = NULL;
	}

	// destroy the super object
	__DESTROY_BASE;
}

const CharSet AnimationCoordinator_getCharSet(AnimationCoordinator this)
{
	ASSERT(this, "AnimationCoordinator::getCharSet: null this");

	return this->charSet;
}