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

#include <string.h>
#include <SpatialObject.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(SpatialObject, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(SpatialObject)
__CLASS_NEW_END(SpatialObject);

// class's constructor
void SpatialObject_constructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();
}

// class's destructor
void SpatialObject_destructor(SpatialObject this)
{
	ASSERT(this, "SpatialObject::destructor: null this");

	// destroy the super SpatialObject
	__DESTROY_BASE;
}

Shape SpatialObject_getShape(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getShape: null this");

	return NULL;
}

int SpatialObject_getShapeType(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getShapeType: null this");

	return kCuboid;
}

// does it move?
bool SpatialObject_moves(SpatialObject this)
{
	ASSERT(this, "SpatialObject::moves: null this");

	// not necessarily
	return false;
}

// defaults to true
bool SpatialObject_canMoveOverAxis(SpatialObject this, const Acceleration* acceleration)
{
	ASSERT(this, "SpatialObject::canMoveOverAxis: null this");

	return false;
}

u16 SpatialObject_getWidth(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getWidth: null this");

	return 0;
}

u16 SpatialObject_getHeight(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getHeight: null this");

	return 0;
}

u16 SpatialObject_getDeep(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getDeep: null this");

	return 0;
}

Gap SpatialObject_getGap(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getGap: null this");

	Gap gap =
	{
			0, 0, 0, 0
	};
	
	return gap;
}

const VBVec3D* SpatialObject_getPosition(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getPosition: null this");
	
	static VBVec3D position = 
	{
		0, 0, 0
	};
	
	return &position;
}

const VBVec3D* SpatialObject_getPreviousPosition(SpatialObject this)
{
	ASSERT(this, "SpatialObject::getPreviousPosition: null this");
	
	static VBVec3D position = 
	{
		0, 0, 0
	};
	
	return &position;
}

